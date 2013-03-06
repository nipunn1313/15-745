// 15-745 S13 Assignment 2: dataflow.h
// Group: nkoorapa, pdixit
///////////////////////////////////////////////////////////////////////////////

#ifndef __CLASSICAL_DATAFLOW_DATAFLOW_H__
#define __CLASSICAL_DATAFLOW_DATAFLOW_H__

#include <stdio.h>

#include "llvm/Instructions.h"
#include "llvm/ADT/BitVector.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/ValueMap.h"
#include "llvm/Support/CFG.h"

#include "llvm/Support/raw_ostream.h"

#include <deque>
#include <map>

namespace llvm {

// Generic dataflow framework template. The template variable is Param_t
// which is the type defined by a user of the framework. Param_t is the
// type for property of an instruction. For example, both liveness and
// reaching definition use a pair of Gen and Kill sets (vectors) as Param_t
// A variable of type Param_t is associated with each instruction and
// does not change throughout the analysis, so it can be cached.
// Generic transfer function takes input bit-vector and Param_t as input
// More description is in the write-up.
template<class Param_t>
class DataFlow {
  protected:

  // To identify direction of analysis
  enum Direction {
    FORWARDS,
    BACKWARDS
  };

  // Useful notation to represent empty set
  BitVector emptySet(int s) {
    return BitVector(s, false);
  }

  // Useful notation to represent universal set
  BitVector universalSet(int s) {
    return BitVector(s, true);
  }

  private:

  // Useful shorthand
  typedef std::pair<BitVector, BitVector> BVPair;

  // Maps to cache parameter values for instructions and basic blocks
  // For example, these will cache GenKill sets for each instr/block
  std::map<Instruction*, Param_t> paramCache;
  std::map<BasicBlock*, Param_t> bbParamCache;

  // Variables which identify conditions for specific analysis
  BitVector boundary;
  BitVector top;
  Direction direction;

  void postOrder(BasicBlock* bb, std::deque<BasicBlock*> &q,
                 std::map<BasicBlock*, bool> &visited);

  // Generic transfer function for block, takes basic block,
  // Input bitvector and a reference to instruction parameters
  // cache. Returns the output bitvector
  BitVector instTransferFunc(BasicBlock* bb, BitVector input,
        std::map<Instruction*, BitVector>& map);

  // Returns cached values of parameters for an instruction
  Param_t getCachedTFParams(Instruction* inst);

  public:

  // Constructor - needs boundary, top and direction of analysis
  DataFlow(BitVector b, BitVector t, Direction d) :
    boundary(b), top(t), direction(d) {}
  ~DataFlow() {}

  // Transfer function per instruction and per basic-block.
  // They use cached value of parameters while applying transfer-function
  // If parameters were never obtained then they are calculated and cached
  BitVector transferFunction(Instruction* inst, BitVector input);
  BitVector transferFunctionBB(BasicBlock* bb, BitVector input);

  virtual BitVector transferFunctionParams(Param_t params, BitVector input) = 0;
  virtual Param_t getTFParams(Instruction* inst) = 0;
  virtual Param_t compose(Param_t a, Param_t b) = 0;
  virtual BitVector meet(BitVector left, const BitVector& right) = 0;

  // The top level function. Call this to run the dataflow engine:
  std::map<Instruction*, BitVector> doAnalysis(Function& f);
};

// Get Transfer-function parameters for instruction.
// If never calculated previously calculate and cache
// Return gen-kill sets for instructions
template<class Param_t> Param_t
DataFlow<Param_t>::getCachedTFParams(Instruction* inst) {
  typename std::map<Instruction*, Param_t>::iterator it;
  it = paramCache.find(inst);
  if (it != paramCache.end()) {
    return it->second;
  } else {
    Param_t params = getTFParams(inst);
    paramCache[inst] = params;
    return params;
  }
}

// Transfer function for instruction. Calls transferFunctionParams
// with parameter for the instruction and input bit-vector
// transferFunctionParams is defined in specific implementation
template<class Param_t> BitVector
DataFlow<Param_t>::transferFunction(Instruction* inst,
                                    BitVector input) {
  return transferFunctionParams(getCachedTFParams(inst), input);
}

// Transfer function at basic block level. If it was called previously,
// TFParams for the basic block should be available in cache. Otherwise,
// it calls 'compose' function for instructions to get TFParams for basic
// block
template<class Param_t> BitVector
DataFlow<Param_t>::transferFunctionBB(BasicBlock* bb, BitVector input) {
  Param_t params;
  typename std::map<BasicBlock*, Param_t>::iterator it;
  typename std::map<Instruction*, Param_t>::iterator it2;

  // If cached value of basic block TFParams is available, use that to quickly
  // get the output vector
  it = bbParamCache.find(bb);
  if (it != bbParamCache.end()) {
    return transferFunctionParams(it->second, input);
  }

  // If cached value is not available, compose the TFParams of
  // instructions together to get TFParams for the basic block
  // For example, gen and kill sets of instructions can be composed
  // to form gen and kill set for a basic-block. It is users responsibility
  // to define the composition function.
  params = getTFParams(NULL);
  for (BasicBlock::iterator iter = bb->begin(), end = bb->end();
       iter != end; ++iter) {

    Instruction* inst = iter;
    Param_t instParam = getCachedTFParams(inst);
    if (direction == FORWARDS) {
      params = compose(params, instParam);
    } else {
      params = compose(instParam, params);
    }
  }

  // Cache the value
  bbParamCache[bb] = params;

  // With the TFParams now available, calculate the output vector using
  // these and input vector.
  return transferFunctionParams(params, input);
}

// doAnalysis on the function class passed as input and return
// map from instruction to the output for each instruction.
template<class Param_t> std::map<Instruction*, BitVector>
DataFlow<Param_t>::doAnalysis(Function& f) {
  std::map<BasicBlock*, BVPair> bbStartEnd;
  std::vector<BasicBlock*> exitBlocks;
  BasicBlock* entryBlock = &(f.getEntryBlock());

  // Initialize every basic block's output bit vectors to TOP
  for (Function::iterator iter = f.begin(), end = f.end();
      iter != end; ++iter) {
    BasicBlock* bb = iter;
    bbStartEnd[bb] = BVPair(top, top);

    // Find all the exit blocks to this function
    if (bb->getTerminator()->getNumSuccessors() == 0) {
      exitBlocks.push_back(bb);
    }
  }

  // Initialize the boundary for entry or exit blocks depending on
  // direction of analysis.
  if (direction == FORWARDS) {
    bbStartEnd[entryBlock].first = boundary;
  } else {
    for (std::vector<BasicBlock*>::iterator iter = exitBlocks.begin(),
        end = exitBlocks.end(); iter != end; ++iter) {
      bbStartEnd[*iter].second = boundary;
    }
  }

  // Worklist to be used when iterating over basic blocks
  std::deque<BasicBlock*> worklist;

  // Initialize worklist to either forward or reverse postorder
  std::map<BasicBlock*, bool> visited;
  postOrder(entryBlock, worklist, visited);
  if (direction == FORWARDS) {
    std::reverse(worklist.begin(), worklist.end());
  }

  // iterate until convergence
  visited.clear();  // visited in the code below means, block is not present in work-list
                    // So when input to a block changes and it needs to be re-visited,
                    // this variable is set to false for that block
  while (!worklist.empty()) {

    // Get first block to analyze
    BasicBlock* bb = worklist.front();
    BVPair& bvp = bbStartEnd[bb];
    worklist.pop_front();

    // input represents input to a basic block analysis
    BitVector& input = (direction == FORWARDS) ?
      bvp.first : bvp.second;
    // output represents output of transfer function due to basic block
    // so it may be 'before' a basic block for backward analysis
    BitVector& output = (direction == FORWARDS) ?
      bvp.second : bvp.first;

    if (direction == FORWARDS) { // Forward analysis
      // Get input value for each basic block by meet on all outputs
      for (pred_iterator iter = pred_begin(bb), end = pred_end(bb);
          iter != end; ++iter) {
        input = meet(input, bbStartEnd[*iter].second);
      }

      // Compute the output using transfer function for this basic block
      output = transferFunctionBB(bb, input);

      // Iterate over all the successors to see if input for any block
      // changed. Add them to worklist if it changed and block is not
      // already in work-list (i.e. visited is true)
      for (succ_iterator iter = succ_begin(bb), end = succ_end(bb);
          iter != end; ++iter) {
        BasicBlock* succBb = *iter;
        BVPair& succBvp = bbStartEnd[succBb];
        if (succBvp.first != output) {
          succBvp.first = output;
          if (visited[succBb]) {
            visited[succBb] = false;
            worklist.push_back(succBb);
          }
        }
      }
    } else { // Backward analysis

      // Obtain input by meet of outputs for all sucessors
      for (succ_iterator iter = succ_begin(bb), end = succ_end(bb);
          iter != end; ++iter) {
        input = meet(input, bbStartEnd[*iter].first);
      }

      fprintf (stderr, "\nItarating on basic block : \n");
      bb->print(errs());
      fprintf (stderr, "basic-block end\n\n");

      // Apply transfer function to get output bit vector
      output = transferFunctionBB(bb, input);

      // Iterate over all the predecessors and add them to work-list
      // if input to any of them changed. Don't add to work-list if
      // they are already present there.
      for (pred_iterator iter = pred_begin(bb), end = pred_end(bb);
          iter != end; ++iter) {
        BasicBlock* predBb = *iter;
        BVPair& predBvp = bbStartEnd[predBb];
        if (predBvp.second != output) {
          predBvp.second = output;
          if (visited[predBb]) {
            visited[predBb] = false;
            worklist.push_back(predBb);
          }
        }
      }
    }

    // Mark the block visited.
    visited[bb] = true;
  }


  // Last pass to create output for each instruction once BB inputs are stable
  // iterate through each basic block to get BitVector for each inst
  std::map<Instruction*, BitVector> result;
  for (std::map<BasicBlock*, BVPair>::iterator iter = bbStartEnd.begin(),
       end = bbStartEnd.end(); iter != end; ++iter) {
    BasicBlock* bb = iter->first;
    if (direction == FORWARDS) {
      BitVector& curr = iter->second.first;
      for (BasicBlock::iterator bbIt = bb->begin(),
           bbEnd = bb->end(); bbIt != bbEnd; ++bbIt) {
        Instruction* inst = bbIt;
        curr = transferFunction(inst, curr);
        result[inst] = curr;
      }
    } else {
      BitVector& curr = iter->second.second;
      BasicBlock::InstListType& il = bb->getInstList();
      for (BasicBlock::InstListType::reverse_iterator bbIt = il.rbegin(),
           bbEnd = il.rend(); bbIt != bbEnd; ++bbIt) {
        Instruction* inst = &(*bbIt); // Stupid STL
        curr = transferFunction(inst, curr);
        result[inst] = curr;
      }
    }
  }

  return result;
}

// Read the basic blocks and return a queue in postOrder that can be
// reversed for Forward problems and used as it is for backward problems
// to minimize the iteration on basic-blocks while computing transfer
// function
template<class Param_t> void
DataFlow<Param_t>::postOrder(BasicBlock* bb, std::deque<BasicBlock*> &q,
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
