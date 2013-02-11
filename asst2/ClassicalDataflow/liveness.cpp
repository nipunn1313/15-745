// 15-745 S13 Assignment 2: liveness.cpp
// Group: nkoorapa, pdixit
////////////////////////////////////////////////////////////////////////////////

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

  class LivenessDF : public DataFlow {
    private:
      std::map<Value*, int>* map;

    public:
      LivenessDF(std::map<Value*, int>* map) :
        DataFlow(emptySet(map->size()), emptySet(map->size()), BACKWARDS) {
          this->map = map;

#if 0
          for (std::map<Value*, int>::iterator it = map->begin(), end =
              map->end(); it != end; ++it) {
            fprintf(stderr, "%p -> %d\n", it->first, it->second);
            it->first->print(errs());
            fprintf(stderr, "\n");
          }
#endif
      }
      ~LivenessDF() {}

      virtual BitVector transferFunction(Instruction* inst, BitVector before) {

        std::map<Value*, int>::iterator it;

        // Eliminate kill set (writing to a variable)
        if (StoreInst* storeInst = dyn_cast<StoreInst>(inst)) {
          Value* v = storeInst->getPointerOperand();
          it = map->find(v);
          if (it != map->end()) {
            before[it->second] = false;
          }
        }
        // Union with vars_used set
        else if (LoadInst* loadInst = dyn_cast<LoadInst>(inst)) {
          it = map->find(loadInst->getPointerOperand());
          if (it != map->end()) {
            before[it->second] = true;
          }
        }

        // (before - kill) U vars_used
        return before;
      }

      virtual BitVector meet(BitVector left, const BitVector& right) {
        return left |= right;
      }
  };

  Liveness() : FunctionPass(ID) { }

  virtual bool runOnFunction(Function& F) {
    //ExampleFunctionPrinter(errs(), F);

    // Make one pass through to find all variables.
    std::vector<Value*> varList;
    std::map<Value*, int> varMap;
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
        std::map<Value*, int>::iterator it = varMap.find(v);
        if (it != varMap.end()) {
          varMap.erase(it);
        }
        it = varMap.find(p);
        if (it != varMap.end() && storeInst->isVolatile()) {
          varMap.erase(it);
        }
      } else if (LoadInst* loadInst = dyn_cast<LoadInst>(inst)) {
        Value* v = loadInst->getPointerOperand();
        std::map<Value*, int>::iterator it = varMap.find(v);
        if (it != varMap.end() && loadInst->isVolatile()) {
          varMap.erase(it);
        }
      } else {
        for (User::value_op_iterator op = inst->value_op_begin(),
             opEnd = inst->value_op_end(); op != opEnd; ++op) {
          Value* v = *op;
          std::map<Value*, int>::iterator it = varMap.find(v);
          if (it != varMap.end()) {
            varMap.erase(it);
          }
        }
      }
    }

    // Assign indices to the variables in this function
    int idx = 0;
    for (std::map<Value*, int>::iterator iter = varMap.begin(),
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
