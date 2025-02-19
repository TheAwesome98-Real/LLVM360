#pragma once
#include "IRGenerator.h"

//
// Helpers
//

inline uint32_t signExtend(uint32_t value, int size)
{
    if (value & (1 << (size - 1))) {
        return value | (~0 << size); 
    }
    return value; 
}

inline bool isBoBit(uint32_t value, uint32_t idx) {
    return (value >> idx) & 0b1;
}

inline llvm::Value* getBOOperation(IRGenerator* gen, Instruction instr, llvm::Value* bi)
{
    llvm::Value* should_branch{};

    /*0000y Decrement the CTR, then branch if the decremented CTR[M–63] is not 0 and the condition is FALSE.
      0001y Decrement the CTR, then branch if the decremented CTR[M–63] = 0 and the condition is FALSE.
      001zy Branch if the condition is FALSE.
      0100y Decrement the CTR, then branch if the decremented CTR[M–63] is not 0 and the condition is TRUE.
      0101y Decrement the CTR, then branch if the decremented CTR[M–63] = 0 and the condition is TRUE.
      011zy Branch if the condition is TRUE.
      1z00y Decrement the CTR, then branch if the decremented CTR[M–63] is not 0.*/


      // NOTE, remember to cast "bools" with int1Ty, cause if i do CreateNot with an 32 bit value it will mess up the cmp result

    if (!isBoBit(instr.ops[0], 1) && !isBoBit(instr.ops[0], 2) &&
        !isBoBit(instr.ops[0], 3) && !isBoBit(instr.ops[0], 4))
    {
        gen->m_builder->CreateSub(gen->getRegister("CTR"), llvm::ConstantInt::get(llvm::Type::getInt32Ty(gen->m_builder->getContext()), 1));
        llvm::Value* isCTRnz = gen->m_builder->CreateICmpNE(gen->getRegister("CTR"), 0, "ctrnz");
        should_branch = gen->m_builder->CreateAnd(isCTRnz, gen->m_builder->CreateNot(bi), "shBr");
    }
    // 0001y
    if (isBoBit(instr.ops[0], 1) && !isBoBit(instr.ops[0], 2) &&
        !isBoBit(instr.ops[0], 3) && !isBoBit(instr.ops[0], 4))
    {
        gen->m_builder->CreateSub(gen->getRegister("CTR"), llvm::ConstantInt::get(llvm::Type::getInt32Ty(gen->m_builder->getContext()), 1));
        llvm::Value* isCTRz = gen->m_builder->CreateICmpEQ(gen->getRegister("CTR"), 0, "ctrnz");
        should_branch = gen->m_builder->CreateAnd(isCTRz, gen->m_builder->CreateNot(bi), "shBr");
    }
    // 001zy
    if (isBoBit(instr.ops[0], 2) &&
        !isBoBit(instr.ops[0], 3) && !isBoBit(instr.ops[0], 4))
    {
        should_branch = gen->m_builder->CreateAnd(gen->m_builder->CreateNot(bi), llvm::ConstantInt::get(llvm::Type::getInt1Ty(gen->m_builder->getContext()), 1), "shBr");
    }
    // 0100y
    if (!isBoBit(instr.ops[0], 1) && !isBoBit(instr.ops[0], 2) &&
        isBoBit(instr.ops[0], 3) && !isBoBit(instr.ops[0], 4))
    {
        gen->m_builder->CreateSub(gen->getRegister("CTR"), llvm::ConstantInt::get(llvm::Type::getInt32Ty(gen->m_builder->getContext()), 1));
        llvm::Value* isCTRnz = gen->m_builder->CreateICmpNE(gen->getRegister("CTR"), 0, "ctrnz");
        should_branch = gen->m_builder->CreateAnd(isCTRnz, bi, "shBr");
    }
    // 0101y
    if (isBoBit(instr.ops[0], 1) && !isBoBit(instr.ops[0], 2) &&
        isBoBit(instr.ops[0], 3) && !isBoBit(instr.ops[0], 4))
    {
        gen->m_builder->CreateSub(gen->getRegister("CTR"), llvm::ConstantInt::get(llvm::Type::getInt32Ty(gen->m_builder->getContext()), 1));
        llvm::Value* isCTRnz = gen->m_builder->CreateICmpEQ(gen->getRegister("CTR"), 0, "ctrnz");
        should_branch = gen->m_builder->CreateAnd(isCTRnz, bi, "shBr");
    }
    // 0b011zy (Branch if condition is TRUE)
    if (isBoBit(instr.ops[0], 2) && isBoBit(instr.ops[0], 3) && !isBoBit(instr.ops[0], 4))
    {
        should_branch = gen->m_builder->CreateAnd(bi, llvm::ConstantInt::get(llvm::Type::getInt1Ty(gen->m_builder->getContext()), 1), "shBr");
    }
    // 0b1z00y
    if (!isBoBit(instr.ops[0], 1) && !isBoBit(instr.ops[0], 2) && isBoBit(instr.ops[0], 4))
    {
        gen->m_builder->CreateSub(gen->getRegister("CTR"), llvm::ConstantInt::get(llvm::Type::getInt32Ty(gen->m_builder->getContext()), 1));
        llvm::Value* isCTRnz = gen->m_builder->CreateICmpNE(gen->getRegister("CTR"), 0, "ctrnz");
        should_branch = gen->m_builder->CreateAnd(isCTRnz, llvm::ConstantInt::get(llvm::Type::getInt1Ty(gen->m_builder->getContext()), 1), "shBr");
    }

    return should_branch;
}

inline void setCRField(IRGenerator* gen, uint32_t index, llvm::Value* field)
{
    llvm::LLVMContext& context = gen->m_module->getContext();

    int bitOffset = index * 4;
    llvm::Value* shiftAmount = llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), bitOffset);
    llvm::Value* mask = llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), 0b1111);
    llvm::Value* shiftedMask = gen->m_builder->CreateShl(mask, shiftAmount, "shiftedMask");

    // clear the already stored bits
    llvm::Value* invertedMask = gen->m_builder->CreateNot(shiftedMask, "invertedMask");
    llvm::Value* clearedCR = gen->m_builder->CreateAnd(gen->getRegister("CR"), invertedMask, "clearedCR");

    // update bits
    llvm::Value* shiftedFieldValue = gen->m_builder->CreateShl(field, shiftAmount, "shiftedFieldValue");
    llvm::Value* updatedCR = gen->m_builder->CreateOr(clearedCR, shiftedFieldValue, "updatedCR");

    gen->m_builder->CreateStore(updatedCR, gen->getRegister("CR"));
}

inline llvm::Value* extractCRBit(IRGenerator* gen, uint32_t BI) {
    llvm::LLVMContext& context = gen->m_builder->getContext();

    // create mask at bit pos
    llvm::Value* mask = llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), 1);
    llvm::Value* shiftAmount = llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), BI);
    llvm::Value* shiftedMask = gen->m_builder->CreateShl(mask, shiftAmount, "shm");

    // mask the bit
    llvm::Value* isolatedBit = gen->m_builder->CreateAnd(gen->getRegister("CR"), shiftedMask, "ib");
    llvm::Value* bit = gen->m_builder->CreateLShr(isolatedBit, shiftAmount, "bi");

    return bit;
}

inline llvm::Value* updateCRWithValue(IRGenerator* gen, llvm::Value* result, llvm::Value* value)
{
    llvm::Value* zero = llvm::ConstantInt::get(llvm::Type::getInt32Ty(gen->m_module->getContext()), 0);

    // lt
    llvm::Value* isLT = gen->m_builder->CreateICmpSLT(value, result, "isLT");
    llvm::Value* ltBit = gen->m_builder->CreateZExt(isLT, llvm::Type::getInt32Ty(gen->m_module->getContext()), "ltBit");

    // gt
    llvm::Value* isGT = gen->m_builder->CreateICmpSGT(value, result, "isGT");
    llvm::Value* gtBit = gen->m_builder->CreateZExt(isGT, llvm::Type::getInt32Ty(gen->m_module->getContext()), "gtBit");

    // eq 
    llvm::Value* isEQ = gen->m_builder->CreateICmpEQ(value, result, "isEQ");
    llvm::Value* eqBit = gen->m_builder->CreateZExt(isEQ, llvm::Type::getInt32Ty(gen->m_module->getContext()), "eqBit");

    // so // TODO
    //llvm::Value* soBit = xerSO;

    // build cr0 field 
    llvm::Value* crField = gen->m_builder->CreateOr(
        gen->m_builder->CreateOr(ltBit, gen->m_builder->CreateShl(gtBit, 1)),
        gen->m_builder->CreateOr(gen->m_builder->CreateShl(eqBit, 2),
            gen->m_builder->CreateShl(llvm::ConstantInt::get(llvm::Type::getInt32Ty(gen->m_module->getContext()), 0), 3)), // so BIT TODO
        "crField"
    );

    return crField;
}

inline llvm::Value* updateCRWithZero(IRGenerator* gen, llvm::Value* result)
{
    llvm::Value* zero = llvm::ConstantInt::get(llvm::Type::getInt32Ty(gen->m_module->getContext()), 0);

    // lt
    llvm::Value* isLT = gen->m_builder->CreateICmpSLT(result, zero, "isLT");
    llvm::Value* ltBit = gen->m_builder->CreateZExt(isLT, llvm::Type::getInt32Ty(gen->m_module->getContext()), "ltBit");

    // gt
    llvm::Value* isGT = gen->m_builder->CreateICmpSGT(result, zero, "isGT");
    llvm::Value* gtBit = gen->m_builder->CreateZExt(isGT, llvm::Type::getInt32Ty(gen->m_module->getContext()), "gtBit");

    // eq 
    llvm::Value* isEQ = gen->m_builder->CreateICmpEQ(result, zero, "isEQ");
    llvm::Value* eqBit = gen->m_builder->CreateZExt(isEQ, llvm::Type::getInt32Ty(gen->m_module->getContext()), "eqBit");

    // so // TODO
    //llvm::Value* soBit = xerSO;

    // build cr0 field 
    llvm::Value* crField = gen->m_builder->CreateOr(
        gen->m_builder->CreateOr(ltBit, gen->m_builder->CreateShl(gtBit, 1)),
        gen->m_builder->CreateOr(gen->m_builder->CreateShl(eqBit, 2), 
        gen->m_builder->CreateShl(llvm::ConstantInt::get(llvm::Type::getInt32Ty(gen->m_module->getContext()), 0), 3)), // so BIT TODO
        "cr0Field"
    );

    return crField;
}

//
// ---- Dynamic Store and Load ----
// Those are helper functions to emit ir code that checks if the
// resolved address is in bounds of .rdata or .data
// but if the register used is R1 (stack pointer) don't add the checks
// further optimizations can be added, maybe check if R3 is used (usually stores the return value)
//

inline llvm::Value* dynamicStore(IRGenerator* gen, uint32_t displ, uint32_t gpr)
{
    llvm::Value* extendedDisplacement = gen->m_builder->CreateSExt(llvm::ConstantInt::get(llvm::Type::getInt32Ty(gen->m_module->getContext()), llvm::APInt(16, displ, true)),
        llvm::Type::getInt64Ty(gen->m_module->getContext()), "ext_D");

    if (gpr == 1) // if gpr used is R1 (stack pointer)
    {
        llvm::Value* regPtr = gen->getRegister("RR", gpr); 
        llvm::Value* regValue = gen->m_builder->CreateLoad(
            llvm::Type::getInt64Ty(gen->m_module->getContext()), regPtr, "regValue"); 

        llvm::Value* ea = gen->m_builder->CreateAdd(regValue, extendedDisplacement, "ea");
        return ea;
    }

    return nullptr;
}

inline llvm::Value* dynamicLoad(IRGenerator* gen)
{
    return nullptr;
}

inline llvm::Value* getEA(IRGenerator* gen, uint32_t displ, uint32_t gpr)
{
    llvm::Value* extendedDisplacement = gen->m_builder->CreateSExt(llvm::ConstantInt::get(llvm::Type::getInt32Ty(gen->m_module->getContext()), llvm::APInt(16, displ, true)),
        llvm::Type::getInt64Ty(gen->m_module->getContext()), "ext_D");
    llvm::Value* regValue = gen->m_builder->CreateLoad(llvm::Type::getInt64Ty(gen->m_module->getContext()),
        gen->getRegister("RR", gpr),
        "regValue");
    llvm::Value* ea = gen->m_builder->CreateAdd(regValue, extendedDisplacement, "ea");

    llvm::Value* addressPtr = gen->m_builder->CreateIntToPtr(ea,
        llvm::Type::getDoubleTy(gen->m_module->getContext())->getPointerTo(),
        "addressPtr"
    );

	return addressPtr;
}


//
// INSTRUCTIONS Emitters
//

inline void mfspr_e(Instruction instr, IRGenerator* gen)
{
    auto lrValue = gen->m_builder->CreateLoad(gen->m_builder->getInt64Ty(), gen->getSPR(instr.ops[1]), "load_spr");
    gen->m_builder->CreateStore(lrValue, gen->getRegister("RR", instr.ops[0]));
}



inline void bl_e(Instruction instr, IRGenerator* gen)
{
    uint32_t target = instr.address + signExtend(instr.ops[0], 24);
    llvm::BasicBlock* BB = gen->getCreateBBinMap(target);

    // update link register with the llvm return address of the next ir instruction
    // this is an interesting one, since i really don't have a good way to "store the instr.address + 4" in LR and make it work,
    // here is what i do, i create a new basic block for the address of the next instruction (so instr.address + 4 bytes) and store it
    // in LR, so when LR is used to return, it branch to the basic block so the next instruction
    // i think there is a better way to handle this but.. it should work fine for now :}
    llvm::BasicBlock* lr_BB = gen->getCreateBBinMap(instr.address + 4);
    gen->m_builder->CreateStore(lr_BB, gen->getRegister("LR")); // Store in Link Register (LR)

    // emit branch in ir
    gen->m_builder->CreateBr(BB);

    // i actually think this is not needed for BL instruction, 
    // since bl is used if LK is 1 so it assumes that the function is returning 
    // so it will also branch to lr_bb
    gen->m_builder->CreateBr(lr_BB);

    // here i set the emit insertPoint to that basic block i created for LR
    gen->m_builder->SetInsertPoint(lr_BB);
}

inline void stfd_e(Instruction instr, IRGenerator* gen)
{
    auto frValue = gen->m_builder->CreateLoad(gen->m_builder->getDoubleTy(), gen->getRegister("FR", instr.ops[0]), "load_fr");
    gen->m_builder->CreateStore(frValue, getEA(gen, instr.ops[1], instr.ops[2])); // address needs to be a pointer (pointer to an address in memory)
}

inline void stw_e(Instruction instr, IRGenerator* gen)
{
	// truncate the value to 32 bits
    auto rrValue = gen->m_builder->CreateLoad(gen->m_builder->getInt64Ty(), gen->getRegister("RR", instr.ops[0]), "load_rr");
    gen->m_builder->CreateStore(gen->m_builder->CreateTrunc(rrValue, llvm::Type::getInt32Ty(gen->m_module->getContext()), "low32Bits"), 
                                                            getEA(gen, instr.ops[1], instr.ops[2]));
}

// store word with update, update means that the address register is updated with the new address
// so rA = rA + displacement after the store
inline void stwu_e(Instruction instr, IRGenerator* gen)
{
	llvm::Value* eaVal = getEA(gen, instr.ops[1], instr.ops[2]);

    auto rrValue = gen->m_builder->CreateLoad(gen->m_builder->getInt64Ty(), gen->getRegister("RR", instr.ops[0]), "load_rr");
    gen->m_builder->CreateStore(gen->m_builder->CreateTrunc(rrValue, llvm::Type::getInt32Ty(gen->m_module->getContext()), "low32Bits"),
        eaVal);
    
	// update rA
	gen->m_builder->CreateStore(eaVal, gen->getRegister("RR", instr.ops[2]));
}

inline void addi_e(Instruction instr, IRGenerator* gen)
{
    if(strcmp(instr.opcName.c_str(), "li") == 0)
    {
        llvm::Value* im = gen->m_builder->CreateSExt(llvm::ConstantInt::get(llvm::Type::getInt32Ty(gen->m_module->getContext()), instr.ops[1]),
            llvm::Type::getInt64Ty(gen->m_module->getContext()), "im");
        gen->m_builder->CreateStore(im, gen->getRegister("RR", instr.ops[0]));
        return;
    }

    llvm::Value* im = gen->m_builder->CreateSExt(llvm::ConstantInt::get(llvm::Type::getInt32Ty(gen->m_module->getContext()), instr.ops[2]),
        llvm::Type::getInt64Ty(gen->m_module->getContext()), "im");
    // Add immediate
    llvm::Value* val = gen->m_builder->CreateAdd(gen->getRegister("RR", instr.ops[1]), im, "val");
    gen->m_builder->CreateStore(val, gen->getRegister("RR", instr.ops[0]));
}

inline void addis_e(Instruction instr, IRGenerator* gen)
{
    if (strcmp(instr.opcName.c_str(), "lis") == 0)
    {
        uint32_t shifted = instr.ops[1] << 16;
        llvm::Value* im = gen->m_builder->CreateSExt(llvm::ConstantInt::get(llvm::Type::getInt32Ty(gen->m_module->getContext()), shifted),
            llvm::Type::getInt64Ty(gen->m_module->getContext()), "im");
        gen->m_builder->CreateStore(im, gen->getRegister("RR", instr.ops[0]));
        return;
    }

    uint32_t shifted = instr.ops[2] << 16;
    llvm::Value* im = gen->m_builder->CreateSExt(llvm::ConstantInt::get(llvm::Type::getInt32Ty(gen->m_module->getContext()), shifted),
        llvm::Type::getInt64Ty(gen->m_module->getContext()), "im");
    // Add immediate
    llvm::Value* val = gen->m_builder->CreateAdd(gen->getRegister("RR", instr.ops[1]), im, "val");
    gen->m_builder->CreateStore(val, gen->getRegister("RR", instr.ops[0]));
}

inline void orx_e(Instruction instr, IRGenerator* gen)
{
    llvm::Value* result = gen->m_builder->CreateOr(gen->getRegister("RR", instr.ops[1]),
        gen->getRegister("RR", instr.ops[2]), "or_result");
    gen->m_builder->CreateStore(result, gen->getRegister("RR", instr.ops[0]));

    // update condition register if or. (orRC)
    if (strcmp(instr.opcName.c_str(), "orRC") == 0) 
    {
        // update CR0 in CR
        setCRField(gen, 0, updateCRWithZero(gen, result));
    }
}

inline void bcx_e(Instruction instr, IRGenerator* gen)
{
    // first check how to manage the branch condition
    // if "should_branch" == True then
    llvm::Value* bi = gen->m_builder->CreateTrunc(extractCRBit(gen, instr.ops[1]), gen->m_builder->getInt1Ty());
    llvm::Value* should_branch = getBOOperation(gen, instr, bi);


    // compute condition BBs
    llvm::BasicBlock* b_true = gen->getCreateBBinMap(instr.address + (instr.ops[2] << 2));
    llvm::BasicBlock* b_false = gen->getCreateBBinMap(instr.address + 4);

    gen->m_builder->CreateCondBr(should_branch, b_true, b_false);
}

inline void cmpli_e(Instruction instr, IRGenerator* gen)
{
    
    llvm::Value* a;
    if(instr.ops[1] = 0)
    {
        llvm::Value* low32Bits = gen->m_builder->CreateTrunc(gen->getRegister("RR", instr.ops[2]), llvm::Type::getInt32Ty(gen->m_module->getContext()), "low32");
        a = gen->m_builder->CreateZExt(low32Bits, gen->m_builder->getInt64Ty(), "ext");
    } 
    else
    {
        a = gen->getRegister("RR", instr.ops[2]);
    }

    llvm::Value* imm = gen->m_builder->CreateZExt(llvm::ConstantInt::get(llvm::Type::getInt32Ty(gen->m_module->getContext()), 
        instr.ops[3]), gen->m_builder->getInt64Ty(), "extendedA");
    
    setCRField(gen, instr.ops[0], updateCRWithValue(gen, imm, a));
}