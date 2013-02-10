// 15-745 S13 Assignment 2: liveness.cpp
// Group: nkoorapa, pdixit
////////////////////////////////////////////////////////////////////////////////

#include "llvm/Function.h"
#include "llvm/Pass.h"
#include "llvm/Support/InstIterator.h"
#include "llvm/Support/raw_ostream.h"

#include "dataflow.h"

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
      }

      virtual BitVector transferFunction(Instruction* inst, BitVector before) {
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
    std::map<Value*, int>* varMap;

    // Run analysis to get instruction -> bitmap
    LivenessDF ldf(varMap);
    std::map<Instruction*, BitVector> result = ldf.doAnalysis(F);

    // Output!
    for (inst_iterator iter = inst_begin(F), end = inst_end(F);
         iter != end; ++iter) {
      Instruction* inst = &(*iter);
      fprintf(stderr, "Live: { ");
      BitVector& bits = result[inst];
      for (int i=0; i<bits.size(); i++) {
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
