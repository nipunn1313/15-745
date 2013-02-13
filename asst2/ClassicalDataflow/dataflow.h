// 15-745 S13 Assignment 2: dataflow.h
// Group: nkoorapa, pdixit
////////////////////////////////////////////////////////////////////////////////

#ifndef __CLASSICAL_DATAFLOW_DATAFLOW_H__
#define __CLASSICAL_DATAFLOW_DATAFLOW_H__

#include <stdio.h>

#include "llvm/Instructions.h"
#include "llvm/ADT/BitVector.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/ValueMap.h"
#include "llvm/Support/CFG.h"

#include <deque>
#include <map>

namespace llvm {

class DataFlow {
  protected:
  enum Direction {
    FORWARDS,
    BACKWARDS
  };
  BitVector emptySet(int s) {
    return BitVector(s, false);
  }
  BitVector universalSet(int s) {
    return BitVector(s, true);
  }

  private:
  typedef std::pair<BitVector, BitVector> BVPair;

  BitVector boundary;
  BitVector top;
  Direction direction;

  void postOrder(BasicBlock* bb, std::deque<BasicBlock*> &q,
                 std::map<BasicBlock*, bool> &visited);
  BitVector instTransferFunc(BasicBlock* bb, BitVector before,
        std::map<Instruction*, BitVector>& map);

  public:
  DataFlow(BitVector b, BitVector t, Direction d) :
    boundary(b), top(t), direction(d) {}
  ~DataFlow() {}

  virtual BitVector transferFunction(Instruction* inst, BitVector before) = 0;
  virtual BitVector transferFunctionBB(BasicBlock* bb, BitVector before);
  virtual BitVector meet(BitVector left, const BitVector& right) = 0;

  std::map<Instruction*, BitVector> doAnalysis(Function& f);
};

BitVector DataFlow::transferFunctionBB(BasicBlock* bb, BitVector before) {
  std::map<Instruction*, BitVector> dummy;
  return instTransferFunc(bb, before, dummy);
}

BitVector DataFlow::instTransferFunc(BasicBlock* bb, BitVector before,
      std::map<Instruction*, BitVector>& map) {
  BitVector curr = before;

  if (direction == FORWARDS) {
    for (BasicBlock::iterator iter = bb->begin(), end = bb->end();
        iter != end; ++iter) {
      Instruction* inst = iter;
      curr = transferFunction(inst, curr);
      map[inst] = curr;
    }
  } else {
    BasicBlock::InstListType& il = bb->getInstList();
    for (BasicBlock::InstListType::reverse_iterator iter = il.rbegin(),
        end = il.rend(); iter != end; ++iter) {
      Instruction* inst = &(*iter); // Stupid STL
      curr = transferFunction(inst, curr);
      map[inst] = curr;
    }
  }

  return curr;
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


  // iterate through each basic block to get BitVector for each inst
  std::map<Instruction*, BitVector> result;
  for (std::map<BasicBlock*, BVPair>::iterator iter = bbStartEnd.begin(),
       end = bbStartEnd.end(); iter != end; ++iter) {
    BitVector& before = (direction == FORWARDS) ? iter->second.first :
                        iter->second.second;
    instTransferFunc(iter->first, before, result);
  }

  return result;
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

// Prints a representation of F to raw_ostream O.
void ExampleFunctionPrinter(raw_ostream& O, const Function& F);

}

#endif
