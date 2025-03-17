#pragma once
#include "IRGenerator.h"
#include "IRFunc.h"


//
// Helpers
//

#define BUILD  func->m_irGen->m_builder
#define i64Const(x) BUILD->getInt64(x)
#define i32Const(x) BUILD->getInt32(x)
#define i16Const(x) BUILD->getInt16(x)
#define i1Const(x) BUILD->getInt1(x)

#define i64_T BUILD->getInt64Ty()
#define i32_T BUILD->getInt32Ty()
#define i16_T BUILD->getInt16Ty()
#define i8_T BUILD->getInt8Ty()
#define i1_T BUILD->getInt1Ty()

#define trcTo1(x) BUILD->CreateTrunc(x, BUILD->getInt1Ty(), "trc1")
#define trcTo8(x) BUILD->CreateTrunc(x, BUILD->getInt8Ty(), "trc8")
#define trcTo16(x) BUILD->CreateTrunc(x, BUILD->getInt16Ty(), "trc16")
#define trcTo32(x) BUILD->CreateTrunc(x, BUILD->getInt32Ty(), "trc32")
#define trcTo64(x) BUILD->CreateTrunc(x, BUILD->getInt64Ty(), "trc64")

// register value access / store
#define gprVal(x) BUILD->CreateLoad(BUILD->getInt64Ty(), func->getRegister("RR", x), "rrV")
#define crVal() BUILD->CreateLoad(BUILD->getInt32Ty(), func->getRegister("CR"), "crV")
#define xerVal() BUILD->CreateLoad(BUILD->getInt32Ty(), func->getRegister("XER"), "xerV")
#define ctrVal() BUILD->CreateLoad(BUILD->getInt32Ty(), func->getRegister("CTR"), "ctrV")


// ext
#define sExt64(x) BUILD->CreateSExt(x, BUILD->getInt64Ty(), "sEx64")
#define sExt32(x) BUILD->CreateSExt(x, BUILD->getInt32Ty(), "sEx32")

#define zExt64(x) BUILD->CreateZExt(x, BUILD->getInt64Ty(), "zEx64")
#define zExt32(x) BUILD->CreateZExt(x, BUILD->getInt32Ty(), "zEx32")

#define sign64(x)  llvm::ConstantInt::getSigned(i64_T, x)
#define sign32(x)  llvm::ConstantInt::getSigned(i32_T, x)
#define sign16(x)  llvm::ConstantInt::getSigned(i16_T, x)




inline void StoreCA(IRFunc* func, llvm::Value* ca)
{
    BUILD->CreateStore(BUILD->CreateOr(xerVal(), BUILD->CreateShl(zExt32(ca), 2, "shl"), "or"), func->getRegister("XER"));
}

static inline uint64_t XEMASK(uint32_t mstart, uint32_t mstop) {
    // if mstart ≤ mstop then
    //   mask[mstart:mstop] = ones
    //   mask[all other bits] = zeros
    // else
    //   mask[mstart:63] = ones
    //   mask[0:mstop] = ones
    //   mask[all other bits] = zeros
    mstart &= 0x3F;
    mstop &= 0x3F;
    uint64_t value =
        (UINT64_MAX >> mstart) ^ ((mstop >= 63) ? 0 : UINT64_MAX >> (mstop + 1));
    return mstart <= mstop ? value : ~value;
}

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
        BUILD->CreateSub(func->getRegister("CTR"), i32Const(1));
        llvm::Value* isCTRnz = BUILD->CreateICmpNE(func->getRegister("CTR"), 0, "ctrnz");
        should_branch = BUILD->CreateAnd(isCTRnz, BUILD->CreateNot(bi), "shBr");
    }
    // 0001y
    if (isBoBit(instr.ops[0], 1) && !isBoBit(instr.ops[0], 2) &&
        !isBoBit(instr.ops[0], 3) && !isBoBit(instr.ops[0], 4))
    {
        BUILD->CreateSub(func->getRegister("CTR"), i32Const(1));
        llvm::Value* isCTRz = BUILD->CreateICmpEQ(func->getRegister("CTR"), 0, "ctrnz");
        should_branch = BUILD->CreateAnd(isCTRz, BUILD->CreateNot(bi), "shBr");
    }
    // 001zy
    if (isBoBit(instr.ops[0], 2) &&
        !isBoBit(instr.ops[0], 3) && !isBoBit(instr.ops[0], 4))
    {
		// TODO: optmize this
        should_branch = BUILD->CreateAnd(BUILD->CreateNot(bi, "not"), i1Const(1), "shBr");
        return should_branch;
    }
    // 0100y
    if (!isBoBit(instr.ops[0], 1) && !isBoBit(instr.ops[0], 2) &&
        isBoBit(instr.ops[0], 3) && !isBoBit(instr.ops[0], 4))
    {
        BUILD->CreateSub(func->getRegister("CTR"), i32Const(1));
        llvm::Value* isCTRnz = BUILD->CreateICmpNE(func->getRegister("CTR"), 0, "ctrnz");
        should_branch = BUILD->CreateAnd(isCTRnz, bi, "shBr");
    }
    // 0101y
    if (isBoBit(instr.ops[0], 1) && !isBoBit(instr.ops[0], 2) &&
        isBoBit(instr.ops[0], 3) && !isBoBit(instr.ops[0], 4))
    {
        BUILD->CreateSub(func->getRegister("CTR"), i32Const(1));
        llvm::Value* isCTRnz = BUILD->CreateICmpEQ(func->getRegister("CTR"), 0, "ctrnz");
        should_branch = BUILD->CreateAnd(isCTRnz, bi, "shBr");
    }
    // 0b011zy (Branch if condition is TRUE)
    if (isBoBit(instr.ops[0], 2) && isBoBit(instr.ops[0], 3) && !isBoBit(instr.ops[0], 4))
    {
        should_branch = BUILD->CreateAnd(bi, i1Const(1), "shBr");
        return should_branch;
    }
    // 0b1z00y
    if (!isBoBit(instr.ops[0], 1) && !isBoBit(instr.ops[0], 2) && isBoBit(instr.ops[0], 4))
    {
        BUILD->CreateSub(func->getRegister("CTR"), i32Const(1));
        llvm::Value* isCTRnz = BUILD->CreateICmpNE(func->getRegister("CTR"), 0, "ctrnz");
        should_branch = BUILD->CreateAnd(isCTRnz, i1Const(1), "shBr");
    }

    printf("un-implemented condition\n");
    DebugBreak();
    return nullptr;
}


// TODO refactor
inline void setCRField(IRFunc* func, uint32_t index, llvm::Value* field)
{
    int bitOffset = index * 4;
    llvm::Value* shiftedMask = BUILD->CreateShl(i32Const(0b1111), bitOffset, "shMsk");

    // clear the already stored bits
    llvm::Value* invertedMask = BUILD->CreateNot(shiftedMask, "inMsk");
    llvm::Value* clearedCR = BUILD->CreateAnd(crVal(), invertedMask, "zCR");
    llvm::Value* fieldExtended = zExt32(field); // Cast 'field' to i32 before shifting

    // update bits
    llvm::Value* shiftedFieldValue = BUILD->CreateShl(fieldExtended, bitOffset, "shVal");
    llvm::Value* updatedCR = BUILD->CreateOr(clearedCR, shiftedFieldValue, "uCR");

    BUILD->CreateStore(updatedCR, func->getRegister("CR"));
}

inline llvm::Value* extractCRBit(IRFunc* func, uint32_t BI) {
    // create mask at bit pos
    llvm::Value* mask = i32Const(1);
    llvm::Value* shiftAmount = i32Const(BI);
    llvm::Value* shiftedMask = BUILD->CreateShl(mask, shiftAmount, "shm");

    // mask the bit
    llvm::Value* isolatedBit = BUILD->CreateAnd(crVal(), shiftedMask, "ib");
    return BUILD->CreateIntCast(BUILD->CreateLShr(isolatedBit, shiftAmount, "bi"), BUILD->getInt1Ty(), false,"cast1");
}

inline llvm::Value* updateCRWithValue(IRFunc* func, llvm::Value* result, llvm::Value* value)
{
    // lt gt eq
    llvm::Value* ltBit = zExt32(BUILD->CreateICmpSLT(value, result, "isLT"));
    llvm::Value* gtBit = zExt32(BUILD->CreateICmpSGT(value, result, "isGT"));
    llvm::Value* eqBit = zExt32(BUILD->CreateICmpEQ(value, result, "isEQ"));

    // so // TODO
    //llvm::Value* soBit = xerSO;

    // build cr0 field 
    llvm::Value* crField = BUILD->CreateOr(
        BUILD->CreateOr(ltBit, BUILD->CreateShl(gtBit, 1, "shl"), "or"),
        BUILD->CreateOr(BUILD->CreateShl(eqBit, 2, "shl"),
            BUILD->CreateShl(i32Const(0), 3, "shl"), "or"), // so BIT TODO
        "crField"
    );

    return crField;
}

inline llvm::Value* updateCRWithZero(IRFunc* func, llvm::Value* result)
{
    llvm::Value* zero = i32Const(0);
    // lt gt eq
    llvm::Value* ltBit = zExt32(BUILD->CreateICmpSLT(result, zero, "isLT"));
    llvm::Value* gtBit = zExt32(BUILD->CreateICmpSGT(result, zero, "isGT"));
    llvm::Value* eqBit = zExt32(BUILD->CreateICmpEQ(result, zero, "isEQ"));

    // so // TODO
    //llvm::Value* soBit = xerSO;

    // build cr0 field 
    llvm::Value* crField = BUILD->CreateOr(
        BUILD->CreateOr(ltBit, BUILD->CreateShl(gtBit, 1)),
        BUILD->CreateOr(BUILD->CreateShl(eqBit, 2), 
        BUILD->CreateShl(i32Const(0), 3)), // so BIT TODO
        "cr0Field"
    );

    return crField;
}

inline llvm::Value* getEA_displ(IRFunc* func, uint32_t displ, uint32_t gpr)
{
    llvm::Value* extendedDisplacement = sExt64(llvm::ConstantInt::get(i32_T, llvm::APInt(16, displ, true)));
    llvm::Value* regValue = gprVal(gpr);
    llvm::Value* ea = BUILD->CreateAdd(regValue, extendedDisplacement, "ea");
    return BUILD->CreateIntToPtr(ea, i32_T->getPointerTo(), "addrPtr");
}

inline llvm::Value* getEA_Dword_displ(IRFunc* func, uint32_t displ, uint32_t gpr)
{
    llvm::Value* extendedDisplacement = sExt64(llvm::ConstantInt::get(i32_T, llvm::APInt(16, displ, true)));
    llvm::Value* regValue = gprVal(gpr);
    llvm::Value* ea = BUILD->CreateAdd(regValue, BUILD->CreateShl(extendedDisplacement, 2, "shl"), "ea");
    return BUILD->CreateIntToPtr(ea, i64_T->getPointerTo(), "addrPtr");
}

inline llvm::Value* getEA_regs(IRFunc* func, uint32_t gpr1, uint32_t gpr2)
{
    llvm::Value* ea = trcTo32(BUILD->CreateAdd(gprVal(gpr1), gprVal(gpr2), "ea"));
    return BUILD->CreateIntToPtr(ea, i64_T->getPointerTo(), "addrPtr");
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
    DebugBreak();
    return;
}

inline void bcctr_e(Instruction instr, IRFunc* func)
{
    if (func->has_jumpTable) 
    {
		for (JumpTable* table : func->jumpTables)
		{
            if (instr.address >= table->start_Address && instr.address <= table->end_Address)
            {
                llvm::SwitchInst* Switch = BUILD->CreateSwitch(ctrVal(), func->getCreateBBinMap(table->targets[0]), table->targets.size());
                for (uint32_t target : table->targets)
                {
                    Switch->addCase(i32Const(target), func->getCreateBBinMap(target));
                }
            	return;
            }
		}
    }


    DebugBreak();
    return;
}

inline void mfspr_e(Instruction instr, IRFunc* func)
{
    auto lrValue = BUILD->CreateLoad(BUILD->getInt64Ty(), func->getSPR(instr.ops[1]), "load_spr");
    BUILD->CreateStore(lrValue, func->getRegister("RR", instr.ops[0]));
}


inline void mtspr_e(Instruction instr, IRFunc* func)
{
    auto rrValue = trcTo32(gprVal(instr.ops[1]));
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

    BUILD->CreateStore(i32Const(instr.address + 4), func->getRegister("LR"));
	BUILD->CreateCall(targetFunc->m_irFunc, {arg1, i32Const(instr.address + 4)});
}

inline void b_e(Instruction instr, IRFunc* func)
{
    uint32_t target = instr.address + signExtend(instr.ops[0], 24);
    llvm::BasicBlock* target_BB = func->getCreateBBinMap(target);

    // tail call
    if(func->m_irGen->isIRFuncinMap(target))
    {
        auto argIter = func->m_irFunc->arg_begin();
        llvm::Argument* arg1 = &*argIter;
        llvm::Argument* arg2 = &*(++argIter);

        IRFunc* tailCall = func->m_irGen->getCreateFuncInMap(target);
        BUILD->CreateCall(tailCall->m_irFunc, { arg1, arg2 });
        BUILD->CreateRetVoid();
        return;
    }


    BUILD->CreateBr(target_BB);
}

inline void bclr_e(Instruction instr, IRFunc* func)
{
	BUILD->CreateRetVoid();
}

inline void bcctrl_e(Instruction instr, IRFunc* func)
{
    auto argIter = func->m_irFunc->arg_begin();
    llvm::Argument* arg1 = &*argIter;
    llvm::Argument* arg2 = &*(++argIter);

    BUILD->CreateCall(func->m_irGen->bcctrlFunc, { arg1, i32Const(instr.address + 4) });
}

inline void stfd_e(Instruction instr, IRFunc* func)
{
    auto frValue = BUILD->CreateLoad(BUILD->getDoubleTy(), func->getRegister("FR", instr.ops[0]), "load_fr");
    BUILD->CreateStore(frValue, getEA_displ(func, instr.ops[1], instr.ops[2])); // address needs to be a pointer (pointer to an address in memory)
}

inline void stw_e(Instruction instr, IRFunc* func)
{
	// truncate the value to 32 bits
    auto rrValue32 = trcTo32(gprVal(instr.ops[0]));
    BUILD->CreateStore(rrValue32, getEA_displ(func, instr.ops[1], instr.ops[2]));
}

// store word with update, update means that the address register is updated with the new address
// so rA = rA + displacement after the store
inline void stwu_e(Instruction instr, IRFunc* func)
{
	llvm::Value* eaVal = getEA_displ(func, instr.ops[1], instr.ops[2]);
    auto rrValue32 = trcTo32(gprVal(instr.ops[0]));
    BUILD->CreateStore(rrValue32, eaVal);
	BUILD->CreateStore(eaVal, func->getRegister("RR", instr.ops[2])); // update rA
}


inline void stwx_e(Instruction instr, IRFunc* func)
{
    llvm::Value* ea = getEA_regs(func, instr.ops[1], instr.ops[2]);
    BUILD->CreateStore(trcTo32(gprVal(instr.ops[0])), ea);
}

inline void std_e(Instruction instr, IRFunc* func)
{

    BUILD->CreateStore(gprVal(instr.ops[0]), getEA_Dword_displ(func, instr.ops[1], instr.ops[2]));
}

inline void addi_e(Instruction instr, IRFunc* func)
{
    llvm::Value* im = sExt64(BUILD->getInt16(instr.ops[2]));
    llvm::Value* rrValue;
    if (instr.ops[1] != 0)
    {
		rrValue = gprVal(instr.ops[1]);
	}
	else
	{
		rrValue = i64Const(0);
	}
    llvm::Value* val = instr.ops[1] ? BUILD->CreateAdd(rrValue, im, "val") : im;
    BUILD->CreateStore(val, func->getRegister("RR", instr.ops[0]));
}



// add immediate shifted
// if rA = 0 then it will use value 0 and not the content of rA (because lis use 0)
inline void addis_e(Instruction instr, IRFunc* func)
{
    llvm::Value* shift = sExt64(i32Const(instr.ops[2] << 16));
    llvm::Value* rrValue;
    if (instr.ops[1] != 0)
    {
        rrValue = gprVal(instr.ops[1]);
    }
    else
    {
        rrValue = i64Const(0);
    }
	llvm::Value* val = instr.ops[1] ? BUILD->CreateAdd(rrValue, shift, "val") : shift;
    BUILD->CreateStore(val, func->getRegister("RR", instr.ops[0]));
}

inline void add_e(Instruction instr, IRFunc* func)
{
    llvm::Value* val = BUILD->CreateAdd(gprVal(instr.ops[1]), gprVal(instr.ops[2]), "val");
    BUILD->CreateStore(val, func->getRegister("RR", instr.ops[0]));
}

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
        llvm::Value* low32Bits = trcTo32(gprVal(instr.ops[2]));
        a = zExt64(low32Bits);
    } 
    else
    {
        a = gprVal(instr.ops[2]);
    }

    setCRField(func, instr.ops[0], updateCRWithValue(func, i64Const(instr.ops[3]), a));
}


inline void cmpw_e(Instruction instr, IRFunc* func)
{
    llvm::Value* rA = gprVal(instr.ops[2]);
    llvm::Value* rB = gprVal(instr.ops[3]);
    if(instr.ops[1] != 1)
    {
        rA = sExt64(trcTo32(rA));
        rB = sExt64(trcTo32(rB));
    }
  
    llvm::Value* LT = zExt32(BUILD->CreateICmpSLT(rA, rB, "lt"));
    llvm::Value* GT = zExt32(BUILD->CreateICmpSGT(rA, rB, "gt"));
    llvm::Value* EQ = zExt32(BUILD->CreateICmpEQ(rA, rB, "eq"));
    // TODO
    llvm::Value* SO_bit = i32Const(0);
	llvm::Value* field = zExt32(BUILD->CreateOr(BUILD->CreateOr(BUILD->CreateOr(LT, BUILD->CreateShl(GT, 1, "sh"), "or"), BUILD->CreateShl(EQ, 2, "sh"), "or"), BUILD->CreateShl(SO_bit, 3, "sh"), "or"));

	setCRField(func, instr.ops[0], field);
}

inline void cmpwi_e(Instruction instr, IRFunc* func)
{
    llvm::Value* rA = gprVal(instr.ops[2]);
    llvm::Value* imm = sign16(instr.ops[3]);
    if (instr.ops[1] != 1)
    {
        rA = sExt64(trcTo32(rA));
        imm = sExt64(imm);
    }

    llvm::Value* LT = zExt32(BUILD->CreateICmpSLT(rA, imm, "lt"));
    llvm::Value* GT = zExt32(BUILD->CreateICmpSGT(rA, imm, "gt"));
    llvm::Value* EQ = zExt32(BUILD->CreateICmpEQ(rA, imm, "eq"));

    // TODO
    llvm::Value* SO_bit = i32Const(0);
    llvm::Value* field = zExt32(BUILD->CreateOr(BUILD->CreateOr(BUILD->CreateOr(LT, BUILD->CreateShl(GT, 1, "sh"), "or"), BUILD->CreateShl(EQ, 2, "sh"), "or"), BUILD->CreateShl(SO_bit, 3, "sh"), "or"));

    setCRField(func, instr.ops[0], field);
}


inline void lwz_e(Instruction instr, IRFunc* func)
{
    // EA is the sum(rA | 0) + d.The word in memory addressed by EA is loaded into the low - order 32 bits of rD.The
    // high - order 32 bits of rD are cleared.
    llvm::Value* loadedValue = BUILD->CreateLoad(i32_T, getEA_displ(func, instr.ops[1], instr.ops[2]), "loaded_value");
    BUILD->CreateStore(zExt64(loadedValue), func->getRegister("RR", instr.ops[0]));
}

inline void lwzx_e(Instruction instr, IRFunc* func)
{
    llvm::Value* loadedValue = BUILD->CreateLoad(i32_T, getEA_regs(func, instr.ops[1], instr.ops[2]), "ld");
    BUILD->CreateStore(zExt64(loadedValue), func->getRegister("RR", instr.ops[0]));
}

// could be better
inline void lhz_e(Instruction instr, IRFunc* func)
{
    //EA is the sum(rA | 0) + d.The half word in memory addressed by EA is loaded into the low - order 16 bits of rD.
    //The remaining bits in rD are cleared.
	llvm::Value* ea_val = BUILD->CreateLoad(i16_T, getEA_displ(func, instr.ops[1], instr.ops[2]), "eaV");
    BUILD->CreateStore(zExt64(trcTo16(ea_val)), func->getRegister("RR", instr.ops[0]));
}


inline void lbzx_e(Instruction instr, IRFunc* func)
{
    llvm::Value* loadedValue = BUILD->CreateLoad(i8_T, getEA_regs(func, instr.ops[1], instr.ops[2]), "ld");
    BUILD->CreateStore(zExt64(loadedValue), func->getRegister("RR", instr.ops[0]));
}

inline void sth_e(Instruction instr, IRFunc* func)
{
    //EA is the sum (rA|0) + d. The contents of the low-order 16 bits of rS are stored into the half word in memory
    //addressed by EA.
    llvm::Value* low16 = trcTo16(gprVal(instr.ops[0]));
    BUILD->CreateStore(low16, getEA_displ(func, instr.ops[1], instr.ops[2]));
}

#define DMASK(b, e) (((0xFFFFFFFF << ((31 + (b)) - (e))) >> (b)))
// please optimize this
inline void rlwinm_e(Instruction instr, IRFunc* func)
{
    uint32_t mask = (instr.ops[3] <= instr.ops[4]) ? (DMASK(instr.ops[3], instr.ops[4])) : (DMASK(0, instr.ops[4]) | DMASK(3, 31));

    uint32_t width = 32 - instr.ops[2];
    llvm::Value* lhs = BUILD->CreateShl(gprVal(instr.ops[1]), instr.ops[2], "lhs");
    llvm::Value* rhs = BUILD->CreateLShr(gprVal(instr.ops[1]), width, "rhs");
    llvm::Value* rotl = BUILD->CreateOr(lhs, rhs, "rotl");
    
    auto masked = trcTo32(BUILD->CreateAnd(rotl, i64Const(mask), "and"));
    BUILD->CreateStore(zExt64(masked), func->getRegister("RR", instr.ops[0]));
    
    // RC
    if(strcmp(instr.opcName.c_str(), "rlwinmRC") == 0)
    {
        
        llvm::Value* LT = zExt32(BUILD->CreateICmpSLT(masked, i32Const(0), "lt"));
        llvm::Value* GT = zExt32(BUILD->CreateICmpSGT(masked, i32Const(0), "gt"));
        llvm::Value* EQ = zExt32(BUILD->CreateICmpEQ(masked, i32Const(0), "eq"));
        // TODO
        llvm::Value* SO_bit = i32Const(0);
        llvm::Value* field = zExt32(BUILD->CreateOr(BUILD->CreateOr(BUILD->CreateOr(LT, BUILD->CreateShl(GT, 1, "sh"), "or"), BUILD->CreateShl(EQ, 2, "sh"), "or"), BUILD->CreateShl(SO_bit, 3, "sh"), "or"));

        setCRField(func, 0, field);
    }
}

inline void srawi_e(Instruction instr, IRFunc* func) 
{
    // n <- SH
    // r <- ROTL32((RS)[32:63], 64-n)
    // m <- MASK(n+32, 63)
    // s <- (RS)[32]
    // RA <- r&m | (i64.s)&¬m
    // CA <- s & ((r&¬m)[32:63]≠0)
    // if n == 0: rA <- sign_extend(rS), XER[CA] = 0
    // if n >= 32: rA <- 64 sign bits of rS, XER[CA] = sign bit of lo_32(rS)
   

    llvm::Value* v = trcTo32(gprVal(instr.ops[1]));
    llvm::Value* ca;
    if (!instr.ops[2]) // if shift is 0 don't calculate the other shis
    {
        // No shift, just a fancy sign extend and CA clearer.
        v = sExt64(v);
        ca = BUILD->getInt8(0);
    }
    else 
    {
        // CA is set if any bits are shifted out of the right and if the result
        // is negative.
        uint32_t mask = (uint32_t)XEMASK(64 - instr.ops[2], 63);

        if (mask == 1) 
        {
            ca = BUILD->CreateAnd(BUILD->CreateICmpSLT(v, i32Const(0), "slt"), trcTo1(v), "and");
        }
        else {
            ca = BUILD->CreateAnd(BUILD->CreateICmpSLT(v, i32Const(0), "slt"),
                BUILD->CreateICmpNE(BUILD->CreateAnd(v, i32Const(mask), "and"), i32Const(0), "cmp"), "and");
        }

        //v = f.Sha(v, (int8_t)instr.ops[2]), v = sExt64(v);
        v = BUILD->CreateAShr(v, i32Const(instr.ops[2]), "ashr"), v = sExt64(v);
    }

    StoreCA(func, ca);

    BUILD->CreateStore(v, func->getRegister("RR", instr.ops[0]));
    /*if (i.X.Rc) {
        f.UpdateCR(0, v);
    }*/
}

// AHHHHHHH instrinsic
inline void cntlzw_e(Instruction instr, IRFunc* func)
{
    llvm::Function* CtlzFunc = llvm::Intrinsic::getDeclaration(func->m_irGen->m_module, llvm::Intrinsic::ctlz, { i32_T });
    llvm::Value* IsZeroUndef = i1Const(false);  // Do not allow undef
    llvm::Value* LeadingZeros = BUILD->CreateCall(CtlzFunc, { trcTo32(gprVal(instr.ops[1])), IsZeroUndef}, "call");
    BUILD->CreateStore(LeadingZeros, func->getRegister("RR", instr.ops[0]));
}

//
//// bitwise operators
//

inline void orx_e(Instruction instr, IRFunc* func)
{
    // The contents of rS are ORed with the contents of rB and the result is placed into rA.
    llvm::Value* value = BUILD->CreateOr(gprVal(instr.ops[1]), gprVal(instr.ops[2]), "or");
    BUILD->CreateStore(value, func->getRegister("RR", instr.ops[0]));
}

inline void ori_e(Instruction instr, IRFunc* func)
{
    auto im64 = i64Const(instr.ops[2]);
    auto orResult = BUILD->CreateOr(gprVal(instr.ops[1]), im64, "or");
    BUILD->CreateStore(orResult, func->getRegister("RR", instr.ops[0]));
}

inline void and_e(Instruction instr, IRFunc* func)
{
    auto andResult = BUILD->CreateAnd(gprVal(instr.ops[1]), gprVal(instr.ops[2]), "and");
    BUILD->CreateStore(andResult, func->getRegister("RR", instr.ops[0]));
}

inline void andc_e(Instruction instr, IRFunc* func)
{
    auto andResult = BUILD->CreateAnd(gprVal(instr.ops[1]), BUILD->CreateNot(gprVal(instr.ops[2]), "not"), "and");
    BUILD->CreateStore(andResult, func->getRegister("RR", instr.ops[0]));
}

inline void andiRC_e(Instruction instr, IRFunc* func)
{
    auto andResult = BUILD->CreateAnd(gprVal(instr.ops[1]), zExt64(i16Const(instr.ops[2])), "and");
    BUILD->CreateStore(andResult, func->getRegister("RR", instr.ops[0]));

    llvm::Value* LT = zExt32(BUILD->CreateICmpSLT(andResult, i64Const(0), "lt"));
    llvm::Value* GT = zExt32(BUILD->CreateICmpSGT(andResult, i64Const(0), "gt"));
    llvm::Value* EQ = zExt32(BUILD->CreateICmpEQ(andResult, i64Const(0), "eq"));
    // TODO
    llvm::Value* SO_bit = i32Const(0);
    llvm::Value* field = zExt32(BUILD->CreateOr(BUILD->CreateOr(BUILD->CreateOr(LT, BUILD->CreateShl(GT, 1, "sh"), "or"), BUILD->CreateShl(EQ, 2, "sh"), "or"), BUILD->CreateShl(SO_bit, 3, "sh"), "or"));

    setCRField(func, 0, field);
}

inline void xor_e(Instruction instr, IRFunc* func)
{
    auto xorResult = BUILD->CreateXor(gprVal(instr.ops[1]), gprVal(instr.ops[2]), "xor");
    BUILD->CreateStore(xorResult, func->getRegister("RR", instr.ops[0]));
}

inline void xori_e(Instruction instr, IRFunc* func)
{
    auto xorResult = BUILD->CreateXor(gprVal(instr.ops[1]), zExt64(i16Const( instr.ops[2])), "xor");
    BUILD->CreateStore(xorResult, func->getRegister("RR", instr.ops[0]));
}

inline void neg_e(Instruction instr, IRFunc* func)
{
    llvm::Value* negVal = BUILD->CreateNeg(gprVal(instr.ops[1]), "neg");
    BUILD->CreateStore(negVal, func->getRegister("RR", instr.ops[0]));
}

inline void nor_e(Instruction instr, IRFunc* func)
{
    llvm::Value* norVal = BUILD->CreateNeg(BUILD->CreateOr(gprVal(instr.ops[1]), gprVal(instr.ops[2]), "or"), "neg");
    BUILD->CreateStore(norVal, func->getRegister("RR", instr.ops[0]));
}



//
//// MATH
//

inline void mullw_e(Instruction instr, IRFunc* func)
{
    auto mulResult = trcTo32(BUILD->CreateMul(gprVal(instr.ops[1]), gprVal(instr.ops[2]), "Mul"));
    BUILD->CreateStore(mulResult, func->getRegister("RR", instr.ops[0]));
}

inline void mulli_e(Instruction instr, IRFunc* func)
{
    auto mulResult = BUILD->CreateMul(gprVal(instr.ops[1]), sign64(instr.ops[2]), "Mul");
    BUILD->CreateStore(mulResult, func->getRegister("RR", instr.ops[0]));
}


inline void divwx_e(Instruction instr, IRFunc* func) 
{
    llvm::Value* divisor = trcTo32(gprVal(instr.ops[2]));
    llvm::Value* v = BUILD->CreateSDiv(trcTo32(gprVal(instr.ops[1])), divisor, "div");
    v = zExt64(v);
    BUILD->CreateStore(v, func->getRegister("RR", instr.ops[0]));
    /*if (i.XO.Rc) {
        f.UpdateCR(0, v);
    }*/
}

inline void subf_e(Instruction instr, IRFunc* func)
{
    // in docs the operation is:
    // rD ← ~ (rA) + (rB) + 1
    // but can be simplified to -> rB - rA, THEY ARE SWAPPED
    llvm::Value* v = BUILD->CreateSub(gprVal(instr.ops[2]), gprVal(instr.ops[1]), "sub");
    BUILD->CreateStore(v, func->getRegister("RR", instr.ops[0]));
    
    // RC
    if (strcmp(instr.opcName.c_str(), "subfRC") == 0)
    {

        llvm::Value* LT = zExt32(BUILD->CreateICmpSLT(v, i64Const(0), "lt"));
        llvm::Value* GT = zExt32(BUILD->CreateICmpSGT(v, i64Const(0), "gt"));
        llvm::Value* EQ = zExt32(BUILD->CreateICmpEQ(v, i64Const(0), "eq"));
        // TODO
        llvm::Value* SO_bit = i32Const(0);
        llvm::Value* field = zExt32(BUILD->CreateOr(BUILD->CreateOr(BUILD->CreateOr(LT, BUILD->CreateShl(GT, 1, "sh"), "or"), BUILD->CreateShl(EQ, 2, "sh"), "or"), BUILD->CreateShl(SO_bit, 3, "sh"), "or"));

        setCRField(func, 0, field);
    }
}