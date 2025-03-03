#pragma once
#include <IR/InstructionEmitter.h>
//#include "../InstructionEmitter.h"

//
// This header contains all the various unit tests of every single insturction to ensure
// it the instruction works exaclty how it should
// i'll add unit tests each time i implement a new instruction emitter
// i can view the register dump via the runtime CpuContext dumper
//

inline void unit_mfspr(IRFunc* func, IRGenerator* gen)
{
	Instruction instr;
	instr.ops.push_back(15); // GPR R12
	instr.ops.push_back(256); // SPR 8 (LR)
	mfspr_e(instr, func);
}

inline void unit_stfd(IRFunc* func, IRGenerator* gen)
{
	Instruction instr;
	instr.ops.push_back(0); // FR FR0
	instr.ops.push_back(0xFFD0); // Displ FFD0
	instr.ops.push_back(1); // gpr R1
	
	auto stack_val = llvm::ConstantInt::get(gen->m_builder->getInt64Ty(), 0x3232, "stack_val");
	gen->m_builder->CreateStore(stack_val, func->getRegister("FR", instr.ops[0]));

	stfd_e(instr, func);
}

inline void unit_stw(IRFunc* func, IRGenerator* gen)
{
	Instruction instr;
	instr.ops.push_back(12); // RR RR12
	instr.ops.push_back(0xFFF8); // Displ FFF8
	instr.ops.push_back(1); // gpr R1

	auto stack_val = llvm::ConstantInt::get(gen->m_builder->getInt64Ty(), 0x1102030405060708, "stack_val");
	gen->m_builder->CreateStore(stack_val, func->getRegister("RR", instr.ops[0]));

	stw_e(instr, func);

	auto load = gen->m_builder->CreateLoad(gen->m_builder->getInt32Ty(), getEA(func, instr.ops[1], instr.ops[2]), "load");
	gen->m_builder->CreateStore(load, func->getRegister("RR", 14));
}

inline void unit_stwu(IRFunc* func, IRGenerator* gen)
{
	Instruction instr;
	instr.ops.push_back(12); // RR RR12
	instr.ops.push_back(0xFFF8); // Displ FFF8
	instr.ops.push_back(1); // gpr R1

	auto stack_val = llvm::ConstantInt::get(gen->m_builder->getInt64Ty(), 0x1102030405060708, "stack_val");
	gen->m_builder->CreateStore(stack_val, func->getRegister("RR", instr.ops[0]));

	// test stack update
	auto loadstack = gen->m_builder->CreateLoad(gen->m_builder->getInt64Ty(), func->getRegister("RR", 1), "load");
	gen->m_builder->CreateStore(loadstack, func->getRegister("RR", 2));


	stwu_e(instr, func);

	//
	// note for debugging with unit testing
	// since i'm updating the stack in this unit test, here i store the original value in r2 so i can use that
	//
	auto load = gen->m_builder->CreateLoad(gen->m_builder->getInt32Ty(), getEA(func, instr.ops[1], 2), "load");
	gen->m_builder->CreateStore(load, func->getRegister("RR", 14));
}

inline void unit_lis(IRFunc* func, IRGenerator* gen)
{
	Instruction instr;
	instr.ops.push_back(21); // rD R21
	instr.ops.push_back(0x0); // rA
	instr.ops.push_back(21); // imm

	addis_e(instr, func);
}

inline void unit_li(IRFunc* func, IRGenerator* gen)
{
	Instruction instr;
	instr.ops.push_back(21); // rD R21
	instr.ops.push_back(0x0); // rA
	instr.ops.push_back(211); // imm

	addi_e(instr, func);
}

inline void unit_lwz(IRFunc* func, IRGenerator* gen)
{
	Instruction instr;
	instr.ops.push_back(21); // rD R21
	instr.ops.push_back(0xfff8); // displ
	instr.ops.push_back(1); // rA (stack pointer)

	// to test zero
	auto stack_val = llvm::ConstantInt::get(gen->m_builder->getInt64Ty(), 0x1122334455667788, "stack_val");
	gen->m_builder->CreateStore(stack_val, func->getRegister("RR", instr.ops[0]));

	unit_stw(func, gen); // load value in stack for testing
	lwz_e(instr, func);
}

inline void unit_cmpw(IRFunc* func, IRGenerator* gen)
{
	Instruction instr;
	instr.ops.push_back(0x06); // rD R21
	instr.ops.push_back(0x0); // displ
	instr.ops.push_back(0x0b); // rA (stack pointer
	instr.ops.push_back(0x0a); // rA (stack pointer)

	// to test zero
	auto a_val = llvm::ConstantInt::get(gen->m_builder->getInt64Ty(), 13, "stack_val");
	gen->m_builder->CreateStore(a_val, func->getRegister("RR", instr.ops[2]));
	auto b_val = llvm::ConstantInt::get(gen->m_builder->getInt64Ty(), 13, "stack_val");
	gen->m_builder->CreateStore(b_val, func->getRegister("RR", instr.ops[3]));

	
	cmpw_e(instr, func);
}