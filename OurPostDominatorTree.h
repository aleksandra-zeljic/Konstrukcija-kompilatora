#ifndef LLVM_PROJECT_OURPOSTDOMINATORTREE_H
#define LLVM_PROJECT_OURPOSTDOMINATORTREE_H

#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace llvm;

class OurPostDominatorTree {
private:
 
  std::unordered_map<BasicBlock *, std::unordered_set<BasicBlock *>> PostDom;

  std::unordered_map<BasicBlock *, BasicBlock *> IPostDom;

  std::vector<BasicBlock *> Blocks;
  std::vector<BasicBlock *> ExitBlocks;

  void collectBlocks(Function &F);
  void computePostDominators();
  void computeImmediatePostDominators();

public:
  OurPostDominatorTree(Function &F);

  bool postDominates(BasicBlock *A, BasicBlock *B);

  BasicBlock *getIPostDom(BasicBlock *B);

  std::vector<BasicBlock *> controlDependencesOf(BasicBlock *BB);

  BasicBlock *nearestLivePostDominator(
      BasicBlock *BB, std::unordered_set<BasicBlock *> &LiveBlocks);
};

#endif // LLVM_PROJECT_OURPOSTDOMINATORTREE_H
