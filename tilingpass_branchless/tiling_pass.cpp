#include "llvm/Analysis/BlockFrequencyInfo.h"
#include "llvm/Analysis/BranchProbabilityInfo.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/LoopIterator.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/IRBuilder.h"
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

#include <iostream>
#include <algorithm>
#include <cmath>
#include <vector>
using namespace llvm;

#define CLS 64  // Cache Line Size
#define CS 32768 // Cache Size
#define DTS 8 // Data type Size (double = 8)

namespace
{
    struct FactorInformation 
    {
        int factorOne;
        int factorTwo;
        double CIM;
        double cost;
        double penalty;

        FactorInformation(int _factorOne, int _factorTwo, double _CIM, double _cost, double _penalty) :
            factorOne(_factorOne), factorTwo(_factorTwo), CIM(_CIM), cost(_cost), penalty(_penalty) { }
    };

    struct TilingPass : public PassInfoMixin<TilingPass>
    {
        PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM)
        {
            // don't run of not in main()
            if (F.getName() != "main")
            {
                F.printAsOperand(errs(), false);
                return PreservedAnalyses::none();
            }

            /* USEFUL ANALYSES */
            LoopAnalysis::Result &LI = FAM.getResult<LoopAnalysis>(F);

            /* USEFUL IR INSERTION POINTS
             *  - MAIN() ENTRY POINT: use mainEntryBuilder
             */
            BasicBlock *mainEntryBB = &F.getEntryBlock();
            IRBuilder<> mainEntryBuilder(mainEntryBB->getContext());
            auto mainTerminator = mainEntryBB->getTerminator();
            if (mainTerminator)
            {
                // Set the insertion point before the terminator instruction
                mainEntryBuilder.SetInsertPoint(mainEntryBB, mainTerminator->getIterator());
            }
            else
            {
                // No terminator, so set the insert point to the end of the block
                mainEntryBuilder.SetInsertPoint(mainEntryBB);
            }

            /* BLOCKING FACTORS:
             *  - B1 (height), B2 (width)
             */

            /* LLVM CONSTANT VALUES */
            auto zeroVal = mainEntryBuilder.getInt32(0);

            // Loop header of original outer loop
            auto iLoop = LI.begin();
            auto iLoopHeader = (*iLoop)->getHeader();
            // assume never nullptr
            auto iReg = cast<LoadInst>(*(iLoopHeader->begin())).getPointerOperand();
            auto matrixSize = (++iLoopHeader->begin())->getOperand(1);
            llvm::ConstantInt *matrixSizeInt = llvm::dyn_cast<llvm::ConstantInt>(matrixSize);

            // errs() << "MATRIS SIZE: " << matrixSizeInt->getSExtValue() << "\n";
            std::vector<int> factors;
            for (int i = 1; i < matrixSizeInt->getSExtValue(); ++i) {
                if (matrixSizeInt->getSExtValue() % i == 0) {
                    factors.push_back(i);
                }
            }

            std::vector<FactorInformation> factorData;
            for (int i = 0; i < factors.size(); ++i) {
                for (int j = 0; j < factors.size(); ++j) {
                    double cost = std::ceil((DTS * (factors[i] * factors[j] + factors[i])) / CLS) * CLS + CLS;
                    if (cost < CS) {
                        factorData.push_back(FactorInformation(
                            factors[i], factors[j], 
                            (2.0 * factors[i] + factors[j]) / (factors[i] * factors[j]), 
                            cost,
                            (std::ceil((double) factors[i] * DTS / CLS) * CLS - (factors[i] * DTS))
                        ));
                    }
                }
            }

            std::sort(factorData.begin(), factorData.end(), [&](FactorInformation &lhs, FactorInformation &rhs) { 
                if (lhs.penalty == rhs.penalty) { return lhs.CIM > rhs.CIM; }
                else { return lhs.penalty > rhs.penalty; }
            });

            for (auto &val : factorData) {
                errs() << "Factor: " << val.factorOne << " " << val.factorTwo << " "
                       << llvm::format("%.5f", val.CIM) << " " << llvm::format("%.0f", val.cost) 
                       << " " << llvm::format("%.0f", val.penalty) << "\n";
            }

            int A = factorData[factorData.size() - 1].factorOne;
            int B = factorData[factorData.size() - 1].factorTwo;
            errs() << "VALUES USED: TJ " << A << " TK" << B << "\n";

            auto B1Val = mainEntryBuilder.getInt32(B);
            auto B2Val = mainEntryBuilder.getInt32(A);

            // Delete first store
            for (User *U : iReg->users())
            {
                if (StoreInst *I = dyn_cast<StoreInst>(U))
                {
                    if (I->getParent() == mainEntryBB)
                    {
                        I->eraseFromParent();
                        break;
                    }
                }
            }

            // Get info for k loop
            auto kLoop = (*iLoop)->getSubLoops().begin();
            auto kLoopHeader = (*kLoop)->getHeader();
            auto &kLoopInit = cast<StoreInst>(*(iLoopHeader->getTerminator()->getSuccessor(0)->begin()));
            auto &kLoopCmp = cast<ICmpInst>(*(++kLoopHeader->begin()));

            // Get info for j loop
            auto jLoop = (*kLoop)->getSubLoops().begin();
            auto jLoopHeader = (*jLoop)->getHeader();
            auto &jLoopInit = cast<StoreInst>(*(++(kLoopHeader->getTerminator()->getSuccessor(0)->rbegin())));
            auto &jLoopCmp = cast<llvm::ICmpInst>(*(++jLoopHeader->begin()));

            // After matrix mult, loops exit here
            auto outerLoopExit = dyn_cast<BranchInst>(iLoopHeader->getTerminator())->getSuccessor(1);

            /* ALL NEW BBs TO ADD*/
            auto kkLoopHeader = BasicBlock::Create(F.getContext(), "kk_loop_header", &F, iLoopHeader);
            auto kkLoopLatch = BasicBlock::Create(F.getContext(), "kk_loop_latch", &F);
            auto kkLoopBB = BasicBlock::Create(F.getContext(), "kk_loop_bb", &F);

            auto jjLoopHeader = BasicBlock::Create(F.getContext(), "jj_loop_header", &F);
            auto jjLoopLatch = BasicBlock::Create(F.getContext(), "jj_loop_latch", &F);
            auto jjLoopBB = BasicBlock::Create(F.getContext(), "jj_loop_bb", &F);

            /* ======= jj LOOP START =======*/
            // Fill jjLoopHeader
            IRBuilder<> jjLoopHeaderBuilder(jjLoopHeader);
            auto currJJ = jjLoopHeaderBuilder.CreatePHI(Type::getInt32Ty(F.getContext()), 2, "curr_jj");
            currJJ->addIncoming(zeroVal, kkLoopBB);
            auto jjCmp = jjLoopHeaderBuilder.CreateICmpSLT(currJJ, matrixSize, "jj_cmp");
            jjLoopHeaderBuilder.CreateCondBr(jjCmp, jjLoopBB, kkLoopLatch);

            // Fill jjLoopLatch
            IRBuilder<> jjLoopLatchBuilder(jjLoopLatch);
            auto newJJLatch = jjLoopLatchBuilder.CreateNSWAdd(currJJ, B2Val, "inc_jj_latch");
            currJJ->addIncoming(newJJLatch, jjLoopLatch);
            jjLoopLatchBuilder.CreateBr(jjLoopHeader);
            iLoopHeader->getTerminator()->setSuccessor(1, jjLoopLatch);

            // Fill jjLoopBB1
            IRBuilder<> jjLoopBBBuilder(jjLoopBB);
            auto newJBound = jjLoopBBBuilder.CreateNSWAdd(currJJ, B2Val, "jj_bound");
            jjLoopBBBuilder.CreateStore(zeroVal, iReg);
            jjLoopBBBuilder.CreateBr(iLoopHeader);

            /* ======= jj LOOP END =======*/

            /* ======= kk LOOP START ======= */
            // Fill kkLoopHeader
            if (BranchInst *I = dyn_cast<BranchInst>(mainEntryBB->getTerminator()))
            {
                I->setSuccessor(0, kkLoopHeader);
            }
            IRBuilder<> kkLoopHeaderBuilder(kkLoopHeader);
            auto currKK = kkLoopHeaderBuilder.CreatePHI(Type::getInt32Ty(F.getContext()), 2, "curr_kk");
            currKK->addIncoming(zeroVal, mainEntryBB);
            auto kkCmp = kkLoopHeaderBuilder.CreateICmpSLT(currKK, matrixSize, "kk_cmp");
            kkLoopHeaderBuilder.CreateCondBr(kkCmp, kkLoopBB, outerLoopExit);

            // Fill kkLoopLatch
            IRBuilder<> kkLoopLatchBuilder(kkLoopLatch);
            auto newKKLatch = kkLoopLatchBuilder.CreateNSWAdd(currKK, B1Val, "inc_kk_latch");
            currKK->addIncoming(newKKLatch, kkLoopLatch);
            kkLoopLatchBuilder.CreateBr(kkLoopHeader);

            // Fill kkLoopBB
            IRBuilder<> kkLoopBBBuilder(kkLoopBB);
            auto newKBound = kkLoopBBBuilder.CreateNSWAdd(currKK, B1Val, "kk_bound");
            kkLoopBBBuilder.CreateBr(jjLoopHeader);

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

extern "C" ::llvm::PassPluginLibraryInfo LLVM_ATTRIBUTE_WEAK llvmGetPassPluginInfo()
{
    return {
        LLVM_PLUGIN_API_VERSION, "tilingpass_branchless", "v0.1",
        [](PassBuilder &PB)
        {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>)
                {
                    if (Name == "tilingpass_branchless")
                    {
                        FPM.addPass(TilingPass());
                        return true;
                    }
                    return false;
                });
        }};
}