#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include "OurPostDominatorTree.h"
#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace llvm;


namespace {
struct OurAggressiveDCEPass : public FunctionPass {
 
  std::unordered_map<Instruction *, bool> Live;

  
  std::unordered_set<BasicBlock *> LiveBlocks;

  
  std::vector<Instruction *> Worklist;

  static char ID;
  OurAggressiveDCEPass() : FunctionPass(ID) {}


  bool isAlwaysLive(Instruction &I)
  {
    
    if (isa<ReturnInst>(&I) || isa<UnreachableInst>(&I)) {
      return true;
    }

    if (isa<StoreInst>(&I)) {
      return true;
    }

    if (isa<CallInst>(&I)) {
      return true;
    }

    return false;
  }

  void markLive(Instruction *I)
  {
    if (Live[I]) {
      return; 
    }
    Live[I] = true;
    LiveBlocks.insert(I->getParent());
    Worklist.push_back(I);
  }

  void initialize(Function &F)
  {
    for (BasicBlock &BB : F) {
      for (Instruction &I : BB) {
        Live[&I] = false; 
      }
    }

    for (BasicBlock &BB : F) {
      for (Instruction &I : BB) {
        if (isAlwaysLive(I)) {
          markLive(&I);
        }
      }
    }
  }

  void propagate(OurPostDominatorTree &PDT)
  {
    while (!Worklist.empty()) {
      Instruction *I = Worklist.back();
      Worklist.pop_back();

      for (size_t i = 0; i < I->getNumOperands(); i++) {
        if (Instruction *OpInst = dyn_cast<Instruction>(I->getOperand(i))) {
          markLive(OpInst);
        }
      }

      BasicBlock *BB = I->getParent();
      std::vector<BasicBlock *> ControlBlocks = PDT.controlDependencesOf(BB);
      for (BasicBlock *CtrlBB : ControlBlocks) {
        if (Instruction *Term = CtrlBB->getTerminator()) {
          markLive(Term);
        }
      }
    }
  }

  bool removeDeadInstructions(Function &F, OurPostDominatorTree &PDT)
  {
    bool Changed = false;

    std::vector<Instruction *> DeadBranches;
    for (BasicBlock &BB : F) {
      Instruction *Term = BB.getTerminator();
      if (Term == nullptr) {
        continue;
      }
      bool IsBranch = isa<BranchInst>(Term) || isa<SwitchInst>(Term);
      if (IsBranch && !Live[Term]) {
        DeadBranches.push_back(Term);
      }
    }

    for (Instruction *Term : DeadBranches) {
      BasicBlock *BB = Term->getParent();

      BasicBlock *Target = PDT.nearestLivePostDominator(BB, LiveBlocks);
      if (Target == nullptr) {
        continue; 
      }
      if (!Target->empty() && isa<PHINode>(&Target->front())) {
        continue;
      }

      std::unordered_set<BasicBlock *> Handled;
      for (unsigned i = 0; i < Term->getNumSuccessors(); i++) {
        BasicBlock *Succ = Term->getSuccessor(i);
        if (Succ == Target) {
          continue;
        }
        if (Handled.count(Succ)) {
          continue;
        }
        Handled.insert(Succ);
        Succ->removePredecessor(BB);
      }

      IRBuilder<> Builder(Term);
      Builder.CreateBr(Target);
      Term->eraseFromParent();
      Changed = true;
    }

    std::vector<Instruction *> DeadInsts;
    for (BasicBlock &BB : F) {
      for (Instruction &I : BB) {
        if (I.isTerminator()) {
          continue;
        }
        if (!Live[&I]) {
          DeadInsts.push_back(&I);
        }
      }
    }

  
    for (auto it = DeadInsts.rbegin(); it != DeadInsts.rend(); ++it) {
      Instruction *I = *it;
      I->replaceAllUsesWith(PoisonValue::get(I->getType()));
      I->eraseFromParent();
      Changed = true;
    }

    return Changed;
  }

  bool runOnFunction(Function &F) override
  {
    if (F.isDeclaration()) {
      return false; 
    }

    Live.clear();
    LiveBlocks.clear();
    Worklist.clear();

    OurPostDominatorTree PDT(F);

    initialize(F);                     
    propagate(PDT);                    
    bool Changed = removeDeadInstructions(F, PDT); 

    return Changed;
  }
}; 
} 

char OurAggressiveDCEPass::ID = 0;
static RegisterPass<OurAggressiveDCEPass>
    X("aggressive-dead-code-elimination",
      "Our aggressive dead code elimination",
      false /* Only looks at CFG */,
      false /* Analysis Pass */);

// ============================================================================
//  Registracija za NOVI Pass Manager (New PM)
//
//  Na vezbama se koristi stariji LLVM gde Legacy PM radi preko:
//    opt -load ... -enable-new-pm=0 -aggressive-dead-code-elimination
//
//  Noviji LLVM (npr. 18 iz apt paketa) podrazumevano koristi New PM i trazi
//  sintaksu -passes=... . Zato dodajemo i ovaj omotac. Logika je ista - samo
//  se poziva runOnFunction iz gornje strukture. Pass se onda pokrece sa:
//    opt -load-pass-plugin=./build/libOurAggressiveDCE.so \
//        -passes=aggressive-dead-code-elimination -S ulaz.ll
// ============================================================================

namespace {
struct OurAggressiveDCENewPM : public PassInfoMixin<OurAggressiveDCENewPM> {
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &)
  {
    // Pravimo instancu Legacy strukture i pozivamo njen runOnFunction.
    OurAggressiveDCEPass Impl;
    bool Changed = Impl.runOnFunction(F);
    return Changed ? PreservedAnalyses::none() : PreservedAnalyses::all();
  }

  static bool isRequired() { return true; }
};
} // end of anonymous namespace

llvm::PassPluginLibraryInfo getOurADCEPluginInfo()
{
  return {LLVM_PLUGIN_API_VERSION, "OurAggressiveDCE", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "aggressive-dead-code-elimination") {
                    FPM.addPass(OurAggressiveDCENewPM());
                    return true;
                  }
                  return false;
                });
          }};
}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo()
{
  return getOurADCEPluginInfo();
}
