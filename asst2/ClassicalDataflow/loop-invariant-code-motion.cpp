#include "llvm/Pass.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/LoopPass.h"

#include "dataflow.h"

#include <iostream>

using namespace llvm;

namespace {

class LICM : public LoopPass {
 public:
  static char ID;

  typedef std::map<BasicBlock*, int> block_idxmap_t;

  // We're just going to use idx into bb map as param
  class DominatorDF : public DataFlow<int> {
    private:
      block_idxmap_t* map;
    public:
      DominatorDF(block_idxmap_t* map) :
      DataFlow<int>(emptySet(map->size()),
                    universalSet(map->size()),
                    FORWARDS) {
        this->map = map;
      }

      BitVector transferFunctionParams(int paramIdx, BitVector input) {
        input[paramIdx] = true;
        return input;
      }

      int getTFParams(Instruction* inst) {
        if (inst == NULL) return -1;
        return (*map)[inst->getParent()];
      }

      // This is stupid. We didn't set up a good way to skip instruction
      // level summaries
      int compose(int a, int b) {
        if (a == -1) return b;
        if (b == -1) return a;
        // only ever compose instructions within a BB
        assert(a == b);
        return a;
      }

      BitVector meet(BitVector left, const BitVector& right) {
        return left &= right;
      }
  };

  LICM() : LoopPass(ID) {}

  virtual bool runOnLoop(Loop* L, LPPassManager &LPM) {
    // Ignore loops without preheaders
    if (L->getLoopPreheader() == NULL) {
      return false;
    }

    Function* F = L->getHeader()->getParent();

    block_idxmap_t idxMap;
    std::vector<BasicBlock*> bbVect;

    // Assign indices to blocks
    int idx=0;
    for (Function::iterator iter = F->begin(),
         end = F->end(); iter != end; ++iter) {
      BasicBlock* bb = iter;
      bbVect.push_back(bb);
      idxMap[bb] = idx;
      idx++;
    }

    // Do analysis to find total dominators
    std::map<Instruction*, BitVector> result;
    DominatorDF ddf(&idxMap);
    result = ddf.doAnalysis(*F);

    // Iterate to print out result
    for (Loop::block_iterator iter = L->block_begin(),
         end = L->block_end(); iter != end; ++iter) {
      BasicBlock* bb = *iter;
      Instruction* first = &(bb->front());
      BitVector& dominated = result[first];
      unsigned myIdx = idxMap[bb];

      // Find immediate dominator from list of dominators
      BasicBlock* idom = &F->getEntryBlock();
      for (unsigned i=0; i<dominated.size(); i++) {
        if (i != myIdx && dominated[i]) {
          BasicBlock* dominator = bbVect[i];
          BitVector& domBV = result[&dominator->front()];
          if (domBV[idxMap[idom]]) {
            idom = dominator;
          }
        }
      }

      std::cerr << bb->getName().str() << " idom " <<
                   idom->getName().str() << "\n";
    }

    // TODO For now. Need to be true if we modify
    return false;
  }

  virtual void getAnalysisUsage(AnalysisUsage& AU) const {
    // TODO???
    AU.setPreservesAll();
  }

};

char LICM::ID = 0;
RegisterPass<LICM> X("cd-licm", "15745 LICM");

}
