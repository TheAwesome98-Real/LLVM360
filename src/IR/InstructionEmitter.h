#pragma once
#include "IRGenerator.h"
#include "IRFunc.h"


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

inline llvm::Value* getBOOperation(IRFunc* func, Instruction instr, llvm::Value* bi)
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
        func->m_irGen->m_builder->CreateSub(func->m_irGen->getRegister("CTR"), llvm::ConstantInt::get(llvm::Type::getInt32Ty(func->m_irGen->m_builder->getContext()), 1));
        llvm::Value* isCTRnz = func->m_irGen->m_builder->CreateICmpNE(func->m_irGen->getRegister("CTR"), 0, "ctrnz");
        should_branch = func->m_irGen->m_builder->CreateAnd(isCTRnz, func->m_irGen->m_builder->CreateNot(bi), "shBr");
    }
    // 0001y
    if (isBoBit(instr.ops[0], 1) && !isBoBit(instr.ops[0], 2) &&
        !isBoBit(instr.ops[0], 3) && !isBoBit(instr.ops[0], 4))
    {
        func->m_irGen->m_builder->CreateSub(func->m_irGen->getRegister("CTR"), llvm::ConstantInt::get(llvm::Type::getInt32Ty(func->m_irGen->m_builder->getContext()), 1));
        llvm::Value* isCTRz = func->m_irGen->m_builder->CreateICmpEQ(func->m_irGen->getRegister("CTR"), 0, "ctrnz");
        should_branch = func->m_irGen->m_builder->CreateAnd(isCTRz, func->m_irGen->m_builder->CreateNot(bi), "shBr");
    }
    // 001zy
    if (isBoBit(instr.ops[0], 2) &&
        !isBoBit(instr.ops[0], 3) && !isBoBit(instr.ops[0], 4))
    {
        should_branch = func->m_irGen->m_builder->CreateAnd(func->m_irGen->m_builder->CreateNot(bi), llvm::ConstantInt::get(llvm::Type::getInt1Ty(func->m_irGen->m_builder->getContext()), 1), "shBr");
    }
    // 0100y
    if (!isBoBit(instr.ops[0], 1) && !isBoBit(instr.ops[0], 2) &&
        isBoBit(instr.ops[0], 3) && !isBoBit(instr.ops[0], 4))
    {
        func->m_irGen->m_builder->CreateSub(func->m_irGen->getRegister("CTR"), llvm::ConstantInt::get(llvm::Type::getInt32Ty(func->m_irGen->m_builder->getContext()), 1));
        llvm::Value* isCTRnz = func->m_irGen->m_builder->CreateICmpNE(func->m_irGen->getRegister("CTR"), 0, "ctrnz");
        should_branch = func->m_irGen->m_builder->CreateAnd(isCTRnz, bi, "shBr");
    }
    // 0101y
    if (isBoBit(instr.ops[0], 1) && !isBoBit(instr.ops[0], 2) &&
        isBoBit(instr.ops[0], 3) && !isBoBit(instr.ops[0], 4))
    {
        func->m_irGen->m_builder->CreateSub(func->m_irGen->getRegister("CTR"), llvm::ConstantInt::get(llvm::Type::getInt32Ty(func->m_irGen->m_builder->getContext()), 1));
        llvm::Value* isCTRnz = func->m_irGen->m_builder->CreateICmpEQ(func->m_irGen->getRegister("CTR"), 0, "ctrnz");
        should_branch = func->m_irGen->m_builder->CreateAnd(isCTRnz, bi, "shBr");
    }
    // 0b011zy (Branch if condition is TRUE)
    if (isBoBit(instr.ops[0], 2) && isBoBit(instr.ops[0], 3) && !isBoBit(instr.ops[0], 4))
    {
        should_branch = func->m_irGen->m_builder->CreateAnd(bi, llvm::ConstantInt::get(llvm::Type::getInt1Ty(func->m_irGen->m_builder->getContext()), 1), "shBr");
    }
    // 0b1z00y
    if (!isBoBit(instr.ops[0], 1) && !isBoBit(instr.ops[0], 2) && isBoBit(instr.ops[0], 4))
    {
        func->m_irGen->m_builder->CreateSub(func->m_irGen->getRegister("CTR"), llvm::ConstantInt::get(llvm::Type::getInt32Ty(func->m_irGen->m_builder->getContext()), 1));
        llvm::Value* isCTRnz = func->m_irGen->m_builder->CreateICmpNE(func->m_irGen->getRegister("CTR"), 0, "ctrnz");
        should_branch = func->m_irGen->m_builder->CreateAnd(isCTRnz, llvm::ConstantInt::get(llvm::Type::getInt1Ty(func->m_irGen->m_builder->getContext()), 1), "shBr");
    }

    return should_branch;
}

inline void setCRField(IRFunc* func, uint32_t index, llvm::Value* field)
{
    llvm::LLVMContext& context = func->m_irGen->m_module->getContext();

    int bitOffset = index * 4;
    llvm::Value* shiftAmount = llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), bitOffset);
    llvm::Value* mask = llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), 0b1111);
    llvm::Value* shiftedMask = func->m_irGen->m_builder->CreateShl(mask, shiftAmount, "shiftedMask");

    // clear the already stored bits
    llvm::Value* invertedMask = func->m_irGen->m_builder->CreateNot(shiftedMask, "invertedMask");
    llvm::Value* clearedCR = func->m_irGen->m_builder->CreateAnd(func->m_irGen->getRegister("CR"), invertedMask, "clearedCR");

    // update bits
    llvm::Value* shiftedFieldValue = func->m_irGen->m_builder->CreateShl(field, shiftAmount, "shiftedFieldValue");
    llvm::Value* updatedCR = func->m_irGen->m_builder->CreateOr(clearedCR, shiftedFieldValue, "updatedCR");

    func->m_irGen->m_builder->CreateStore(updatedCR, func->m_irGen->getRegister("CR"));
}

inline llvm::Value* extractCRBit(IRFunc* func, uint32_t BI) {
    llvm::LLVMContext& context = func->m_irGen->m_builder->getContext();

    // create mask at bit pos
    llvm::Value* mask = llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), 1);
    llvm::Value* shiftAmount = llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), BI);
    llvm::Value* shiftedMask = func->m_irGen->m_builder->CreateShl(mask, shiftAmount, "shm");

    // mask the bit
    llvm::Value* isolatedBit = func->m_irGen->m_builder->CreateAnd(func->m_irGen->getRegister("CR"), shiftedMask, "ib");
    llvm::Value* bit = func->m_irGen->m_builder->CreateLShr(isolatedBit, shiftAmount, "bi");

    return bit;
}

inline llvm::Value* updateCRWithValue(IRFunc* func, llvm::Value* result, llvm::Value* value)
{
    llvm::Value* zero = llvm::ConstantInt::get(llvm::Type::getInt32Ty(func->m_irGen->m_module->getContext()), 0);

    // lt
    llvm::Value* isLT = func->m_irGen->m_builder->CreateICmpSLT(value, result, "isLT");
    llvm::Value* ltBit = func->m_irGen->m_builder->CreateZExt(isLT, llvm::Type::getInt32Ty(func->m_irGen->m_module->getContext()), "ltBit");

    // gt
    llvm::Value* isGT = func->m_irGen->m_builder->CreateICmpSGT(value, result, "isGT");
    llvm::Value* gtBit = func->m_irGen->m_builder->CreateZExt(isGT, llvm::Type::getInt32Ty(func->m_irGen->m_module->getContext()), "gtBit");

    // eq 
    llvm::Value* isEQ = func->m_irGen->m_builder->CreateICmpEQ(value, result, "isEQ");
    llvm::Value* eqBit = func->m_irGen->m_builder->CreateZExt(isEQ, llvm::Type::getInt32Ty(func->m_irGen->m_module->getContext()), "eqBit");

    // so // TODO
    //llvm::Value* soBit = xerSO;

    // build cr0 field 
    llvm::Value* crField = func->m_irGen->m_builder->CreateOr(
        func->m_irGen->m_builder->CreateOr(ltBit, func->m_irGen->m_builder->CreateShl(gtBit, 1)),
        func->m_irGen->m_builder->CreateOr(func->m_irGen->m_builder->CreateShl(eqBit, 2),
            func->m_irGen->m_builder->CreateShl(llvm::ConstantInt::get(llvm::Type::getInt32Ty(func->m_irGen->m_module->getContext()), 0), 3)), // so BIT TODO
        "crField"
    );

    return crField;
}

inline llvm::Value* updateCRWithZero(IRFunc* func, llvm::Value* result)
{
    llvm::Value* zero = llvm::ConstantInt::get(llvm::Type::getInt32Ty(func->m_irGen->m_module->getContext()), 0);

    // lt
    llvm::Value* isLT = func->m_irGen->m_builder->CreateICmpSLT(result, zero, "isLT");
    llvm::Value* ltBit = func->m_irGen->m_builder->CreateZExt(isLT, llvm::Type::getInt32Ty(func->m_irGen->m_module->getContext()), "ltBit");

    // gt
    llvm::Value* isGT = func->m_irGen->m_builder->CreateICmpSGT(result, zero, "isGT");
    llvm::Value* gtBit = func->m_irGen->m_builder->CreateZExt(isGT, llvm::Type::getInt32Ty(func->m_irGen->m_module->getContext()), "gtBit");

    // eq 
    llvm::Value* isEQ = func->m_irGen->m_builder->CreateICmpEQ(result, zero, "isEQ");
    llvm::Value* eqBit = func->m_irGen->m_builder->CreateZExt(isEQ, llvm::Type::getInt32Ty(func->m_irGen->m_module->getContext()), "eqBit");

    // so // TODO
    //llvm::Value* soBit = xerSO;

    // build cr0 field 
    llvm::Value* crField = func->m_irGen->m_builder->CreateOr(
        func->m_irGen->m_builder->CreateOr(ltBit, func->m_irGen->m_builder->CreateShl(gtBit, 1)),
        func->m_irGen->m_builder->CreateOr(func->m_irGen->m_builder->CreateShl(eqBit, 2), 
        func->m_irGen->m_builder->CreateShl(llvm::ConstantInt::get(llvm::Type::getInt32Ty(func->m_irGen->m_module->getContext()), 0), 3)), // so BIT TODO
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

inline llvm::Value* dynamicStore(IRFunc* func, uint32_t displ, uint32_t gpr)
{
    llvm::Value* extendedDisplacement = func->m_irGen->m_builder->CreateSExt(llvm::ConstantInt::get(llvm::Type::getInt32Ty(func->m_irGen->m_module->getContext()), llvm::APInt(16, displ, true)),
        llvm::Type::getInt64Ty(func->m_irGen->m_module->getContext()), "ext_D");

    if (gpr == 1) // if gpr used is R1 (stack pointer)
    {
        llvm::Value* regPtr = func->m_irGen->getRegister("RR", gpr); 
        llvm::Value* regValue = func->m_irGen->m_builder->CreateLoad(
            llvm::Type::getInt64Ty(func->m_irGen->m_module->getContext()), regPtr, "regValue"); 

        llvm::Value* ea = func->m_irGen->m_builder->CreateAdd(regValue, extendedDisplacement, "ea");
        return ea;
    }

    return nullptr;
}

inline llvm::Value* dynamicLoad(IRFunc* func)
{
    return nullptr;
}

inline llvm::Value* getEA(IRFunc* func, uint32_t displ, uint32_t gpr)
{
    llvm::Value* extendedDisplacement = func->m_irGen->m_builder->CreateSExt(llvm::ConstantInt::get(llvm::Type::getInt32Ty(func->m_irGen->m_module->getContext()), llvm::APInt(16, displ, true)),
        llvm::Type::getInt64Ty(func->m_irGen->m_module->getContext()), "ext_D");
    llvm::Value* regValue = func->m_irGen->m_builder->CreateLoad(llvm::Type::getInt64Ty(func->m_irGen->m_module->getContext()),
        func->m_irGen->getRegister("RR", gpr),
        "regValue");
    llvm::Value* ea = func->m_irGen->m_builder->CreateAdd(regValue, extendedDisplacement, "ea");

    llvm::Value* addressPtr = func->m_irGen->m_builder->CreateIntToPtr(ea,
        llvm::Type::getDoubleTy(func->m_irGen->m_module->getContext())->getPointerTo(),
        "addressPtr"
    );

	return addressPtr;
}


//
// INSTRUCTIONS Emitters
//

inline void nop_e(Instruction instr, IRFunc* func)
{
	// best instruction ever
    return;
}

inline void twi_e(Instruction instr, IRFunc* func)
{
    // temp stub
    return;
}

inline void bcctr_e(Instruction instr, IRFunc* func)
{
    // temp stub
    return;
}

inline void mfspr_e(Instruction instr, IRFunc* func)
{
    auto lrValue = func->m_irGen->m_builder->CreateLoad(func->m_irGen->m_builder->getInt64Ty(), func->m_irGen->getSPR(instr.ops[1]), "load_spr");
    func->m_irGen->m_builder->CreateStore(lrValue, func->m_irGen->getRegister("RR", instr.ops[0]));
}


inline void mtspr_e(Instruction instr, IRFunc* func)
{
    auto rrValue = func->m_irGen->m_builder->CreateTrunc(func->m_irGen->m_builder->CreateLoad(func->m_irGen->m_builder->getInt64Ty(), func->m_irGen->getRegister("RR", instr.ops[1]), "load_rr"),
                                               func->m_irGen->m_builder->getInt32Ty(), "32b_rr");
    func->m_irGen->m_builder->CreateStore(rrValue, func->m_irGen->getSPR(instr.ops[0]));
}

inline void bl_e(Instruction instr, IRFunc* func)
{
    uint32_t target = instr.address + signExtend(instr.ops[0], 24);
    llvm::BasicBlock* target_BB = func->getCreateBBinMap(target);

    // update link register with the llvm return address of the next ir instruction
    // this is an interesting one, since i really don't have a good way to "store the instr.address + 4" in LR and make it work,
    // here is what i do, i create a new basic block for the address of the next instruction (so instr.address + 4 bytes) and store it
    // in LR, so when LR is used to return, it branch to the basic block so the next instruction
    // i think there is a better way to handle this but.. it should work fine for now :}
    // llvm::BlockAddress* lr_BB = func->getCreateBBinMap(instr.address + 4); fix it

    // get the function containing the basic block
    llvm::Function* F = func->m_irGen->m_builder->GetInsertBlock()->getParent();
    llvm::BasicBlock* returnAddr = func->getCreateBBinMap(instr.address + 4);
 
    // block address
    llvm::Value* blockAddr = llvm::BlockAddress::get(F, returnAddr);
    llvm::Value* blockAddrInt = func->m_irGen->m_builder->CreatePtrToInt(blockAddr, func->m_irGen->m_builder->getInt64Ty());

    func->m_irGen->m_builder->CreateStore(blockAddrInt, func->m_irGen->getRegister("LR"));




    // emit branch in ir
    func->m_irGen->m_builder->CreateBr(target_BB);

    // i actually think this is not needed for BL instruction, 
    // since bl is used if LK is 1 so it assumes that the function is returning 
    // so it will also branch to lr_bb
    func->m_irGen->m_builder->CreateBr(returnAddr);

    // here i set the emit insertPoint to that basic block i created for LR
    func->m_irGen->m_builder->SetInsertPoint(returnAddr);
}

inline void b_e(Instruction instr, IRFunc* func)
{
    uint32_t target = instr.address + signExtend(instr.ops[0], 24);
    llvm::BasicBlock* target_BB = func->getCreateBBinMap(target);

    func->m_irGen->m_builder->CreateBr(target_BB);
}

inline void bclr_e(Instruction instr, IRFunc* func)
{

    //
    // TODO, BRANCH CONDITIONS
    //
    assert(instr.ops[0] == 0x14); // only BO = 0x14 is supported for now (branch always)

    // Load the stored LR value
    llvm::Value* LR_val = func->m_irGen->m_builder->CreateLoad(func->m_irGen->m_builder->getInt64Ty(), func->m_irGen->getRegister("LR"), "LR_val");
   
    // Cast i64 back to an actual block address (i8*)
    llvm::Value* blockPtr = func->m_irGen->m_builder->CreateIntToPtr(LR_val, llvm::Type::getInt8Ty(func->m_irGen->m_module->getContext())->getPointerTo(), "retPtr");


    // Create an indirect branch to the stored block
    llvm::IndirectBrInst* indirectBranch = func->m_irGen->m_builder->CreateIndirectBr(blockPtr);
	indirectBranch->addDestination(func->getCreateBBinMap(0));
}

inline void stfd_e(Instruction instr, IRFunc* func)
{
    auto frValue = func->m_irGen->m_builder->CreateLoad(func->m_irGen->m_builder->getDoubleTy(), func->m_irGen->getRegister("FR", instr.ops[0]), "load_fr");
    func->m_irGen->m_builder->CreateStore(frValue, getEA(func, instr.ops[1], instr.ops[2])); // address needs to be a pointer (pointer to an address in memory)
}

inline void stw_e(Instruction instr, IRFunc* func)
{
	// truncate the value to 32 bits
    auto rrValue = func->m_irGen->m_builder->CreateLoad(func->m_irGen->m_builder->getInt64Ty(), func->m_irGen->getRegister("RR", instr.ops[0]), "load_rr");
    func->m_irGen->m_builder->CreateStore(func->m_irGen->m_builder->CreateTrunc(rrValue, llvm::Type::getInt32Ty(func->m_irGen->m_module->getContext()), "low32Bits"), 
                                                            getEA(func, instr.ops[1], instr.ops[2]));
}

// store word with update, update means that the address register is updated with the new address
// so rA = rA + displacement after the store
inline void stwu_e(Instruction instr, IRFunc* func)
{
	llvm::Value* eaVal = getEA(func, instr.ops[1], instr.ops[2]);

    auto rrValue = func->m_irGen->m_builder->CreateLoad(func->m_irGen->m_builder->getInt64Ty(), func->m_irGen->getRegister("RR", instr.ops[0]), "load_rr");
    func->m_irGen->m_builder->CreateStore(func->m_irGen->m_builder->CreateTrunc(rrValue, llvm::Type::getInt32Ty(func->m_irGen->m_module->getContext()), "low32Bits"),
        eaVal);
    
	// update rA
	func->m_irGen->m_builder->CreateStore(eaVal, func->m_irGen->getRegister("RR", instr.ops[2]));
}

inline void addi_e(Instruction instr, IRFunc* func)
{
    llvm::Value* im = func->m_irGen->m_builder->CreateSExt(llvm::ConstantInt::get(llvm::Type::getInt32Ty(func->m_irGen->m_module->getContext()), instr.ops[2]),
        llvm::Type::getInt64Ty(func->m_irGen->m_module->getContext()), "im");
    llvm::Value* rrValue;
    if (instr.ops[1] != 0)
    {
        rrValue = func->m_irGen->m_builder->CreateLoad(func->m_irGen->m_builder->getInt64Ty(), func->m_irGen->getRegister("RR", instr.ops[1]), "load_rr");
	}
	else
	{
	    rrValue = llvm::ConstantInt::get(llvm::Type::getInt64Ty(func->m_irGen->m_module->getContext()), 0);
	}
       

    llvm::Value* val = instr.ops[1] ? func->m_irGen->m_builder->CreateAdd(rrValue, im, "val") : im;
    func->m_irGen->m_builder->CreateStore(val, func->m_irGen->getRegister("RR", instr.ops[0]));
}


//
// add immediate shifted
// if rA = 0 then it will use value 0 and not the content of rA (because lis use 0)
inline void addis_e(Instruction instr, IRFunc* func)
{
    llvm::Value* shift = func->m_irGen->m_builder->CreateSExt(llvm::ConstantInt::get(llvm::Type::getInt32Ty(func->m_irGen->m_module->getContext()), instr.ops[2] << 16),
                                                    llvm::Type::getInt64Ty(func->m_irGen->m_module->getContext()), "shift");
    llvm::Value* rrValue;
    if (instr.ops[1] != 0)
    {
        rrValue = func->m_irGen->m_builder->CreateLoad(func->m_irGen->m_builder->getInt64Ty(), func->m_irGen->getRegister("RR", instr.ops[1]), "load_rr");
    }
    else
    {
        rrValue = llvm::ConstantInt::get(llvm::Type::getInt64Ty(func->m_irGen->m_module->getContext()), 0);
    }

	llvm::Value* val = instr.ops[1] ? func->m_irGen->m_builder->CreateAdd(rrValue, shift, "val") : shift;
    func->m_irGen->m_builder->CreateStore(val, func->m_irGen->getRegister("RR", instr.ops[0]));
}

/*inline void orx_e(Instruction instr, IRFunc* func)
{
    llvm::Value* result = func->m_irGen->m_builder->CreateOr(func->m_irGen->getRegister("RR", instr.ops[1]),
        func->m_irGen->getRegister("RR", instr.ops[2]), "or_result");
    func->m_irGen->m_builder->CreateStore(result, func->m_irGen->getRegister("RR", instr.ops[0]));

    // update condition register if or. (orRC)
    if (strcmp(instr.opcName.c_str(), "orRC") == 0) 
    {
        // update CR0 in CR
        setCRField(gen, 0, updateCRWithZero(gen, result));
    }
}*/

inline void bcx_e(Instruction instr, IRFunc* func)
{
    // first check how to manage the branch condition
    // if "should_branch" == True then
    llvm::Value* bi = func->m_irGen->m_builder->CreateTrunc(extractCRBit(func, instr.ops[1]), func->m_irGen->m_builder->getInt1Ty());
    llvm::Value* should_branch = getBOOperation(func, instr, bi);


    // compute condition BBs
    llvm::BasicBlock* b_true = func->getCreateBBinMap(instr.address + (instr.ops[2] << 2));
    llvm::BasicBlock* b_false = func->getCreateBBinMap(instr.address + 4);

    func->m_irGen->m_builder->CreateCondBr(should_branch, b_true, b_false);
}

inline void cmpli_e(Instruction instr, IRFunc* func)
{
    
    llvm::Value* a;
    if(instr.ops[1] = 0)
    {
        llvm::Value* low32Bits = func->m_irGen->m_builder->CreateTrunc(func->m_irGen->getRegister("RR", instr.ops[2]), llvm::Type::getInt32Ty(func->m_irGen->m_module->getContext()), "low32");
        a = func->m_irGen->m_builder->CreateZExt(low32Bits, func->m_irGen->m_builder->getInt64Ty(), "ext");
    } 
    else
    {
        a = func->m_irGen->getRegister("RR", instr.ops[2]);
    }

    llvm::Value* imm = func->m_irGen->m_builder->CreateZExt(llvm::ConstantInt::get(llvm::Type::getInt32Ty(func->m_irGen->m_module->getContext()), 
        instr.ops[3]), func->m_irGen->m_builder->getInt64Ty(), "extendedA");
    
    setCRField(func, instr.ops[0], updateCRWithValue(func, imm, a));
}

inline void lwz_e(Instruction instr, IRFunc* func)
{
    // EA is the sum(rA | 0) + d.The word in memory addressed by EA is loaded into the low - order 32 bits of rD.The
    // high - order 32 bits of rD are cleared.
    llvm::Value* loadedValue = func->m_irGen->m_builder->CreateLoad(llvm::Type::getInt32Ty(func->m_irGen->m_builder->getContext()), getEA(func, instr.ops[1], instr.ops[2]), "loaded_value");
    func->m_irGen->m_builder->CreateStore(func->m_irGen->m_builder->CreateZExt(loadedValue, llvm::Type::getInt64Ty(func->m_irGen->m_builder->getContext()), "zext_lwz"), func->m_irGen->getRegister("RR", instr.ops[0]));
}

inline void lhz_e(Instruction instr, IRFunc* func)
{
    //EA is the sum(rA | 0) + d.The half word in memory addressed by EA is loaded into the low - order 16 bits of rD.
    //The remaining bits in rD are cleared.
	llvm::Value* ea_val = func->m_irGen->m_builder->CreateLoad(llvm::Type::getInt16Ty(func->m_irGen->m_builder->getContext()), getEA(func, instr.ops[1], instr.ops[2]), "ea_val");
    llvm::Value* low16 = func->m_irGen->m_builder->CreateTrunc(ea_val, llvm::Type::getInt16Ty(func->m_irGen->m_builder->getContext()), "low16");
    func->m_irGen->m_builder->CreateStore(func->m_irGen->m_builder->CreateZExt(low16, llvm::Type::getInt64Ty(func->m_irGen->m_builder->getContext()), "zext_lhz"), func->m_irGen->getRegister("RR", instr.ops[0]));
}

inline void orx_e(Instruction instr, IRFunc* func)
{
    // The contents of rS are ORed with the contents of rB and the result is placed into rA.
	llvm::Value* value = func->m_irGen->m_builder->CreateOr(func->m_irGen->m_builder->CreateLoad(func->m_irGen->m_builder->getInt64Ty(), func->m_irGen->getRegister("RR", instr.ops[1]), "orA"),
                                                  func->m_irGen->m_builder->CreateLoad(func->m_irGen->m_builder->getInt64Ty(), func->m_irGen->getRegister("RR", instr.ops[2]), "orB"),
                                                  "or");
	func->m_irGen->m_builder->CreateStore(value, func->m_irGen->getRegister("RR", instr.ops[0]));
}

inline void sth_e(Instruction instr, IRFunc* func)
{
    //EA is the sum (rA|0) + d. The contents of the low-order 16 bits of rS are stored into the half word in memory
    //addressed by EA.
    llvm::Value* low16 = func->m_irGen->m_builder->CreateTrunc(func->m_irGen->m_builder->CreateLoad(llvm::Type::getInt64Ty(func->m_irGen->m_builder->getContext()), func->m_irGen->getRegister("RR", instr.ops[0]), "load_rr"),
                                                                                llvm::Type::getInt16Ty(func->m_irGen->m_builder->getContext()), "low16");
    func->m_irGen->m_builder->CreateStore(low16, getEA(func, instr.ops[1], instr.ops[2]));
}
