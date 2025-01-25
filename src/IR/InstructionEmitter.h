#pragma once
#include "IRGenerator.h"

inline void mfspr_e(Instruction instr, IRGenerator *gen) 
{
    auto lrValue = gen->m_builder->CreateLoad(gen->m_builder->getInt64Ty(), gen->xenonCPU->getSPR(instr.ops[1]), "load_spr");
    gen->m_builder->CreateStore(lrValue, gen->xenonCPU->getRR(instr.ops[0]));
}

uint32_t signExtend(uint32_t value, int size) 
{
    if (value & (1 << (size - 1))) {
        return value | (~0 << size); 
    }
    return value; 
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
    gen->m_builder->CreateStore(lr_BB, gen->xenonCPU->LR); // Store in Link Register (LR)

    // emit branch in ir
    gen->m_builder->CreateBr(BB);

    // i actually think this is not needed for BL instruction, 
    // since bl is used if LK is 1 so it assumes that the function is returning 
    // so it will also branch to lr_bb
    gen->m_builder->CreateBr(lr_BB);

    // here i set the emit insertPoint to that basic block i created for LR
    gen->m_builder->SetInsertPoint(lr_BB);

}

