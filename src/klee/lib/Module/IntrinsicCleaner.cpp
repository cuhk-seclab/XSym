//===-- IntrinsicCleaner.cpp ----------------------------------------------===//
//
//                     The KLEE Symbolic Virtual Machine
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include <iostream>
#include "Passes.h"

#include "klee/Config/Version.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

using namespace llvm;

namespace klee {

char IntrinsicCleanerPass::ID;

bool IntrinsicCleanerPass::runOnModule(Module &M) {
  bool dirty = false;
  for (Module::iterator f = M.begin(), fe = M.end(); f != fe; ++f)
    for (Function::iterator b = f->begin(), be = f->end(); b != be; ++b)
      dirty |= runOnBasicBlock(*b, M);

  if (Function *Declare = M.getFunction("llvm.trap")) {
    Declare->eraseFromParent();
    dirty = true;
  }
  return dirty;
}

bool IntrinsicCleanerPass::runOnBasicBlock(BasicBlock &b, Module &M) {
  bool dirty = false;
  LLVMContext &ctx = M.getContext();

  unsigned WordSize = DataLayout.getPointerSizeInBits() / 8;
  for (BasicBlock::iterator i = b.begin(), ie = b.end(); i != ie;) {
    IntrinsicInst *ii = dyn_cast<IntrinsicInst>(&*i);
    // increment now since deletion of instructions makes iterator invalid.
    ++i;
    if (ii) {
      if (isa<DbgInfoIntrinsic>(ii))
        continue;

      switch (ii->getIntrinsicID()) {
      case Intrinsic::vastart:
      case Intrinsic::vaend:
      case Intrinsic::fabs:
        break;

        // Lower vacopy so that object resolution etc is handled by
        // normal instructions.
        //
        // FIXME: This is much more target dependent than just the word size,
        // however this works for x86-32 and x86-64.
      case Intrinsic::vacopy: { // (dst, src) -> *((i8**) dst) = *((i8**) src)
        llvm::IRBuilder<> Builder(ii);
        Value *dst = ii->getArgOperand(0);
        Value *src = ii->getArgOperand(1);

        if (WordSize == 4) {
          Type *i8pp = PointerType::getUnqual(
              PointerType::getUnqual(Type::getInt8Ty(ctx)));
          auto castedDst =
              Builder.CreatePointerCast(dst, i8pp, "vacopy.cast.dst");
          auto castedSrc =
              Builder.CreatePointerCast(src, i8pp, "vacopy.cast.src");
          auto load = Builder.CreateLoad(castedSrc, "vacopy.read");
          Builder.CreateStore(load, castedDst, false /* isVolatile */);
        } else {
          assert(WordSize == 8 && "Invalid word size!");
          Type *i64p = PointerType::getUnqual(Type::getInt64Ty(ctx));
          auto pDst = Builder.CreatePointerCast(dst, i64p, "vacopy.cast.dst");
          auto pSrc = Builder.CreatePointerCast(src, i64p, "vacopy.cast.src");
          auto val = Builder.CreateLoad(pSrc, std::string());
          Builder.CreateStore(val, pDst, ii);

          auto off = ConstantInt::get(Type::getInt64Ty(ctx), 1);
          pDst = Builder.CreateGEP(nullptr, pDst, off, std::string());
          pSrc = Builder.CreateGEP(nullptr, pSrc, off, std::string());
          val = Builder.CreateLoad(pSrc, std::string());
          Builder.CreateStore(val, pDst);
          pDst = Builder.CreateGEP(nullptr, pDst, off, std::string());
          pSrc = Builder.CreateGEP(nullptr, pSrc, off, std::string());
          val = Builder.CreateLoad(pSrc, std::string());
          Builder.CreateStore(val, pDst);
        }
        ii->eraseFromParent();
        dirty = true;
        break;
      }

      case Intrinsic::sadd_with_overflow:
      case Intrinsic::ssub_with_overflow:
      case Intrinsic::smul_with_overflow:
      case Intrinsic::uadd_with_overflow:
      case Intrinsic::usub_with_overflow:
      case Intrinsic::umul_with_overflow: {
        IRBuilder<> builder(ii->getParent(), ii->getIterator());

        Value *op1 = ii->getArgOperand(0);
        Value *op2 = ii->getArgOperand(1);

        Value *result = 0;
        Value *result_ext = 0;
        Value *overflow = 0;

        unsigned int bw = op1->getType()->getPrimitiveSizeInBits();
        unsigned int bw2 = op1->getType()->getPrimitiveSizeInBits() * 2;

        if ((ii->getIntrinsicID() == Intrinsic::uadd_with_overflow) ||
            (ii->getIntrinsicID() == Intrinsic::usub_with_overflow) ||
            (ii->getIntrinsicID() == Intrinsic::umul_with_overflow)) {

          Value *op1ext =
              builder.CreateZExt(op1, IntegerType::get(M.getContext(), bw2));
          Value *op2ext =
              builder.CreateZExt(op2, IntegerType::get(M.getContext(), bw2));
          Value *int_max_s =
              ConstantInt::get(op1->getType(), APInt::getMaxValue(bw));
          Value *int_max = builder.CreateZExt(
              int_max_s, IntegerType::get(M.getContext(), bw2));

          if (ii->getIntrinsicID() == Intrinsic::uadd_with_overflow) {
            result_ext = builder.CreateAdd(op1ext, op2ext);
          } else if (ii->getIntrinsicID() == Intrinsic::usub_with_overflow) {
            result_ext = builder.CreateSub(op1ext, op2ext);
          } else if (ii->getIntrinsicID() == Intrinsic::umul_with_overflow) {
            result_ext = builder.CreateMul(op1ext, op2ext);
          }
          overflow = builder.CreateICmpUGT(result_ext, int_max);

        } else if ((ii->getIntrinsicID() == Intrinsic::sadd_with_overflow) ||
                   (ii->getIntrinsicID() == Intrinsic::ssub_with_overflow) ||
                   (ii->getIntrinsicID() == Intrinsic::smul_with_overflow)) {

          Value *op1ext =
              builder.CreateSExt(op1, IntegerType::get(M.getContext(), bw2));
          Value *op2ext =
              builder.CreateSExt(op2, IntegerType::get(M.getContext(), bw2));
          Value *int_max_s =
              ConstantInt::get(op1->getType(), APInt::getSignedMaxValue(bw));
          Value *int_min_s =
              ConstantInt::get(op1->getType(), APInt::getSignedMinValue(bw));
          Value *int_max = builder.CreateSExt(
              int_max_s, IntegerType::get(M.getContext(), bw2));
          Value *int_min = builder.CreateSExt(
              int_min_s, IntegerType::get(M.getContext(), bw2));

          if (ii->getIntrinsicID() == Intrinsic::sadd_with_overflow) {
            result_ext = builder.CreateAdd(op1ext, op2ext);
          } else if (ii->getIntrinsicID() == Intrinsic::ssub_with_overflow) {
            result_ext = builder.CreateSub(op1ext, op2ext);
          } else if (ii->getIntrinsicID() == Intrinsic::smul_with_overflow) {
            result_ext = builder.CreateMul(op1ext, op2ext);
          }
          overflow =
              builder.CreateOr(builder.CreateICmpSGT(result_ext, int_max),
                               builder.CreateICmpSLT(result_ext, int_min));
        }

        // This trunc cound be replaced by a more general trunc replacement
        // that allows to detect also undefined behavior in assignments or
        // overflow in operation with integers whose dimension is smaller than
        // int's dimension, e.g.
        //     uint8_t = uint8_t + uint8_t;
        // if one desires the wrapping should write
        //     uint8_t = (uint8_t + uint8_t) & 0xFF;
        // before this, must check if it has side effects on other operations
        result = builder.CreateTrunc(result_ext, op1->getType());
        Value *resultStruct = builder.CreateInsertValue(
            UndefValue::get(ii->getType()), result, 0);
        resultStruct = builder.CreateInsertValue(resultStruct, overflow, 1);

        ii->replaceAllUsesWith(resultStruct);
        ii->eraseFromParent();
        dirty = true;
        break;
      }
#if LLVM_VERSION_CODE >= LLVM_VERSION(8, 0)
      case Intrinsic::sadd_sat:
      case Intrinsic::ssub_sat:
      case Intrinsic::uadd_sat:
      case Intrinsic::usub_sat: {
        IRBuilder<> builder(ii);

        Value *op1 = ii->getArgOperand(0);
        Value *op2 = ii->getArgOperand(1);

        unsigned int bw = op1->getType()->getPrimitiveSizeInBits();
        assert(bw == op2->getType()->getPrimitiveSizeInBits());

        Value *overflow = nullptr;
        Value *result = nullptr;
        Value *saturated = nullptr;
        switch(ii->getIntrinsicID()) {
          case Intrinsic::usub_sat:
            result = builder.CreateSub(op1, op2);
            overflow = builder.CreateICmpULT(op1, op2); // a < b  =>  a - b < 0
            saturated = ConstantInt::get(ctx, APInt(bw, 0));
            break;
          case Intrinsic::uadd_sat:
            result = builder.CreateAdd(op1, op2);
            overflow = builder.CreateICmpULT(result, op1); // a + b < a
            saturated = ConstantInt::get(ctx, APInt::getMaxValue(bw));
            break;
          case Intrinsic::ssub_sat:
          case Intrinsic::sadd_sat: {
            if (ii->getIntrinsicID() == Intrinsic::ssub_sat) {
              result = builder.CreateSub(op1, op2);
            } else {
              result = builder.CreateAdd(op1, op2);
            }
            ConstantInt *zero = ConstantInt::get(ctx, APInt(bw, 0));
            ConstantInt *smin = ConstantInt::get(ctx, APInt::getSignedMinValue(bw));
            ConstantInt *smax = ConstantInt::get(ctx, APInt::getSignedMaxValue(bw));

            Value *sign1 = builder.CreateICmpSLT(op1, zero);
            Value *sign2 = builder.CreateICmpSLT(op2, zero);
            Value *signR = builder.CreateICmpSLT(result, zero);

            if (ii->getIntrinsicID() == Intrinsic::ssub_sat) {
              saturated = builder.CreateSelect(sign2, smax, smin);
            } else {
              saturated = builder.CreateSelect(sign2, smin, smax);
            }

            // The sign of the result differs from the sign of the first operand
            overflow = builder.CreateXor(sign1, signR);
            if (ii->getIntrinsicID() == Intrinsic::ssub_sat) {
              // AND the signs of the operands differ
              overflow = builder.CreateAnd(overflow, builder.CreateXor(sign1, sign2));
            } else {
              // AND the signs of the operands are the same
              overflow = builder.CreateAnd(overflow, builder.CreateNot(builder.CreateXor(sign1, sign2)));
            }
            break;
          }
          default: ;
        }

        result = builder.CreateSelect(overflow, saturated, result);
        ii->replaceAllUsesWith(result);
        ii->eraseFromParent();
        dirty = true;
        break;
      }
#endif
      case Intrinsic::trap: {
        // Intrinsic instruction "llvm.trap" found. Directly lower it to
        // a call of the abort() function.
        Function *F = cast<Function>(
            M.getOrInsertFunction("abort", Type::getVoidTy(ctx)
		    KLEE_LLVM_GOIF_TERMINATOR));
        F->setDoesNotReturn();
        F->setDoesNotThrow();

        llvm::IRBuilder<> Builder(ii);
        Builder.CreateCall(F);
        Builder.CreateUnreachable();

        i = ii->eraseFromParent();

        // check if the instruction after the one we just replaced is not the
        // end of the basic block and if it is not (i.e. it is a valid
        // instruction), delete it and all remaining because the cleaner just
        // introduced a terminating instruction (unreachable) otherwise llvm will
        // assert in Verifier::visitTerminatorInstr
        while (i != ie) { // i was already incremented above.
          i = i->eraseFromParent();
        }

        dirty = true;
        break;
      }
      case Intrinsic::objectsize: {
        // We don't know the size of an object in general so we replace
        // with 0 or -1 depending on the second argument to the intrinsic.
#if LLVM_VERSION_CODE >= LLVM_VERSION(5, 0)
        assert(ii->getNumArgOperands() == 3 && "wrong number of arguments");
#else
        assert(ii->getNumArgOperands() == 2 && "wrong number of arguments");
#endif

        Value *minArg = ii->getArgOperand(1);
        assert(minArg && "Failed to get second argument");
        ConstantInt *minArgAsInt = dyn_cast<ConstantInt>(minArg);
        assert(minArgAsInt && "Second arg is not a ConstantInt");
        assert(minArgAsInt->getBitWidth() == 1 &&
               "Second argument is not an i1");

#if LLVM_VERSION_CODE >= LLVM_VERSION(5, 0)
        auto nullArg = ii->getArgOperand(2);
        assert(nullArg && "Failed to get second argument");
        auto nullArgAsInt = dyn_cast<ConstantInt>(nullArg);
        assert(nullArgAsInt && "Third arg is not a ConstantInt");
        assert(nullArgAsInt->getBitWidth() == 1 &&
               "Third argument is not an i1");
	/* TODO should we do something with the 3rd argument? */
#endif

        Value *replacement = NULL;
        IntegerType *intType = dyn_cast<IntegerType>(ii->getType());
        assert(intType && "intrinsic does not have integer return type");
        if (minArgAsInt->isZero()) {
          // min=false
          replacement = ConstantInt::get(intType, -1, /*isSigned=*/true);
        } else {
          // min=true
          replacement = ConstantInt::get(intType, 0, /*isSigned=*/false);
        }
        ii->replaceAllUsesWith(replacement);
        ii->eraseFromParent();
        dirty = true;
        break;
      }

      case Intrinsic::x86_sse42_pcmpestrm128: // llvm.x86.sse42.pcmpestrm128 //phli
       {
        std::cout << ii->getIntrinsicID() <<"\n"; // phli
          std::cout << ii->getNumArgOperands() <<"\n";
          Value *op0 = ii->getArgOperand(0);

          unsigned int i = 0;
          std::cout << "first\n";
          for(i = 0; i < ii->getNumArgOperands(); i ++){
             Value *top = ii->getArgOperand(i);
             std::cout << "type" << top->getType() << "\n";
             std::cout << "size" << top->getType()->getPrimitiveSizeInBits() << "\n";

          }
            ii->replaceAllUsesWith(op0);
            ii->eraseFromParent();
          break;
       }
      case Intrinsic::x86_sse2_pmovmskb_128: // llvm.x86.sse2.pmovmskb.128 //phli
       // Create mask from the most significan bit of each 8 bit element in a.
        {
          std::cout << "x_86_sse2_pmovmskb_128 " << ii->getIntrinsicID() << "\n";// phli
          std::cout << "numof args " << ii->getNumArgOperands() <<"\n";
          Value *op = ii->getArgOperand(0); // 
          //ConstantInt *tryvalue = dyn_cast<ConstantInt>(op);
          //std::cout << "type " << typeof(op) << "\n";
          std::cout << "size " << op->getType()->getPrimitiveSizeInBits() << "\n";
          std::cout << "type " << op->getType() << "\n";
          FunctionType * ft = ii->getFunctionType();
          std::cout << "function type " << "\n";
             ii->getFunctionType()->dump(); //<< "\n";
            //std::string TestStr;
            //llvm::raw_string_ostream info(TestStr);
          //ft->getParamType(0)->dump();
          //ft->getParamType(0)->print(info);
          //std::cout <<info.str() << "\n";


           // std::cout << info.str() << "\n";
        //state.dumpStack(info);
          //std::cout << "function arg type " << ft->getParamType(0)->dump() << "\n";
          Value *p  = NULL; 
          //auto *MD = cast<MetadataAsValue>(op)->getMetadata();
          //auto *V = dyn_cast<ValueAsMetadata>(MD);
          //std::cout << "value "  << tryvalue->getBitWidth() << "\n";
          IntegerType *intType = dyn_cast<IntegerType>(ii->getType());
          p = ConstantInt::get(intType, 0, /*isSigned=*/true);
          ii->replaceAllUsesWith(p);
          ii->eraseFromParent();
          // todo //phli
          break;
       } 
      default:
        std::cout << ii->getIntrinsicID() <<"\n"; // phli
        IL->LowerIntrinsicCall(ii);
        std::cout << "default intrinsic function\n";
        dirty = true;
        break;
      }
    }
  }

  return dirty;
}
} // namespace klee
