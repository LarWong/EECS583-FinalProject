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
            LoopAnalysis::Result &LI= FAM.getResult<LoopAnalysis>(F);

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
            auto outerMatType = dyn_cast<ArrayType>(dyn_cast<AllocaInst>((++mainEntryBB->begin()))->getAllocatedType());
            // auto innerMatType = dyn_cast<ArrayType>(outerMatType->getElementType());
            auto matrixSize = mainEntryBuilder.getInt32(static_cast<int>(outerMatType->getNumElements())); 
            // auto matrixRows = mainEntryBuilder.getInt32(static_cast<int>(outerMatType->getNumElements())); 
            // auto matrixCols = mainEntryBuilder.getInt32(static_cast<int>(innerMatType->getNumElements())); 

            /* NEW LOOP VARIABLES: 
             *  - jj, kk 
             */
            AllocaInst* jjReg = mainEntryBuilder.CreateAlloca(Type::getInt32Ty(F.getContext()), 0, "jj");
            AllocaInst* kkReg = mainEntryBuilder.CreateAlloca(Type::getInt32Ty(F.getContext()), 0, "kk");

            /* 
             * BLOCK BOUNDS VARIABLES: 
             *  - k_block, k_bounds, j_block, j_bounds 
             */

            // Inititalize new variables 
            mainEntryBuilder.CreateStore(zeroVal, jjReg);
            mainEntryBuilder.CreateStore(zeroVal, kkReg);

            // Loop header of original outer loop
            auto iLoop = LI.begin();
            auto iLoopHeader = (*iLoop)->getHeader();
            // assume never nullptr
            auto iReg = cast<LoadInst>(*(iLoopHeader->begin())).getPointerOperand();
            // Delete first store
            for (User *U : iReg->users()) {
                if (StoreInst *I = dyn_cast<StoreInst>(U)) {
                    if (I->getParent() == mainEntryBB) {
                        I->eraseFromParent();
                        break;
                    }
                }
            }

            // Get info for k loop
            auto kLoop = (*iLoop)->getSubLoops().begin();
            auto kLoopHeader = (*kLoop)->getHeader();
            auto& kLoopInit = cast<StoreInst>(*(iLoopHeader->getTerminator()->getSuccessor(0)->begin()));
            auto& kLoopCmp = cast<ICmpInst>(*(++kLoopHeader->begin()));

            // Get info for j loop
            auto jLoop = (*kLoop)->getSubLoops().begin();
            auto jLoopHeader = (*jLoop)->getHeader();
            auto& jLoopInit = cast<StoreInst>(*(++(kLoopHeader->getTerminator()->getSuccessor(0)->rbegin())));
            auto& jLoopCmp = cast<llvm::ICmpInst>(*(++jLoopHeader->begin()));

            // After matrix mult, loops exit here
            auto outerLoopExit = dyn_cast<BranchInst>(iLoopHeader->getTerminator())->getSuccessor(1);

            /* ALL NEW BBs TO ADD*/
            auto kkLoopHeader = BasicBlock::Create(F.getContext(), "kk_loop_header", &F, iLoopHeader);
            auto kkLoopLatch = BasicBlock::Create(F.getContext(), "kk_loop_latch", &F);
            auto kkLoopBB1 = BasicBlock::Create(F.getContext(), "kk_loop_bb_1", &F);
            auto kkLoopTBB = BasicBlock::Create(F.getContext(), "kk_loop_t", &F);
            auto kkLoopFBB = BasicBlock::Create(F.getContext(), "kk_loop_f", &F);
            auto kkLoopBB2 = BasicBlock::Create(F.getContext(), "kk_loop_bb_2", &F);

            auto jjLoopHeader = BasicBlock::Create(F.getContext(), "jj_loop_header", &F);
            auto jjLoopLatch = BasicBlock::Create(F.getContext(), "jj_loop_latch", &F);
            auto jjLoopBB1 = BasicBlock::Create(F.getContext(), "jj_loop_bb_1", &F);
            auto jjLoopTBB = BasicBlock::Create(F.getContext(), "jj_loop_t", &F);
            auto jjLoopFBB = BasicBlock::Create(F.getContext(), "jj_loop_f", &F);
            auto jjLoopBB2 = BasicBlock::Create(F.getContext(), "jj_loop_bb_2", &F);

            /* ======= jj LOOP START =======*/
            // Fill jjLoopHeader
            IRBuilder<> jjLoopHeaderBuilder(jjLoopHeader);
            auto currJJ = jjLoopHeaderBuilder.CreateLoad(Type::getInt32Ty(F.getContext()), jjReg, "curr_jj");
            auto jjCmp = jjLoopHeaderBuilder.CreateICmpSLT(currJJ, matrixSize, "jj_cmp");
            jjLoopHeaderBuilder.CreateCondBr(jjCmp, jjLoopBB1, kkLoopLatch);

            // Fill jjLoopLatch
            IRBuilder<> jjLoopLatchBuilder(jjLoopLatch);
            // auto currJJToInc = jjLoopLatchBuilder.CreateLoad(Type::getInt32Ty(F.getContext()), jjReg, "curr_jj_to_inc");
            auto newJJLatch = jjLoopLatchBuilder.CreateNSWAdd(currJJ, B2Val, "inc_jj_latch");
            jjLoopLatchBuilder.CreateStore(newJJLatch, jjReg);
            jjLoopLatchBuilder.CreateBr(jjLoopHeader);
            iLoopHeader->getTerminator()->setSuccessor(1, jjLoopLatch);

            // Fill jjLoopBB1
            IRBuilder<> jjLoopBB1Builder(jjLoopBB1);
            // auto currJJBB1 = jjLoopBB1Builder.CreateLoad(Type::getInt32Ty(F.getContext()), jjReg, "curr_jj_BB1");
            auto newJJBB1 = jjLoopBB1Builder.CreateNSWAdd(currJJ, B2Val, "new_jj_BB1");
            auto jjTest = jjLoopBB1Builder.CreateICmpSGT(newJJBB1, matrixSize, "test_new_jj");
            jjLoopBB1Builder.CreateCondBr(jjTest, jjLoopTBB, jjLoopFBB);

            // Fill jjLoopT: used for Phi node
            IRBuilder<> jjLoopTBBBuilder(jjLoopTBB);
            jjLoopTBBBuilder.CreateBr(jjLoopBB2); 
            // Fill jjLoopF: used for Phi node
            IRBuilder<> jjLoopFBBBuilder(jjLoopFBB);
            jjLoopFBBBuilder.CreateBr(jjLoopBB2); 

            // Fill jjLoopBB2
            IRBuilder<> jjLoopBB2Builder(jjLoopBB2);
            auto newJBound = jjLoopBB2Builder.CreatePHI(Type::getInt32Ty(F.getContext()), 2, "j_bound_phi");
            newJBound->addIncoming(newJJBB1, jjLoopFBB);
            newJBound->addIncoming(matrixSize, jjLoopTBB);
            jjLoopBB2Builder.CreateStore(zeroVal, iReg);
            jjLoopBB2Builder.CreateBr(iLoopHeader);

            /* ======= jj LOOP END =======*/


            /* ======= kk LOOP START ======= */

            // Fill kkLoopHeader
            if (BranchInst *I = dyn_cast<BranchInst>(mainEntryBB->getTerminator())) {
                I->setSuccessor(0, kkLoopHeader);
            }

            IRBuilder<> kkLoopHeaderBuilder(kkLoopHeader);
            auto currKK = kkLoopHeaderBuilder.CreateLoad(Type::getInt32Ty(F.getContext()), kkReg, "curr_kk");
            auto kkCmp = kkLoopHeaderBuilder.CreateICmpSLT(currKK, matrixSize, "kk_cmp");
            kkLoopHeaderBuilder.CreateCondBr(kkCmp, kkLoopBB1, outerLoopExit);

            // Fill kkLoopLatch
            IRBuilder<> kkLoopLatchBuilder(kkLoopLatch);
            // auto currKKToInc = kkLoopLatchBuilder.CreateLoad(Type::getInt32Ty(F.getContext()), kkReg, "curr_kk_to_inc");
            auto newKKLatch = kkLoopLatchBuilder.CreateNSWAdd(currKK, B1Val, "inc_kk_latch");
            kkLoopLatchBuilder.CreateStore(newKKLatch, kkReg);
            kkLoopLatchBuilder.CreateBr(kkLoopHeader);

            // Fill kkLoopBB1
            IRBuilder<> kkLoopBB1Builder(kkLoopBB1);
            // auto currKKBB1 = kkLoopBB1Builder.CreateLoad(Type::getInt32Ty(F.getContext()), kkReg, "curr_kk_BB1");
            auto newKKBB1 = kkLoopBB1Builder.CreateNSWAdd(currKK, B1Val, "new_kk_BB1");
            auto kkTest = kkLoopBB1Builder.CreateICmpSGT(newKKBB1, matrixSize, "test_new_kk");
            kkLoopBB1Builder.CreateCondBr(kkTest, kkLoopTBB, kkLoopFBB);

            // Fill kkLoopT: used for Phi node
            IRBuilder<> kkLoopTBBBuilder(kkLoopTBB);
            kkLoopTBBBuilder.CreateBr(kkLoopBB2); 
            // Fill kkLoopF: used for Phi node
            IRBuilder<> kkLoopFBBBuilder(kkLoopFBB);
            kkLoopFBBBuilder.CreateBr(kkLoopBB2); 

            // Fill kkLoopBB2
            IRBuilder<> kkLoopBB2Builder(kkLoopBB2);
            auto newKBound = kkLoopBB2Builder.CreatePHI(Type::getInt32Ty(F.getContext()), 2, "k_bound_phi");
            newKBound->addIncoming(newKKBB1, kkLoopFBB);
            newKBound->addIncoming(matrixSize, kkLoopTBB);
            kkLoopBB2Builder.CreateStore(zeroVal, jjReg);
            kkLoopBB2Builder.CreateBr(jjLoopHeader);

            /* ======= kk LOOP END ======= */


            /* UPDATE BOUNDS FOR INNER LOOPS */
            kLoopInit.setOperand(0, currKK);
            kLoopCmp.setOperand(1, newKBound);

            jLoopInit.setOperand(0, currJJ);
            jLoopCmp.setOperand(1, newJBound);

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