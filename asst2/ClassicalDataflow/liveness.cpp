// 15-745 S13 Assignment 2: liveness.cpp
// Group: bovik, bovik2
////////////////////////////////////////////////////////////////////////////////

#include "llvm/Function.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"

#include "dataflow.h"

using namespace llvm;

namespace {

class Liveness : public FunctionPass {
 public:
  static char ID;

  class LivenessDF : DataFlow {
    private:
      std::map<Value*, int>* map;

    public:
      Liveness(std::map<Value*, int>* map) :
      DataFlow(emptySet(map->size()), emptySet(map->size()), BACKWARDS) {
      }

      virtual BitVector transferFunction(Instruction inst, BitVector before) {
        // (before - kill) U vars_used

          // TODO
      }
  }

  Liveness() : FunctionPass(ID) { }

  virtual bool runOnFunction(Function& F) {
    //ExampleFunctionPrinter(errs(), F);

    // Make one pass through to find all variables. create map

    LivenessDF ldf(map);
    ldf.doAnalysis();

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
