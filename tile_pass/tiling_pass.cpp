#include <string>
#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/IR/Dominators.h"

#include "llvm/Analysis/ValueTracking.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Utils.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Transforms/Utils/LoopUtils.h"

using namespace llvm;

namespace
{
    struct ForDeletionPass : public LoopPass
    {
        static char ID;
        ForDeletionPass() : LoopPass(ID) {}

        void sub_loop_header(Loop *L)
        {
            errs() << "Loop Header: " << L->getHeader() << "\n";
            errs() << "Loop Depth: " << L->getLoopDepth() << "\n";
            errs() << "Number of Back Edges: " << L->getNumBackEdges() << "\n";
            errs() << "Loop Preheader: " << L->getLoopPreheader() << "\n";

            auto SubLoops = L->getSubLoops();
            size_t NumSubLoops = std::distance(SubLoops.begin(), SubLoops.end());
            errs() << "Number of Sub-Loops: " << NumSubLoops << "\n\n";
            for (Loop *SubLoop : SubLoops)
            {
                sub_loop_header(SubLoop);
            }

            errs() << "\n";
        }

        // This example modifies the program, but preserves CFG.
        void getAnalysisUsage(AnalysisUsage &AU) const override
        {
            getLoopAnalysisUsage(AU);
            // Legacy analysis pass to compute loop infomation.
            AU.addRequired<LoopInfoWrapperPass>();
            // Legacy analysis pass to compute dominator tree.
            AU.addRequired<DominatorTreeWrapperPass>();
        }

        bool runOnLoop(Loop *L, LPPassManager &LPM) override
        {
            errs() << "Default optimizations... \n";

            if (L->getSubLoops().size() > 1)
            {
                errs() << "Find nested loop...\n";
                // Loop analysis pass
                ScalarEvolution *SE = &getAnalysis<ScalarEvolutionWrapperPass>().getSE();
                // Loop basic info
                LoopInfo *LI = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
                // Donimatortree
                auto *DTWP = getAnalysisIfAvailable<DominatorTreeWrapperPass>();
                DominatorTree *DT = DTWP ? &DTWP->getDomTree() : nullptr;

                Loop *InnerLoop = *L->begin();
                // errs() << "Loop flattening running on nested loop: " << L->getHeader()->getName() << "\n";
                // errs() << "Loop flattening running on nested loop: " << L->getName() << "\n";

                // local variables
                BranchInst *InnerBranch, *OuterBranch;
                Value *InnerBound, *OuterBound;
                BinaryOperator *InnerIncrement, *OuterIncrement;
                BasicBlock *body;

                sub_loop_header(L);

                BasicBlock *InnerExitBlock = InnerLoop->getExitBlock();
                BasicBlock *InnerExitingBlock = InnerLoop->getExitingBlock();
                InnerExitingBlock->getTerminator()->eraseFromParent();
                BranchInst::Create(InnerExitBlock, InnerExitingBlock);
                DT->deleteEdge(InnerExitingBlock, InnerLoop->getHeader());

                errs() << "==========================\n";
                errs() << *InnerExitBlock << "\n";
                errs() << "==========================\n";
                errs() << *InnerExitingBlock << "\n";

                // mark the inner loop as a deleted one
                LPM.markLoopAsDeleted(*InnerLoop);
                SE->forgetLoop(L);
                SE->forgetLoop(InnerLoop);
                LI->erase(InnerLoop);
                errs() << *L << "\n";
            }

            // change the code if LICM optimization is performaned
            return true;
        }
    };
}

char ForDeletionPass::ID = 0;

// Automatically enable the pass.
static void registerForDeletionPass(const PassManagerBuilder &, legacy::PassManagerBase &PM) {
    PM.add(new ForDeletionPass());
}
static RegisterStandardPasses RegisterMyPass(PassManagerBuilder::EP_EarlyAsPossible, registerForDeletionPass);