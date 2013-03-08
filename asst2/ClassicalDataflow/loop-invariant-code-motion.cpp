#include "llvm/Pass.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/Analysis/ValueTracking.h"

#include "dataflow.h"

#include <iostream>
#include <queue>

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

  void printDominatorInfo(Loop* L) {
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
  }

  bool isLoopInvariant(Instruction* inst,
                       std::map<Instruction*, bool> inLoop) {
    //inst->print(errs()); fprintf(stderr, "\n");
    if (!isSafeToSpeculativelyExecute(inst) ||
        inst->mayReadFromMemory() ||
        isa<LandingPadInst>(inst)) {
      return false;
    }

    for (Instruction::op_iterator iter = inst->op_begin(),
         end = inst->op_end(); iter != end; ++iter) {
      Use* use = iter;
      Instruction* useInst = (Instruction*) use->get();
#if 0
      fprintf(stderr, "  Use = ");
      useInst->print(errs());
      fprintf(stderr, "\n");
#endif
      if (inLoop[useInst]) {
        //fprintf(stderr, "op is in loop!\n");
        return false;
      }
    }
    return true;
  }

  virtual bool runOnLoop(Loop* L, LPPassManager &LPM) {
    BasicBlock* preHeader = L->getLoopPreheader();
    // Ignore loops without preheaders
    if (preHeader == NULL) {
      return false;
    }

    printDominatorInfo(L);

    // Now do LICM

    // Pick instructions to hoist
    std::map<Instruction*, bool> inLoop;
    std::vector<Instruction*> toHoist;
    std::queue<Instruction*> worklist;

    // Initialize worklist to all instructions
    // Initialize inLoop to all instructions
    for (Loop::block_iterator i1 = L->block_begin(),
         e1 = L->block_end(); i1 != e1; ++i1) {
      BasicBlock* bb = *i1;
      for (BasicBlock::iterator i2 = bb->begin(),
           e2 = bb->end(); i2 != e2; ++i2) {
        Instruction* i = i2;
        worklist.push(i);
        inLoop[i] = true;
      }
    }

    // Go through worklist hoisting when appropriate. When hoisting, put uses
    // back in the worklist as they may become hoistable
    while (!worklist.empty()) {
      Instruction* inst = worklist.front();
      worklist.pop();
      if (inLoop[inst] && isLoopInvariant(inst, inLoop)) {
        toHoist.push_back(inst);
        inLoop[inst] = false;
        // Go back to look at this instruction's uses
        for (Instruction::use_iterator ui = inst->use_begin(),
             ue = inst->use_end(); ui != ue; ++ui) {
          Instruction* use = (Instruction*) *ui;
          worklist.push(use);
        }
      }
    }

    // Track whether we modified the code
    bool modified = false;

#if 0
    std::cerr << "To hoist:\n";
#endif
    for (std::vector<Instruction*>::iterator it = toHoist.begin(),
         end = toHoist.end(); it != end; ++it) {
      Instruction* inst = *it;
      inst->removeFromParent();
      inst->insertBefore(preHeader->getTerminator());
      modified = true;
#if 0
      inst->print(errs());
      std::cerr << "\n";
#endif
    }

    // Return true if some instructions were hoisted from the loop
    return modified;
  }

  virtual void getAnalysisUsage(AnalysisUsage& AU) const {
    AU.setPreservesCFG();
    AU.addRequired<LoopInfo>(); // Helps us by inserting preheaders
  }

};

char LICM::ID = 0;
RegisterPass<LICM> X("cd-licm", "15745 LICM");

}
