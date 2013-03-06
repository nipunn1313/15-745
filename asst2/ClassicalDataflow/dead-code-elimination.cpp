// 15-745 S13 Assignment 2: dead-code-elimination.cpp
// Group: nkoorapa, pdixit
// Description : This file implements dead-code-elimination analysis by exploiting the   // TODO : Update this at end .. Also change 'Liveness' to something else
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
//               2. DeadCodeEliminationDF is the main dataflow class which extends DataFlow 
//                  with GKPair and uses constructor parameters to indicate that
//                  DeadCodeElimination is a BACKWARDS analysis with emptySet as the TOP 
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
//                  each instruction based on how they are defined for
//                  dead-code-elimination
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

class DeadCodeElimination : public FunctionPass {
 public:
  static char ID;

  // Map from a variable to it's index in vector of variables
  typedef std::map<Value*, int> idxmap_t;
  //typedef std::pair<BitVector, BitVector> GKPair;

  // For one instruction, it's Transfer-Function parameter are
  // it's index in the bit vector, and users
  // These can be computed and cached and used again and again
  typedef struct {
    bool definitely_live;
    int index;
    BitVector uses;
  } deadcode_params_inst_t;

  // Generic Transfer-Function parameter for basic-blocks
  // (one element per instr)
  typedef std::vector<deadcode_params_inst_t> deadcode_params_t;

  /* Composeable parameters where parameters are a Gen/Kill pair */
  class DeadCodeEliminationDF : public DataFlow<deadcode_params_t> {
    private:
      idxmap_t* index_map;

    public:
      DeadCodeEliminationDF(idxmap_t* index_map) :
        DataFlow<deadcode_params_t>(emptySet(index_map->size()),
                         emptySet(index_map->size()), BACKWARDS) {
          this->index_map = index_map;

#if 0
          for (idxmap_t::iterator it = map->begin(), end =
              map->end(); it != end; ++it) {
            fprintf(stderr, "%p -> %d\n", it->first, it->second);
            it->first->print(errs());
            fprintf(stderr, "\n");
          }
#endif
      }
      ~DeadCodeEliminationDF() {}

      // Override. (in - kill) U gen
      // TODO : Make it take reference to params
      BitVector transferFunctionParams(deadcode_params_t params, BitVector input) {
        // Set input bits for all the uses which are already set:
        for (deadcode_params_t::iterator iter = params.begin(),
          end = params.end(); iter != end; ++iter) {

          deadcode_params_inst_t* temp = &(*iter);

          // If input is set for an index or it is definitely live,
          // set the use bits in input
          if (temp->definitely_live || input[temp->index] ||
              input.anyCommon(temp->uses)) {
            input[temp->index] = true; // Ensures for definitely_live case
          }
        }
        return input;
      }

      // Override
      deadcode_params_t compose(deadcode_params_t a, deadcode_params_t b) {
        // Concatenate the two vectors in reverse order :
        // TODO : Review/test this part to ensure things are as expected.
        // Within a block when iterating, we want to work in reverse order
        b.insert(b.end(), a.begin(), a.end());
        return b;
      }

      // Return parameters for an instruction:
      deadcode_params_t getTFParams(Instruction* inst) {
        idxmap_t::iterator it;
        deadcode_params_inst_t result;

        // Value to return is a vector
        deadcode_params_t ret_result;

        // Identity transfer function (on no instructions)
        if (inst == NULL) {
          // Empty vector for no instructions
          return ret_result;
        }

        // Initial values :

        // Definitely live case:
        if (isa<TerminatorInst>(inst) ||
            isa<DbgInfoIntrinsic>(inst) ||
            isa<LandingPadInst>(inst) ||
            inst->mayHaveSideEffects()) {
          result.definitely_live = true;
        } else {
          result.definitely_live = false;
        }

        result.index = (*index_map)[inst];

        // Print the instruction :
        //*//fprintf (stderr, "Instruction : ");
        //*//inst->print(errs());
        //*//fprintf (stderr, "\n");

        // Get the uses :
        result.uses = emptySet(index_map->size());
        for (User::value_op_iterator iter = inst->value_op_begin(),
            end = inst->value_op_end(); iter != end; ++iter) {

          Value* use_inst = *iter;
          result.uses[(*index_map)[use_inst]] = true;
        }

        // Push on vector and return it:
        ret_result.push_back(result);
        return (ret_result);
      }

      // Override
      BitVector meet(BitVector left, const BitVector& right) {
        return left |= right;
      }
  };

  DeadCodeElimination() : FunctionPass(ID) { }

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
    DeadCodeEliminationDF ldf(&varMap);
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

char DeadCodeElimination::ID = 0;
RegisterPass<DeadCodeElimination> X("cd-dce", "15745 Dead-Code-Elimination");

}
