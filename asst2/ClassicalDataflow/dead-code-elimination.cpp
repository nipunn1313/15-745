// 15-745 S13 Assignment 2: dead-code-elimination.cpp
// Group: nkoorapa, pdixit
// Description : This file implements dead-code-elimination analysis by 
//               exploiting the generic framework in dataflow.h 
//
//               Following steps were taken to use the generic framework :
//
//               1. Domain is a bit vector of all instructions, initially 
//                  all set to empty to indicate all instructions are dead.
//                  Then dataflow framework sets bits during it's iteration
//                  if an instruction is definitely live or one of it's uses
//                  is live. An instruction is definitely live if it has 
//                  side effects.
//
//               2. Tranfer-function parameter type is defined as a struct 
//                  containing a bool definitely_live, an int index and a 
//                  bitvector of uses of the instruction. Index points to index
//                  of the instruction in bitvector
//
//               3. DeadCodeEliminationDF is the main dataflow class which 
//                  extends DataFlow with deadcode_params_t and uses constructor
//                  parameters to indicate that DeadCodeElimination is a 
//                  BACKWARDS analysis with emptySet as the TOP and boundary.
//          
//               4. Virtual function transferFunctionParams is implemented to 
//                  calculate set the 'live' bit for an instruction if either 
//                  it is definitely live or any of it's uses is live.
//
//               5. Virtual function compose is not implemented
//              
//               6. Meet operator function is overriden to give union
//
//               7. getTFParams is overriden to return definitely_live, uses,
//                  and index for each instruction.
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
  typedef std::map<Instruction*, int> idxmap_t;

  // Parameters for one instruction,
  // These can be computed and cached and used again and again
  typedef struct {
    bool definitely_live;   // True if instr affects results or has side effects
    int index;              // Index of current instr into input bit vector
    BitVector uses;         // Bit Vector of instructions which use current instr
  } deadcode_params_t;

  /* Inherit the dataflow framework with deadcode_params_t as template parameter */
  class DeadCodeEliminationDF : public DataFlow<deadcode_params_t> {
    private:
      idxmap_t* index_map;

    public:

      /* Constructor */
      DeadCodeEliminationDF(idxmap_t* index_map) :
        DataFlow<deadcode_params_t>(emptySet(index_map->size()),
                         emptySet(index_map->size()), BACKWARDS) {
          this->index_map = index_map;
      }
      ~DeadCodeEliminationDF() {}

      // Set the live bit if an instruction is definitely live or another instruction
      // which uses it is live during some iteration of dataflow
      BitVector transferFunctionParams(deadcode_params_t params, BitVector input) {

        // If instruction is definitely live or any of it's uses is live
        // set the live bit in input
        if (params.definitely_live || input.anyCommon(params.uses)) {
          input[params.index] = true; 
        
#if 0
          if (params.definitely_live) {
            fprintf (stderr, "Instruction is definitely live\n");
          }
          else {
            fprintf (stderr, "Some of the uses of instruction are live\n");
          }
#endif

        }
               
        return input;
      }

      // Override to tell that analysis at block level is not useful for this dataflow
      virtual bool composition_defined() {
        return false;
      }

      // Override
      deadcode_params_t compose(deadcode_params_t a, deadcode_params_t b) {
        // Dummy return, it will not be use since we marked 
        // composition_defined as false
        return a;
      }

      // Override by initializing deadcode_params_t struct for each instruction
      // definitely_live set based on conditions mentioned in handout
      deadcode_params_t getTFParams(Instruction* inst) {
        idxmap_t::iterator it;
        deadcode_params_t result;

        // Identity transfer function (on no instructions)
        if (inst == NULL) {
          return result;
        }

        // Definitely live case:
        if (isa<TerminatorInst>(inst) ||
            isa<DbgInfoIntrinsic>(inst) ||
            isa<LandingPadInst>(inst) ||
            inst->mayHaveSideEffects()) {
          result.definitely_live = true;
        } else {
          result.definitely_live = false;
        }

        // Index :
        result.index = (*index_map)[inst];

#if 0
        // Print the instruction :
        fprintf (stderr, "Instruction : ");
        inst->print(errs());
        fprintf (stderr, "\n");
#endif

        // Get the uses :
        result.uses = emptySet(index_map->size());
        for (User::use_iterator iter = inst->use_begin(),
            end = inst->use_end(); iter != end; ++iter) {

          Instruction* use_inst = (Instruction *) *iter;
          result.uses[(*index_map)[use_inst]] = true;

#if 0
          fprintf (stderr, "Uses : ");
          use_inst->print(errs());
          fprintf (stderr,"\n");
#endif
          
        }

#if 0
        fprintf (stderr, "Done getting tfparams\n");
#endif

        // Return the parameters for the instruction:
        return (result);
      }

      // Override meet operator
      BitVector meet(BitVector left, const BitVector& right) {
        return left |= right;
      }
  };

  DeadCodeElimination() : FunctionPass(ID) { }

  virtual bool runOnFunction(Function& F) {

    // Make one pass through to find all variables.
    std::vector<Instruction*> varList;
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

    // Get the vector containing live instructions :
    Instruction* first_inst = &(F.getEntryBlock().front());

    // At the end of analysis, bit vector before the first instr
    // contains live instruction - because of SSA representation.
    BitVector live_insts = result[first_inst];

    bool modified = false;

    // Print the dead instructions :
    for (unsigned i = 0; i < live_insts.size(); ++i) {
        if (live_insts[i] == false) {
          modified = true;
          Instruction *instr = varList[i];
          instr->eraseFromParent();
          //instr->print(errs());
          //errs()<< "\n";
        }
    }

    // the incoming Function is modified.
    return modified;
  }

  virtual void getAnalysisUsage(AnalysisUsage& AU) const {
    AU.setPreservesCFG();
  }

 private:
};

char DeadCodeElimination::ID = 0;
RegisterPass<DeadCodeElimination> X("cd-dce", "15745 Dead-Code-Elimination");

}
