//===-- Frequent Path Loop Invariant Code Motion Pass --------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===---------------------------------------------------------------------===//
//
// EECS583 F23 - This pass can be used as a template for your FPLICM homework
//               assignment.
//               The passes get registered as "fplicm-correctness" and
//               "fplicm-performance".
//
//
////===-------------------------------------------------------------------===//
#include "llvm/Analysis/BlockFrequencyInfo.h"
#include "llvm/Analysis/BranchProbabilityInfo.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/LoopIterator.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Scalar/LoopPassManager.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/LoopUtils.h"

#include<iostream>

/* *******Implementation Starts Here******* */
// You can include more Header files here
/* *******Implementation Ends Here******* */

using namespace llvm;

namespace {
    struct TilingPass : public PassInfoMixin<TilingPass> {
        PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM) {
            // don't run of not in main()
            if (F.getName() != "main") {
                return PreservedAnalyses::all();
            }

            /* USEFUL ANALYSES */
            LoopAnalysis::Result &li = FAM.getResult<LoopAnalysis>(F);

            /* USEFUL IR INSERTION POINTS 
             *  - MAIN() ENTRY POINT: use mainEntryBuilder
             */
            BasicBlock* mainEntryBB = &F.getEntryBlock();
            IRBuilder<> mainEntryBuilder(mainEntryBB->getContext());
            auto mainTerminator = mainEntryBB->getTerminator();
            if (mainTerminator) {
                // Set the insertion point before the terminator instruction
                mainEntryBuilder.SetInsertPoint(mainEntryBB, mainTerminator->getIterator());
            } else {
                // No terminator, so set the insert point to the end of the block
                mainEntryBuilder.SetInsertPoint(mainEntryBB);
            }

            /* BLOCKING FACTORS:
             *  - B1 (height), B2 (width) 
             */
            // TODO: some algo to find values
            int B1 = 2;
            int B2 = 2;

            /* LLVM CONSTANT VALUES */
            auto zeroVal = mainEntryBuilder.getInt32(0);
            auto B1Val = mainEntryBuilder.getInt32(B1);
            auto B2Val = mainEntryBuilder.getInt32(B2);

            /* NEW LOOP VARIABLES: 
             *  - jj, kk 
             */
            AllocaInst* jjReg = mainEntryBuilder.CreateAlloca(Type::getInt32Ty(F.getContext()), 0, "jj");
            AllocaInst* kkReg = mainEntryBuilder.CreateAlloca(Type::getInt32Ty(F.getContext()), 0, "kk");

            /* 
             * BLOCK BOUNDS VARIABLES: 
             *  - k_block, k_bounds, j_block, j_bounds 
             */
            AllocaInst* kBlockReg = mainEntryBuilder.CreateAlloca(Type::getInt32Ty(F.getContext()), 0, "k_block");
            AllocaInst* kBoundsReg = mainEntryBuilder.CreateAlloca(Type::getInt32Ty(F.getContext()), 0, "k_bounds");
            AllocaInst* jBlockReg = mainEntryBuilder.CreateAlloca(Type::getInt32Ty(F.getContext()), 0, "j_block");
            AllocaInst* jBoundsReg = mainEntryBuilder.CreateAlloca(Type::getInt32Ty(F.getContext()), 0, "j_bounds");

            // Inititalize new variables 
            mainEntryBuilder.CreateStore(zeroVal, jjReg);
            mainEntryBuilder.CreateStore(zeroVal, kkReg);
            mainEntryBuilder.CreateStore(zeroVal, kBlockReg);
            mainEntryBuilder.CreateStore(zeroVal, kBoundsReg);
            mainEntryBuilder.CreateStore(zeroVal, jBlockReg);
            mainEntryBuilder.CreateStore(zeroVal, jBoundsReg);

            return PreservedAnalyses::none();
        }
    };
}

extern "C" ::llvm::PassPluginLibraryInfo LLVM_ATTRIBUTE_WEAK llvmGetPassPluginInfo() {
    return {
        LLVM_PLUGIN_API_VERSION, "tilingpass", "v0.1",
        [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>)
                {
                    if (Name == "tilingpass")
                    {
                        FPM.addPass(TilingPass());
                        return true;
                    }
                    return false;
                });
        }};
}