// 15-745 S13 Assignment 2: liveness.cpp
// Group: nkoorapa, pdixit
// Description : This file implements liveness analysis by exploiting the 
//               generic framework in dataflow.h This also serves as an example
//               of how to use the generic framework. 
//
//               Following things should be noticed when refering to it as an 
//               example when writing another pass : 
// 
//               1. Tranfer-function parameter type is defined as GKPair which 
//                  is pair of gen and kill sets that can be associated to each
//                  instruction or basic block
//
//               2. LivenessDF is the main dataflow class which extends DataFlow 
//                  with GKPair and uses constructor parameters to indicate that
//                  Liveness is a BACKWARDS analysis with emptySet as the TOP 
//                  and boundary values.
//          
//               3. Virtual function transferFunctionParams is implemented to 
//                  calculate output = ( input - kill ) U gen
//
//               4. Virtual function compose is implemented to calculate gen
//                  & kill sets of consecutive instructions (or instr & block)
//              
//               5. Meet operator function is overriden to give union
//
//               6. getTFParams is overriden to return gen and kill sets for 
//                  each instruction based on how they are defined for liveness
// 
///////////////////////////////////////////////////////////////////////////////

#include "llvm/Function.h"
#include "llvm/Pass.h"
#include "llvm/Support/InstIterator.h"
#include "llvm/Support/raw_ostream.h"

#include "dataflow.h"

#include <iostream>

using namespace llvm;

namespace {

class Liveness : public FunctionPass {
 public:
  static char ID;

  // Map from a variable to it's index in vector of variables
  typedef std::map<Value*, int> idxmap_t;
  typedef std::pair<BitVector, BitVector> GKPair;

  /* Composeable parameters where parameters are a Gen/Kill pair */
  class LivenessDF : public DataFlow<GKPair> {
    private:
      idxmap_t* map;

    public:
      LivenessDF(idxmap_t* map) :
        DataFlow<GKPair>(emptySet(map->size()),
                         emptySet(map->size()), BACKWARDS) {
          this->map = map;

#if 0
          for (idxmap_t::iterator it = map->begin(), end =
              map->end(); it != end; ++it) {
            fprintf(stderr, "%p -> %d\n", it->first, it->second);
            it->first->print(errs());
            fprintf(stderr, "\n");
          }
#endif
      }
      ~LivenessDF() {}

      // Override. (in - kill) U gen
      BitVector transferFunctionParams(GKPair params, BitVector input) {
        return input.reset(params.second) |= params.first;
      }

      // Override
      GKPair compose(GKPair a, GKPair b) {
        // gen = (a.gen - b.kill) U b.gen
        BitVector gen = a.first;
        gen.reset(b.second) |= b.first;
        // kill = a.kill U b.kill
        BitVector kill = a.second;
        kill |= b.second;
        return GKPair::pair(gen, kill);
      }

      GKPair getTFParams(Instruction* inst) {
        idxmap_t::iterator it;
        BitVector gen = emptySet(map->size());
        BitVector kill = emptySet(map->size());

        // Identity transfer function (on no instructions)
        if (inst == NULL) {
          // empty gen/kill sets
        }
        // kill set (writing to a variable)
        else if (StoreInst* storeInst = dyn_cast<StoreInst>(inst)) {
          it = map->find(storeInst->getPointerOperand());
          if (it != map->end()) {
            kill[it->second] = true;
          }
        }
        // gen set (loading from variable)
        else if (LoadInst* loadInst = dyn_cast<LoadInst>(inst)) {
          it = map->find(loadInst->getPointerOperand());
          if (it != map->end()) {
            gen[it->second] = true;
          }
        }

        return GKPair::pair(gen, kill);
      }

      // Override
      BitVector meet(BitVector left, const BitVector& right) {
        return left |= right;
      }
  };

  Liveness() : FunctionPass(ID) { }

  virtual bool runOnFunction(Function& F) {

    // Make one pass through to find all variables.
    std::vector<Value*> varList;
    idxmap_t varMap;
    BasicBlock& entryBlock = F.getEntryBlock();

    // Walk through entry block to find all variable candidates
    for (BasicBlock::iterator iter = entryBlock.begin(),
         end = entryBlock.end(); iter != end; ++iter) {
      Instruction* inst = &(*iter);
      if (dyn_cast<AllocaInst>(inst)) {
        varMap[inst] = 0;
      }
    }
    // Walk through all instructions to make sure variables are valid.
    for (inst_iterator iter = inst_begin(F), end = inst_end(F);
         iter != end; ++iter) {
      Instruction* inst = &(*iter);
      if (dyn_cast<AllocaInst>(inst)) {
        // Do nothing
      } else if (StoreInst* storeInst = dyn_cast<StoreInst>(inst)) {
        Value* p = storeInst->getPointerOperand();
        Value* v = storeInst->getValueOperand();
        idxmap_t::iterator it = varMap.find(v);
        if (it != varMap.end()) {
          varMap.erase(it);
        }
        it = varMap.find(p);
        if (it != varMap.end() && storeInst->isVolatile()) {
          varMap.erase(it);
        }
      } else if (LoadInst* loadInst = dyn_cast<LoadInst>(inst)) {
        Value* v = loadInst->getPointerOperand();
        idxmap_t::iterator it = varMap.find(v);
        if (it != varMap.end() && loadInst->isVolatile()) {
          varMap.erase(it);
        }
      } else {
        for (User::value_op_iterator op = inst->value_op_begin(),
             opEnd = inst->value_op_end(); op != opEnd; ++op) {
          Value* v = *op;
          idxmap_t::iterator it = varMap.find(v);
          if (it != varMap.end()) {
            varMap.erase(it);
          }
        }
      }
    }

    // Assign indices to the variables in this function
    int idx = 0;
    for (idxmap_t::iterator iter = varMap.begin(),
         end = varMap.end(); iter != end; ++iter) {
      varList.push_back(iter->first);
      iter->second = idx;
      idx++;
    }

    // Run analysis to get instruction -> bitmap
    LivenessDF ldf(&varMap);
    std::map<Instruction*, BitVector> result = ldf.doAnalysis(F);

    // Output!
    for (inst_iterator iter = inst_begin(F), end = inst_end(F);
         iter != end; ++iter) {
      Instruction* inst = &(*iter);
      fprintf(stderr, "Live: {");
      BitVector& bits = result[inst];
      for (unsigned i=0; i<bits.size(); i++) {
        if (bits[i]) {
          Value* v = varList[i];
          v->print(errs());
          fprintf(stderr, "; ");
        }
      }
      fprintf(stderr, "}\n");
      inst->print(errs());
      fprintf(stderr, "\n");
    }
    fprintf(stderr, "Live: {}\n");

    // Did not modify the incoming Function.
    return false;
  }

  virtual void getAnalysisUsage(AnalysisUsage& AU) const {
    AU.setPreservesCFG();
  }

 private:
};

char Liveness::ID = 0;
RegisterPass<Liveness> X("cd-liveness", "15745 Liveness");

}
