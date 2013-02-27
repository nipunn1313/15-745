// 15-745 S13 Assignment 2: liveness.cpp
// Group: nkoorapa, pdixit
// TODO : Update this at end
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
#include "llvm/IntrinsicInst.h"

#include "dataflow.h"

#include <iostream>

using namespace llvm;

namespace {

class Liveness : public FunctionPass {
 public:
  static char ID;

  // Map from a variable to it's index in vector of variables
  typedef std::map<Value*, int> idxmap_t;
  //typedef std::pair<BitVector, BitVector> GKPair;
  
  typedef struct {
    bool definitely_live;
    int index;
    BitVector uses;
  } deadcode_params_t;

  /* Composeable parameters where parameters are a Gen/Kill pair */
  class LivenessDF : public DataFlow<deadcode_params_t> {
    private:
      idxmap_t* map;

    public:
      LivenessDF(idxmap_t* map) :
        DataFlow<deadcode_params_t>(emptySet(map->size()),
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
      // TODO : Make it take reference to params

Implement the new scheme in which we keep track of index of current instruction mapped to bit vector of it's sources

      BitVector transferFunctionParams(deadcode_params_t params, BitVector input) {
        if (params.definitely_live || (input.anyCommon(params.uses))) {
            input[index] = true;
        }
        return input;
      }

      // Override
      deadcode_params_t compose(deadcode_params_t a, deadcode_params_t b) {
        return (a |= b);
      }

      deadcode_params_t getTFParams(Instruction* inst) {
        idxmap_t::iterator it;
        deadcode_params_t gen = emptySet(map->size());

        // Identity transfer function (on no instructions)
        if (inst == NULL) {
          // empty gen sets
        }
        // gen set (writing to a variable)
        else {
            if (isa<TerminatorInst>(inst) ||
                isa<DbgInfoIntrinsic>(inst) ||
                isa<LandingPadInst>(inst) ||
                inst->mayHaveSideEffects()) {

                it = map->find(inst);
                if (it != map->end()) {
                    gen[it->second] = true;
                }
            }
        }
        return gen;
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

    int idx = 0;

    // Walk through all instructions to get variable candidates
    for (inst_iterator iter = inst_begin(F), end = inst_end(F);
         iter != end; ++iter) {
      Instruction* inst = &(*iter);
      varMap[inst] = idx;
      idx++;

      varList.push_back(inst);
    }


    // Run analysis to get instruction -> bitmap
    LivenessDF ldf(&varMap);
    std::map<Instruction*, BitVector> result = ldf.doAnalysis(F);

    // Print out the dead instructions :
    Instruction* first_inst = &(F.getEntryBlock().front());

    // At the end of analysis, bit vector before the first instr
    // contains live instruction - because of SSA representation.
    BitVector live_insts = result[first_inst];

    for (unsigned i = 0; i < live_insts.size(); ++i) {
        if (live_insts[i] == false) {
          varList[i]->print(errs());  
          errs()<< "\n";
        }
    }

    // the incoming Function is modified.
    return true;
  }

  virtual void getAnalysisUsage(AnalysisUsage& AU) const {
    AU.setPreservesAll();
  }

 private:
};

char Liveness::ID = 0;
RegisterPass<Liveness> X("cd-dce", "15745 Dead-Code-Elimination");

}
