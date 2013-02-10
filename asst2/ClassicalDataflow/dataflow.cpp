// 15-745 S13 Assignment 2: dataflow.cpp
// Group: nkoorapa, pdixit
///////////////////////////////////////////////////////////////////////////////

#include "dataflow.h"

#include "llvm/ADT/BitVector.h"
#include "llvm/ADT/ilist.h"
#include "llvm/BasicBlock.h"

#include <deque>

namespace llvm {

  BitVector DataFlow::transferFunctionBB(BasicBlock* bb, BitVector before) {
      BitVector curr = before;
      if (direction == FORWARDS) {
        for (BasicBlock::iterator iter = bb->begin(), end = bb->end();
            iter != end; ++iter) {
          Instruction* inst = iter;
          curr = transferFunction(inst, curr);
        }
      } else {
        BasicBlock::InstListType& il = bb->getInstList();
        for (BasicBlock::InstListType::reverse_iterator iter = il.rbegin(),
            end = il.rend(); iter != end; ++iter) {
          Instruction* inst = &(*iter); // Stupid STL
          curr = transferFunction(inst, curr);
        }
      }
    }

  std::map<Instruction*, BitVector> DataFlow::doAnalysis(Function& f) {
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
    if (direction == FORWARDS) {
      bbStartEnd[entryBlock].first = boundary;
    } else {
      for (std::vector<BasicBlock*>::iterator iter = exitBlocks.begin(),
          end = exitBlocks.end(); iter != end; ++iter) {
        bbStartEnd[*iter].second = boundary;
      }
    }

    std::deque<BasicBlock*> worklist;

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
      worklist.pop_front();

      BitVector& before = (direction == FORWARDS) ?
        bvp.first : bvp.second;
      BitVector& after = (direction == FORWARDS) ?
        bvp.second : bvp.first;

      if (direction == FORWARDS) {
        for (pred_iterator iter = pred_begin(bb), end = pred_end(bb);
            iter != end; ++iter) {
          before = meet(before, bbStartEnd[*iter].second);
        }
        after = transferFunctionBB(bb, before);
        for (succ_iterator iter = succ_begin(bb), end = succ_end(bb);
            iter != end; ++iter) {
          BasicBlock* succBb = *iter;
          BVPair& succBvp = bbStartEnd[succBb];
          if (succBvp.first != after) {
            succBvp.first = after;
            if (visited[succBb]) {
              visited[succBb] = false;
              worklist.push_back(succBb);
            }
          }
        }
      } else {
        for (succ_iterator iter = succ_begin(bb), end = succ_end(bb);
            iter != end; ++iter) {
          before = meet(before, bbStartEnd[*iter].first);
        }
        after = transferFunctionBB(bb, before);
        for (pred_iterator iter = pred_begin(bb), end = pred_end(bb);
            iter != end; ++iter) {
          BasicBlock* predBb = *iter;
          BVPair& predBvp = bbStartEnd[predBb];
          if (predBvp.second != after) {
            predBvp.second = after;
            if (visited[predBb]) {
              visited[predBb] = false;
              worklist.push_back(predBb);
            }
          }
        }
      }

      visited[bb] = true;
    }


    // Step 3: iterate through each basic block to get BitVector for each inst
  }

  void DataFlow::postOrder(BasicBlock* bb, std::deque<BasicBlock*> &q,
      std::map<BasicBlock*, bool> &visited) {
    visited[bb] = true;
    for (succ_iterator iter = succ_begin(bb), end = succ_end(bb);
        iter != end; ++iter) {
      BasicBlock* child = *iter;
      if (!visited[child]) {
        postOrder(child, q, visited);
      }
    }
    q.push_back(bb);
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
