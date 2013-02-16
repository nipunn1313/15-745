// 15-745 S13 Assignment 2: reaching-definitions.cpp
// Group: nkoorapa, pdixit 
// Description : This file implements reaching definition analysis by exploiting
//               generic framework in dataflow.h This also serves as an example
//               of how to use the generic framework. 
//
//               Following things should be noticed when refering to it as an 
//               example when writing another pass : 
// 
//               1. Tranfer-function parameter type is defined as GKPair which 
//                  is pair of gen and kill sets that can be associated to each
//                  instruction or basic block. This should be defined.
//
//               2. "RD_DF" is the main dataflow class which extends DataFlow 
//                  with GKPair template parameter. It uses constructor arguments 
//                  to indicate that RD is a FORWARD analysis with emptySet as 
//                  the TOP and boundary values.
//          
//               3. Virtual function "transferFunctionParams" is implemented to 
//                  calculate output = ( input - kill ) U gen
//
//               4. Virtual function "compose" is implemented to calculate gen
//                  & kill sets of consecutive instructions (or instr & block)
//              
//               5. Meet operator function is overriden to give union
//              
//               6. getTFParams is overriden to return gen and kill sets for 
//                  each instruction based on how they are defined for reaching-
//                  definitions
////////////////////////////////////////////////////////////////////////////////

#include "llvm/Function.h"
#include "llvm/Pass.h"
#include "llvm/Support/InstIterator.h"
#include "llvm/Support/raw_ostream.h"

#include "dataflow.h"

using namespace llvm;

namespace {

class ReachingDefinitions : public FunctionPass {
 public:
  static char ID;

  // Map from memory location (pointer operand) to instruction pointer 
  typedef std::map<Value*, std::vector<Instruction*> > defmap_t;

  // Map from a definition to it's index in vector of all definitions
  typedef std::map<Value*, int> idxmap_t;
  typedef std::pair<BitVector, BitVector> GKPair;

  // Inherit the DataFlow class 
  class RD_DF : public DataFlow<GKPair> {

    private:
      defmap_t& defMap;
      idxmap_t& idxMap;

    public:
      RD_DF(idxmap_t& idxMap, defmap_t& defMap) :
        DataFlow<GKPair>(emptySet(idxMap.size()),
                         emptySet(idxMap.size()), FORWARDS),
         defMap(defMap), idxMap(idxMap) {}
      ~RD_DF() {}

      // Override. (in - kill) U gen
      BitVector transferFunctionParams(GKPair params,
                                               BitVector input) {
        return input.reset(params.second) |= params.first;
      }

      // Override composition
      GKPair compose(GKPair a, GKPair b) {
        // gen = (a.gen - b.kill) U b.gen
        BitVector gen = a.first;
        gen.reset(b.second) |= b.first;
        // kill = a.kill U b.kill
        BitVector kill = a.second;
        kill |= b.second;
        return GKPair::pair(gen, kill);
      }

      // Override to return Gen and Kill sets for each instruction
      GKPair getTFParams(Instruction* inst) {
        BitVector gen = emptySet(idxMap.size());
        BitVector kill = emptySet(idxMap.size());

        if (inst == NULL) {
          // Do nothing. Identity function on no instruction is empty gen/kill
        } else if (StoreInst* storeInst = dyn_cast<StoreInst>(inst)) {
          std::vector<Instruction*>& vect =
            defMap[storeInst->getPointerOperand()];
          for (std::vector<Instruction*>::iterator it = vect.begin(),
               end = vect.end(); it != end; ++it) {
            kill[idxMap[*it]] = true;
          }

          gen[idxMap[inst]] = true;
        }

        return GKPair::pair(gen, kill);
      }

      // Override to implement union
      BitVector meet(BitVector left, const BitVector& right) {
        return left |= right;
      }
  };

  ReachingDefinitions() : FunctionPass(ID) { }

  virtual bool runOnFunction(Function& F) {

    // Create variable list and their mappings to instructions
    std::vector<Value*> varList;
    idxmap_t idxMap;
    defmap_t defMap;

    // For each store instruction, 
    for (inst_iterator iter = inst_begin(F), end = inst_end(F);
         iter != end; ++iter) {
      Instruction* inst = &(*iter);
      if (StoreInst* storeInst = dyn_cast<StoreInst>(inst)) {
        idxMap[inst] = 0;
        defMap[storeInst->getPointerOperand()].push_back(inst);
      }
    }
    // Assign indices to the variables in this function
    int idx = 0;
    for (idxmap_t::iterator iter = idxMap.begin(),
         end = idxMap.end(); iter != end; ++iter) {
      varList.push_back(iter->first);
      iter->second = idx;
      idx++;
    }

    // Run analysis to get instruction -> bitmap
    RD_DF rddf(idxMap, defMap);
    std::map<Instruction*, BitVector> result = rddf.doAnalysis(F);

    // Output!
    fprintf(stderr, "Reach: {}\n");
    for (inst_iterator iter = inst_begin(F), end = inst_end(F);
         iter != end; ++iter) {
      Instruction* inst = &(*iter);
      inst->print(errs());
      fprintf(stderr, "\nReach: {");
      BitVector& bits = result[inst];
      for (unsigned i=0; i<bits.size(); i++) {
        if (bits[i]) {
          Value* v = varList[i];
          v->print(errs());
          fprintf(stderr, "; ");
        }
      }
      fprintf(stderr, "}\n");
    }

    // Did not modify the incoming Function.
    return false;

  }

  virtual void getAnalysisUsage(AnalysisUsage& AU) const {
    AU.setPreservesCFG();
  }

 private:
};

char ReachingDefinitions::ID = 0;
RegisterPass<ReachingDefinitions> X("cd-reaching-definitions",
    "15745 ReachingDefinitions");

}
