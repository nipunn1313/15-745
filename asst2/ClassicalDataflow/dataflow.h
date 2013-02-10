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
    BitVector(s, false);
  }
  BitVector universalSet(int s) {
    BitVector(s, true);
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

// Prints a representation of F to raw_ostream O.
void ExampleFunctionPrinter(raw_ostream& O, const Function& F);

}

#endif
