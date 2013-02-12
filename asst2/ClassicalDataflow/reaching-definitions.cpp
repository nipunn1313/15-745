// 15-745 S13 Assignment 2: reaching-definitions.cpp
// Group: bovik, bovik2
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

  typedef std::map<Value*, std::vector<Instruction*> > defmap_t;
  typedef std::map<Value*, int> idxmap_t;
  typedef std::pair<BitVector, BitVector> GKPair;

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
                                               BitVector before) {
        return before.reset(params.second) |= params.first;
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

      // Override
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

      // Override
      BitVector meet(BitVector left, const BitVector& right) {
        return left |= right;
      }
  };

  ReachingDefinitions() : FunctionPass(ID) { }

  virtual bool runOnFunction(Function& F) {
    //ExampleFunctionPrinter(errs(), F);

    std::vector<Value*> varList;
    idxmap_t idxMap;
    defmap_t defMap;

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
