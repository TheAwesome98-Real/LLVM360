#pragma once
#include "IRGenerator.h"
#include "IRFunc.h"


//
// Helpers
//

#define BUILD  func->m_irGen->m_builder

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
        BUILD->CreateSub(func->getRegister("CTR"), llvm::ConstantInt::get(llvm::Type::getInt32Ty(BUILD->getContext()), 1));
        llvm::Value* isCTRnz = BUILD->CreateICmpNE(func->getRegister("CTR"), 0, "ctrnz");
        should_branch = BUILD->CreateAnd(isCTRnz, BUILD->CreateNot(bi), "shBr");
    }
    // 0001y
    if (isBoBit(instr.ops[0], 1) && !isBoBit(instr.ops[0], 2) &&
        !isBoBit(instr.ops[0], 3) && !isBoBit(instr.ops[0], 4))
    {
        BUILD->CreateSub(func->getRegister("CTR"), llvm::ConstantInt::get(llvm::Type::getInt32Ty(BUILD->getContext()), 1));
        llvm::Value* isCTRz = BUILD->CreateICmpEQ(func->getRegister("CTR"), 0, "ctrnz");
        should_branch = BUILD->CreateAnd(isCTRz, BUILD->CreateNot(bi), "shBr");
    }
    // 001zy
    if (isBoBit(instr.ops[0], 2) &&
        !isBoBit(instr.ops[0], 3) && !isBoBit(instr.ops[0], 4))
    {
        should_branch = BUILD->CreateAnd(BUILD->CreateNot(bi, "not"), llvm::ConstantInt::get(llvm::Type::getInt1Ty(BUILD->getContext()), 1), "shBr");
    }
    // 0100y
    if (!isBoBit(instr.ops[0], 1) && !isBoBit(instr.ops[0], 2) &&
        isBoBit(instr.ops[0], 3) && !isBoBit(instr.ops[0], 4))
    {
        BUILD->CreateSub(func->getRegister("CTR"), llvm::ConstantInt::get(llvm::Type::getInt32Ty(BUILD->getContext()), 1));
        llvm::Value* isCTRnz = BUILD->CreateICmpNE(func->getRegister("CTR"), 0, "ctrnz");
        should_branch = BUILD->CreateAnd(isCTRnz, bi, "shBr");
    }
    // 0101y
    if (isBoBit(instr.ops[0], 1) && !isBoBit(instr.ops[0], 2) &&
        isBoBit(instr.ops[0], 3) && !isBoBit(instr.ops[0], 4))
    {
        BUILD->CreateSub(func->getRegister("CTR"), llvm::ConstantInt::get(llvm::Type::getInt32Ty(BUILD->getContext()), 1));
        llvm::Value* isCTRnz = BUILD->CreateICmpEQ(func->getRegister("CTR"), 0, "ctrnz");
        should_branch = BUILD->CreateAnd(isCTRnz, bi, "shBr");
    }
    // 0b011zy (Branch if condition is TRUE)
    if (isBoBit(instr.ops[0], 2) && isBoBit(instr.ops[0], 3) && !isBoBit(instr.ops[0], 4))
    {
        should_branch = BUILD->CreateAnd(bi, llvm::ConstantInt::get(llvm::Type::getInt1Ty(BUILD->getContext()), 1), "shBr");
    }
    // 0b1z00y
    if (!isBoBit(instr.ops[0], 1) && !isBoBit(instr.ops[0], 2) && isBoBit(instr.ops[0], 4))
    {
        BUILD->CreateSub(func->getRegister("CTR"), llvm::ConstantInt::get(llvm::Type::getInt32Ty(BUILD->getContext()), 1));
        llvm::Value* isCTRnz = BUILD->CreateICmpNE(func->getRegister("CTR"), 0, "ctrnz");
        should_branch = BUILD->CreateAnd(isCTRnz, llvm::ConstantInt::get(llvm::Type::getInt1Ty(BUILD->getContext()), 1), "shBr");
    }

    return should_branch;
}

inline void setCRField(IRFunc* func, uint32_t index, llvm::Value* field)
{
    llvm::LLVMContext& context = func->m_irGen->m_module->getContext();

    int bitOffset = index * 4;
    llvm::Value* shiftAmount = llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), bitOffset);
    llvm::Value* mask = llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), 0b1111);
    llvm::Value* shiftedMask = BUILD->CreateShl(mask, shiftAmount, "shiftedMask");

    // clear the already stored bits
    llvm::Value* invertedMask = BUILD->CreateNot(shiftedMask, "invertedMask");
    llvm::Value* CR_value = BUILD->CreateLoad(
        llvm::Type::getInt32Ty(context),
        func->getRegister("CR"),
        "CR_value"
    );
    llvm::Value* clearedCR = BUILD->CreateAnd(CR_value, invertedMask, "clearedCR");

    // update bits
    llvm::Value* shiftedFieldValue = BUILD->CreateShl(field, shiftAmount, "shiftedFieldValue");
    llvm::Value* updatedCR = BUILD->CreateOr(clearedCR, shiftedFieldValue, "updatedCR");

    BUILD->CreateStore(updatedCR, func->getRegister("CR"));
}

inline llvm::Value* extractCRBit(IRFunc* func, uint32_t BI) {
    llvm::LLVMContext& context = BUILD->getContext();

    // create mask at bit pos
    llvm::Value* mask = llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), 1);
    llvm::Value* shiftAmount = llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), BI);
    llvm::Value* shiftedMask = BUILD->CreateShl(mask, shiftAmount, "shm");

    // mask the bit
    llvm::Value* CR_value = BUILD->CreateLoad(
        llvm::Type::getInt32Ty(context),
        func->getRegister("CR"),
        "CR_value"
    );
    llvm::Value* isolatedBit = BUILD->CreateAnd(CR_value, shiftedMask, "ib");

    llvm::Value* bit = BUILD->CreateLShr(isolatedBit, shiftAmount, "bi");

    return bit;
}

inline llvm::Value* updateCRWithValue(IRFunc* func, llvm::Value* result, llvm::Value* value)
{
    llvm::Value* zero = llvm::ConstantInt::get(llvm::Type::getInt32Ty(func->m_irGen->m_module->getContext()), 0);

    // lt
    llvm::Value* isLT = BUILD->CreateICmpSLT(value, result, "isLT");
    llvm::Value* ltBit = BUILD->CreateZExt(isLT, llvm::Type::getInt32Ty(func->m_irGen->m_module->getContext()), "ltBit");

    // gt
    llvm::Value* isGT = BUILD->CreateICmpSGT(value, result, "isGT");
    llvm::Value* gtBit = BUILD->CreateZExt(isGT, llvm::Type::getInt32Ty(func->m_irGen->m_module->getContext()), "gtBit");

    // eq 
    llvm::Value* isEQ = BUILD->CreateICmpEQ(value, result, "isEQ");
    llvm::Value* eqBit = BUILD->CreateZExt(isEQ, llvm::Type::getInt32Ty(func->m_irGen->m_module->getContext()), "eqBit");

    // so // TODO
    //llvm::Value* soBit = xerSO;

    // build cr0 field 
    llvm::Value* crField = BUILD->CreateOr(
        BUILD->CreateOr(ltBit, BUILD->CreateShl(gtBit, 1)),
        BUILD->CreateOr(BUILD->CreateShl(eqBit, 2),
            BUILD->CreateShl(llvm::ConstantInt::get(llvm::Type::getInt32Ty(func->m_irGen->m_module->getContext()), 0), 3)), // so BIT TODO
        "crField"
    );

    return crField;
}

inline llvm::Value* updateCRWithZero(IRFunc* func, llvm::Value* result)
{
    llvm::Value* zero = llvm::ConstantInt::get(llvm::Type::getInt32Ty(func->m_irGen->m_module->getContext()), 0);

    // lt
    llvm::Value* isLT = BUILD->CreateICmpSLT(result, zero, "isLT");
    llvm::Value* ltBit = BUILD->CreateZExt(isLT, llvm::Type::getInt32Ty(func->m_irGen->m_module->getContext()), "ltBit");

    // gt
    llvm::Value* isGT = BUILD->CreateICmpSGT(result, zero, "isGT");
    llvm::Value* gtBit = BUILD->CreateZExt(isGT, llvm::Type::getInt32Ty(func->m_irGen->m_module->getContext()), "gtBit");

    // eq 
    llvm::Value* isEQ = BUILD->CreateICmpEQ(result, zero, "isEQ");
    llvm::Value* eqBit = BUILD->CreateZExt(isEQ, llvm::Type::getInt32Ty(func->m_irGen->m_module->getContext()), "eqBit");

    // so // TODO
    //llvm::Value* soBit = xerSO;

    // build cr0 field 
    llvm::Value* crField = BUILD->CreateOr(
        BUILD->CreateOr(ltBit, BUILD->CreateShl(gtBit, 1)),
        BUILD->CreateOr(BUILD->CreateShl(eqBit, 2), 
        BUILD->CreateShl(llvm::ConstantInt::get(llvm::Type::getInt32Ty(func->m_irGen->m_module->getContext()), 0), 3)), // so BIT TODO
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
    llvm::Value* extendedDisplacement = BUILD->CreateSExt(llvm::ConstantInt::get(llvm::Type::getInt32Ty(func->m_irGen->m_module->getContext()), llvm::APInt(16, displ, true)),
        llvm::Type::getInt64Ty(func->m_irGen->m_module->getContext()), "ext_D");

    if (gpr == 1) // if gpr used is R1 (stack pointer)
    {
        llvm::Value* regPtr = func->getRegister("RR", gpr); 
        llvm::Value* regValue = BUILD->CreateLoad(
            llvm::Type::getInt64Ty(func->m_irGen->m_module->getContext()), regPtr, "regValue"); 

        llvm::Value* ea = BUILD->CreateAdd(regValue, extendedDisplacement, "ea");
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
    llvm::Value* extendedDisplacement = BUILD->CreateSExt(llvm::ConstantInt::get(llvm::Type::getInt32Ty(func->m_irGen->m_module->getContext()), llvm::APInt(16, displ, true)),
        llvm::Type::getInt64Ty(func->m_irGen->m_module->getContext()), "ext_D");
    llvm::Value* regValue = BUILD->CreateLoad(llvm::Type::getInt64Ty(func->m_irGen->m_module->getContext()),
        func->getRegister("RR", gpr),
        "regValue");
    llvm::Value* ea = BUILD->CreateAdd(regValue, extendedDisplacement, "ea");

    llvm::Value* addressPtr = BUILD->CreateIntToPtr(ea,
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
    auto lrValue = BUILD->CreateLoad(BUILD->getInt64Ty(), func->getSPR(instr.ops[1]), "load_spr");
    BUILD->CreateStore(lrValue, func->getRegister("RR", instr.ops[0]));
}


inline void mtspr_e(Instruction instr, IRFunc* func)
{
    auto rrValue = BUILD->CreateTrunc(BUILD->CreateLoad(BUILD->getInt64Ty(), func->getRegister("RR", instr.ops[1]), "load_rr"),
                                               BUILD->getInt32Ty(), "32b_rr");
    BUILD->CreateStore(rrValue, func->getSPR(instr.ops[0]));
}

inline void bl_e(Instruction instr, IRFunc* func)
{
    uint32_t target = instr.address + signExtend(instr.ops[0], 24);
	IRFunc* targetFunc = func->m_irGen->getCreateFuncInMap(target);
    func->m_irGen->initFuncBody(targetFunc);

    // outdated:
    // 
    // update link register with the llvm return address of the next ir instruction
    // this is an interesting one, since i really don't have a good way to "store the instr.address + 4" in LR and make it work,
    // here is what i do, i create a new basic block for the address of the next instruction (so instr.address + 4 bytes) and store it
    // in LR, so when LR is used to return, it branch to the basic block so the next instruction
    // i think there is a better way to handle this but.. it should work fine for now :}
    // llvm::BlockAddress* lr_BB = func->getCreateBBinMap(instr.address + 4); fix it


    auto argIter = func->m_irFunc->arg_begin();
    llvm::Argument* arg1 = &*argIter;
    llvm::Argument* arg2 = &*(++argIter);

    BUILD->CreateStore(llvm::ConstantInt::get(BUILD->getInt64Ty(), instr.address + 4), func->getRegister("LR"));
	BUILD->CreateCall(targetFunc->m_irFunc, {arg1, arg2});
}

inline void b_e(Instruction instr, IRFunc* func)
{
    uint32_t target = instr.address + signExtend(instr.ops[0], 24);
    llvm::BasicBlock* target_BB = func->getCreateBBinMap(target);

    BUILD->CreateBr(target_BB);
}

inline void bclr_e(Instruction instr, IRFunc* func)
{
	BUILD->CreateRetVoid();
}

inline void stfd_e(Instruction instr, IRFunc* func)
{
    auto frValue = BUILD->CreateLoad(BUILD->getDoubleTy(), func->getRegister("FR", instr.ops[0]), "load_fr");
    BUILD->CreateStore(frValue, getEA(func, instr.ops[1], instr.ops[2])); // address needs to be a pointer (pointer to an address in memory)
}

inline void stw_e(Instruction instr, IRFunc* func)
{
	// truncate the value to 32 bits
    auto rrValue = BUILD->CreateLoad(BUILD->getInt64Ty(), func->getRegister("RR", instr.ops[0]), "load_rr");
    BUILD->CreateStore(BUILD->CreateTrunc(rrValue, llvm::Type::getInt32Ty(func->m_irGen->m_module->getContext()), "low32Bits"), 
                                                            getEA(func, instr.ops[1], instr.ops[2]));
}

// store word with update, update means that the address register is updated with the new address
// so rA = rA + displacement after the store
inline void stwu_e(Instruction instr, IRFunc* func)
{
	llvm::Value* eaVal = getEA(func, instr.ops[1], instr.ops[2]);

    auto rrValue = BUILD->CreateLoad(BUILD->getInt64Ty(), func->getRegister("RR", instr.ops[0]), "load_rr");
    BUILD->CreateStore(BUILD->CreateTrunc(rrValue, llvm::Type::getInt32Ty(func->m_irGen->m_module->getContext()), "low32Bits"),
        eaVal);
    
	// update rA
	BUILD->CreateStore(eaVal, func->getRegister("RR", instr.ops[2]));
}

inline void addi_e(Instruction instr, IRFunc* func)
{
    llvm::Value* im = BUILD->CreateSExt(llvm::ConstantInt::get(llvm::Type::getInt32Ty(func->m_irGen->m_module->getContext()), instr.ops[2]),
        llvm::Type::getInt64Ty(func->m_irGen->m_module->getContext()), "im");
    llvm::Value* rrValue;
    if (instr.ops[1] != 0)
    {
        rrValue = BUILD->CreateLoad(BUILD->getInt64Ty(), func->getRegister("RR", instr.ops[1]), "load_rr");
	}
	else
	{
	    rrValue = llvm::ConstantInt::get(llvm::Type::getInt64Ty(func->m_irGen->m_module->getContext()), 0);
	}
       

    llvm::Value* val = instr.ops[1] ? BUILD->CreateAdd(rrValue, im, "val") : im;
    BUILD->CreateStore(val, func->getRegister("RR", instr.ops[0]));
}


//
// add immediate shifted
// if rA = 0 then it will use value 0 and not the content of rA (because lis use 0)
inline void addis_e(Instruction instr, IRFunc* func)
{
    llvm::Value* shift = BUILD->CreateSExt(llvm::ConstantInt::get(llvm::Type::getInt32Ty(func->m_irGen->m_module->getContext()), instr.ops[2] << 16),
                                                    llvm::Type::getInt64Ty(func->m_irGen->m_module->getContext()), "shift");
    llvm::Value* rrValue;
    if (instr.ops[1] != 0)
    {
        rrValue = BUILD->CreateLoad(BUILD->getInt64Ty(), func->getRegister("RR", instr.ops[1]), "load_rr");
    }
    else
    {
        rrValue = llvm::ConstantInt::get(llvm::Type::getInt64Ty(func->m_irGen->m_module->getContext()), 0);
    }

	llvm::Value* val = instr.ops[1] ? BUILD->CreateAdd(rrValue, shift, "val") : shift;
    BUILD->CreateStore(val, func->getRegister("RR", instr.ops[0]));
}

inline void add_e(Instruction instr, IRFunc* func)
{
    llvm::Value* a = llvm::ConstantInt::get(llvm::Type::getInt64Ty(func->m_irGen->m_module->getContext()), instr.ops[1]);
    llvm::Value* b = llvm::ConstantInt::get(llvm::Type::getInt64Ty(func->m_irGen->m_module->getContext()), instr.ops[2]);
    llvm::Value* val = BUILD->CreateAdd(a, b, "val");
    BUILD->CreateStore(val, func->getRegister("RR", instr.ops[0]));
}

/*inline void orx_e(Instruction instr, IRFunc* func)
{
    llvm::Value* result = BUILD->CreateOr(func->getRegister("RR", instr.ops[1]),
        func->getRegister("RR", instr.ops[2]), "or_result");
    BUILD->CreateStore(result, func->getRegister("RR", instr.ops[0]));

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
    llvm::Value* bi = BUILD->CreateTrunc(extractCRBit(func, instr.ops[1]), BUILD->getInt1Ty(), "tr");
    llvm::Value* should_branch = getBOOperation(func, instr, bi);


    // compute condition BBs
    llvm::BasicBlock* b_true = func->getCreateBBinMap(instr.address + (instr.ops[2] << 2));
    llvm::BasicBlock* b_false = func->getCreateBBinMap(instr.address + 4);

    BUILD->CreateCondBr(should_branch, b_true, b_false);
}

inline void cmpli_e(Instruction instr, IRFunc* func)
{
    
    llvm::Value* a;
    if(instr.ops[1] = 0)
    {
        llvm::Value* low32Bits = BUILD->CreateTrunc(func->getRegister("RR", instr.ops[2]), llvm::Type::getInt32Ty(func->m_irGen->m_module->getContext()), "low32");
        a = BUILD->CreateZExt(low32Bits, BUILD->getInt64Ty(), "ext");
    } 
    else
    {
        a = func->getRegister("RR", instr.ops[2]);
    }

    llvm::Value* imm = BUILD->CreateZExt(llvm::ConstantInt::get(llvm::Type::getInt32Ty(func->m_irGen->m_module->getContext()), 
        instr.ops[3]), BUILD->getInt64Ty(), "extendedA");
    
    setCRField(func, instr.ops[0], updateCRWithValue(func, imm, a));
}


inline void cmpw_e(Instruction instr, IRFunc* func)
{
    llvm::Type* i32Type = llvm::Type::getInt32Ty(func->m_irGen->m_module->getContext());
    llvm::Type* i64Type = llvm::Type::getInt64Ty(func->m_irGen->m_module->getContext());
    
    llvm::Value* a;
    llvm::Value* b;

    if(instr.ops[1] == 1)
    {
        a = llvm::ConstantInt::get(i64Type, instr.ops[2]);
		b = llvm::ConstantInt::get(i64Type, instr.ops[3]);
    }
    else
    {
        a = BUILD->CreateSExt(BUILD->CreateTrunc(llvm::ConstantInt::get(i64Type, instr.ops[2]), i32Type, "tr"), i64Type, "ex");
        b = BUILD->CreateSExt(BUILD->CreateTrunc(llvm::ConstantInt::get(i64Type, instr.ops[3]), i32Type, "tr"), i64Type, "ex");
    }

    llvm::Value* LT = BUILD->CreateICmpSLT(a, b, "lt");
    llvm::Value* GT = BUILD->CreateICmpSGT(a, b, "gt");
    llvm::Value* EQ = BUILD->CreateICmpEQ(a, b, "eq");
    // TODO
    llvm::Value* SO_bit = llvm::ConstantInt::get(llvm::Type::getInt1Ty(func->m_irGen->m_module->getContext()), 0);
    
	llvm::Value* field = BUILD->CreateOr(BUILD->CreateOr(BUILD->CreateOr(LT, BUILD->CreateShl(GT, 1), "sh"), BUILD->CreateShl(EQ, 2), "sh"), BUILD->CreateShl(SO_bit, 3), "sh");
                         

	setCRField(func, instr.ops[0], field);
}


inline void lwz_e(Instruction instr, IRFunc* func)
{
    // EA is the sum(rA | 0) + d.The word in memory addressed by EA is loaded into the low - order 32 bits of rD.The
    // high - order 32 bits of rD are cleared.
    llvm::Value* loadedValue = BUILD->CreateLoad(llvm::Type::getInt32Ty(BUILD->getContext()), getEA(func, instr.ops[1], instr.ops[2]), "loaded_value");
    BUILD->CreateStore(BUILD->CreateZExt(loadedValue, llvm::Type::getInt64Ty(BUILD->getContext()), "zext_lwz"), func->getRegister("RR", instr.ops[0]));
}

inline void lhz_e(Instruction instr, IRFunc* func)
{
    //EA is the sum(rA | 0) + d.The half word in memory addressed by EA is loaded into the low - order 16 bits of rD.
    //The remaining bits in rD are cleared.
	llvm::Value* ea_val = BUILD->CreateLoad(llvm::Type::getInt16Ty(BUILD->getContext()), getEA(func, instr.ops[1], instr.ops[2]), "ea_val");
    llvm::Value* low16 = BUILD->CreateTrunc(ea_val, llvm::Type::getInt16Ty(BUILD->getContext()), "low16");
    BUILD->CreateStore(BUILD->CreateZExt(low16, llvm::Type::getInt64Ty(BUILD->getContext()), "zext_lhz"), func->getRegister("RR", instr.ops[0]));
}

inline void orx_e(Instruction instr, IRFunc* func)
{
    // The contents of rS are ORed with the contents of rB and the result is placed into rA.
	llvm::Value* value = BUILD->CreateOr(BUILD->CreateLoad(BUILD->getInt64Ty(), func->getRegister("RR", instr.ops[1]), "orA"),
                                                  BUILD->CreateLoad(BUILD->getInt64Ty(), func->getRegister("RR", instr.ops[2]), "orB"),
                                                  "or");
	BUILD->CreateStore(value, func->getRegister("RR", instr.ops[0]));
}

inline void sth_e(Instruction instr, IRFunc* func)
{
    //EA is the sum (rA|0) + d. The contents of the low-order 16 bits of rS are stored into the half word in memory
    //addressed by EA.
    llvm::Value* low16 = BUILD->CreateTrunc(BUILD->CreateLoad(llvm::Type::getInt64Ty(BUILD->getContext()), func->getRegister("RR", instr.ops[0]), "load_rr"),
                                                                                llvm::Type::getInt16Ty(BUILD->getContext()), "low16");
    BUILD->CreateStore(low16, getEA(func, instr.ops[1], instr.ops[2]));
}
