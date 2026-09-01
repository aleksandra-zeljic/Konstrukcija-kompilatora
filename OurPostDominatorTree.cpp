#include "OurPostDominatorTree.h"
#include "llvm/IR/CFG.h"

OurPostDominatorTree::OurPostDominatorTree(Function &F)
{
  collectBlocks(F);
  computePostDominators();
  computeImmediatePostDominators();
}

void OurPostDominatorTree::collectBlocks(Function &F)
{
  for (BasicBlock &BB : F) {
    Blocks.push_back(&BB);

   
    if (succ_begin(&BB) == succ_end(&BB)) {
      ExitBlocks.push_back(&BB);
    }
  }
}

void OurPostDominatorTree::computePostDominators()
{
 
  std::unordered_set<BasicBlock *> AllBlocks;
  for (BasicBlock *BB : Blocks) {
    AllBlocks.insert(BB);
  }

  for (BasicBlock *BB : Blocks) {
    if (succ_begin(BB) == succ_end(BB)) {
      PostDom[BB] = {BB};
    } else {
      PostDom[BB] = AllBlocks;
    }
  }

  bool Changed = true;
  while (Changed) {
    Changed = false;

    for (BasicBlock *BB : Blocks) {
      if (succ_begin(BB) == succ_end(BB)) {
        continue;
      }

      std::unordered_set<BasicBlock *> NewSet;
      bool First = true;
      for (BasicBlock *Succ : successors(BB)) {
        if (First) {
          NewSet = PostDom[Succ];
          First = false;
        } else {
          std::unordered_set<BasicBlock *> Tmp;
          for (BasicBlock *X : NewSet) {
            if (PostDom[Succ].count(X)) {
              Tmp.insert(X);
            }
          }
          NewSet = Tmp;
        }
      }

      NewSet.insert(BB);

      if (NewSet != PostDom[BB]) {
        PostDom[BB] = NewSet;
        Changed = true;
      }
    }
  }
}

void OurPostDominatorTree::computeImmediatePostDominators()
{
  for (BasicBlock *BB : Blocks) {
    IPostDom[BB] = nullptr;
    std::vector<BasicBlock *> Strict;
    for (BasicBlock *P : PostDom[BB]) {
      if (P != BB) {
        Strict.push_back(P);
      }
    }
    for (BasicBlock *Cand : Strict) {
      bool IsImmediate = true;
      for (BasicBlock *Other : Strict) {
        if (Other == Cand) {
          continue;
        }
        if (PostDom[Other].count(Cand)) {
          IsImmediate = false;
          break;
        }
      }
      if (IsImmediate) {
        IPostDom[BB] = Cand;
        break;
      }
    }
  }
}

bool OurPostDominatorTree::postDominates(BasicBlock *A, BasicBlock *B)
{
  return PostDom[B].count(A) > 0;
}

BasicBlock *OurPostDominatorTree::getIPostDom(BasicBlock *B)
{
  return IPostDom[B];
}

std::vector<BasicBlock *> OurPostDominatorTree::controlDependencesOf(BasicBlock *BB)
{
  std::vector<BasicBlock *> Result;
  std::unordered_set<BasicBlock *> Seen;

  BasicBlock *IPDomOfBB = IPostDom[BB];

  for (BasicBlock *Pred : predecessors(BB)) {
    BasicBlock *Runner = Pred;

    while (Runner != nullptr && Runner != IPDomOfBB) {
      if (!Seen.count(Runner)) {
        Result.push_back(Runner);
        Seen.insert(Runner);
      }
      Runner = IPostDom[Runner];
    }
  }

  return Result;
}

BasicBlock *OurPostDominatorTree::nearestLivePostDominator(
    BasicBlock *BB, std::unordered_set<BasicBlock *> &LiveBlocks)
{
  BasicBlock *Runner = IPostDom[BB];
  while (Runner != nullptr) {
    if (LiveBlocks.count(Runner)) {
      return Runner;
    }
    Runner = IPostDom[Runner];
  }
  return nullptr;
}
