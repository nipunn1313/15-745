// 15-745 S13 Assignment 2: dataflow.cpp
// Group: bovik, bovik2
////////////////////////////////////////////////////////////////////////////////

#include "dataflow.h"
#include "llvm/ADT/BitVector.h"

namespace llvm {

virtual class DataFlow {
  private:
  BitVector boundary;
  BitVector top;
  Direction direction;

  typedef std::pair<BitVector, BitVector> BVPair;
  enum Direction {
    FORWARDS,
    BACKWARDS
  };

  BitVector emptySet(int s) {
    BitVector(s, false);
  }
  BitVector universalSet(int s) {
    BitVector(s, true);
  }

  public:
  DataFlow(BitVector& boundary, BitVector& top, Direction direction) {
    this->boundary = boundary;
    this->top = top;
    this->forwards = forwards;
  }

  virtual BitVector transferFunction(Instruction* inst, BitVector before);
  virtual BitVector transferFunction(BasicBlock* bb, BitVector before) {
    BitVector curr = before;
    if (direction == FORWARDS) {
      for (BasicBlock::iterator iter = bb.begin(), end = bb.end();
           iter != end; ++iter) {
        Instruction* inst = iter;
        curr = transferFunction(inst, curr);
      }
    } else {
      for (BasicBlock::reverse_iterator iter = bb.rbegin(), end = bb.rend();
           iter != end; ++iter) {
        Instruction* inst = iter;
        curr = transferFunction(inst, curr);
      }
    }
  }

  // TODO Maybe take an array?
  virtual BitVector meet(BitVector left, BitVector right);

  std::map<Instruction*, BitVector> doAnalysis(Function& f) {
    std::map<BasicBlock*, BVPair> bbStartEnd;
    std::vector<BasicBlock*> exitBlocks;
    BasicBlock* entryBlock = &(f.getEntryBlock());

    // Initialize every basic block's bit vectors to TOP
    for (Function::iterator iter = f.begin(), end = f.end();
         iter != end; ++iter) {
      BasicBlock* bb = iter;
      bbStartEnd[bb] = BVPair(top, top);

      // Find all the exit blocks to this function
      //TODO make sure this check actually gets exit blocks
      if (bb->getTerminator()->getNumSuccessors() == 0) {
        exitBlocks.push_back(bb);
      }
    }

    std::queue<BasicBlock*> worklist;

    // Initialize worklist to either forward or reverse postorder
    std::map<BasicBlock*, bool> visited;
    postOrder(entryBlock, worklist, visited);
    if (direction == FORWARDS) {
      std::reverse(worklist.begin(), worklist.end());
    }

    // iterate until convergence
    visited.clear();
    while (!worklist.empty()) {
      BasicBlock* bb = worklist.front();
      BVPair& bvp = bbStartEnd[bb];
      worklist.pop();

      BitVector& before = (direction == FORWARDS) ?
                          bvp.first : bvp.second;
      BitVector& after = (direction == FORWARDS) ?
                          bvp.second : bvp.first;

      if (direction == FORWARDS) {
        for (pred_iterator iter = pred_begin(bb), end = pred_end(bb)) {
          before = meet(before, bbStartEnd[iter].second);
        }
        after = transferFunction(bb, before);
        for (succ_iterator iter = succ_begin(bb), end = succ_end(bb)) {
          BasicBlock* succBb = iter;
          BVpair& succBvp = bbStartEnd[succBb];
          if (succBvp.first != after) {
            succBvp.first = after;
            if (visited[succBb]) {
              visited[succBb] = false;
              worklist.push(succBb);
            }
          }
        }
      } else {
        for (succ_iterator iter = succ_begin(bb), end = succ_end(bb)) {
          before = meet(before, bbStartEnd[iter].first);
        }
        after = transferFunction(bb, before);
        for (pred_iterator iter = pred_begin(bb), end = pred_end(bb)) {
          BasicBlock* predBb = iter;
          BVpair& predBvp = bbStartEnd[predBb];
          if (predBvp.second != after) {
            predBvp.second = after;
            if (visited[predBb]) {
              visited[predBb] = false;
              worklist.push(predBb);
            }
          }
        }
      }

      visited[bb] = true;
    }


    // Step 3: iterate through each basic block to get BitVector for each inst
  }

  void postOrder(BasicBlock* bb, std::queue<BasicBlock*> &q,
                 std::map<BasicBlock*, bool> &visited) {
    visited[bb] = true;
    for (child of bb) {
      if (!visited[child]) {
        postOrder(child, q, visited);
      }
    }
    q.push(bb);
  }
}

void PrintInstructionOps(raw_ostream& O, const Instruction* I) {
  O << "\nOps: {";
  if (I != NULL) {
    for (Instruction::const_op_iterator OI = I->op_begin(), OE = I->op_end();
        OI != OE; ++OI) {
      const Value* v = OI->get();
      v->print(O);
      O << ";";
    }
  }
  O << "}\n";
}

void ExampleFunctionPrinter(raw_ostream& O, const Function& F) {
  for (Function::const_iterator FI = F.begin(), FE = F.end(); FI != FE; ++FI) {
    const BasicBlock* block = FI;
    O << block->getName() << ":\n";
    const Value* blockValue = block;
    PrintInstructionOps(O, NULL);
    for (BasicBlock::const_iterator BI = block->begin(), BE = block->end();
        BI != BE; ++BI) {
      BI->print(O);
      PrintInstructionOps(O, &(*BI));
    }
  }
}

}
