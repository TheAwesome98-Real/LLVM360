#pragma once
//#include "../InstructionEmitter.h"

/*/
// This header contains all the various unit tests of every single insturction to ensure
// it the instruction works exaclty how it should
// i'll add unit tests each time i implement a new instruction emitter
// i can view the register dump via the runtime CpuContext dumper
//

inline void unit_mfspr(IRFunc* func)
{
	Instruction instr;
	instr.ops.push_back(12); // GPR R12
	instr.ops.push_back(256); // SPR 8 (LR)
	mfspr_e(instr, func);
}

inline void unit_stfd(IRFunc* func)
{
	Instruction instr;
	instr.ops.push_back(0); // FR FR0
	instr.ops.push_back(0xFFD0); // Displ FFD0
	instr.ops.push_back(1); // gpr R1
	
	auto stack_val = llvm::ConstantInt::get(g_irGen->m_builder->getInt64Ty(), 0x3232, "stack_val");
	g_irGen->m_builder->CreateStore(stack_val, g_irGen->getRegister("FR", instr.ops[0]));

	stfd_e(instr, func);
}

inline void unit_stw(IRFunc* func)
{
	Instruction instr;
	instr.ops.push_back(12); // RR RR12
	instr.ops.push_back(0xFFF8); // Displ FFF8
	instr.ops.push_back(1); // gpr R1

	auto stack_val = llvm::ConstantInt::get(g_irGen->m_builder->getInt64Ty(), 0x1102030405060708, "stack_val");
	g_irGen->m_builder->CreateStore(stack_val, g_irGen->getRegister("RR", instr.ops[0]));

	stw_e(instr, gen);

	auto load = g_irGen->m_builder->CreateLoad(g_irGen->m_builder->getInt32Ty(), getEA(gen, instr.ops[1], instr.ops[2]), "load");
	g_irGen->m_builder->CreateStore(load, g_irGen->getRegister("RR", 14));
}

inline void unit_stwu(IRFunc* func)
{
	Instruction instr;
	instr.ops.push_back(12); // RR RR12
	instr.ops.push_back(0xFFF8); // Displ FFF8
	instr.ops.push_back(1); // gpr R1

	auto stack_val = llvm::ConstantInt::get(g_irGen->m_builder->getInt64Ty(), 0x1102030405060708, "stack_val");
	g_irGen->m_builder->CreateStore(stack_val, g_irGen->getRegister("RR", instr.ops[0]));

	// test stack update
	auto loadstack = g_irGen->m_builder->CreateLoad(g_irGen->m_builder->getInt64Ty(), g_irGen->getRegister("RR", 1), "load");
	g_irGen->m_builder->CreateStore(loadstack, g_irGen->getRegister("RR", 2));


	stwu_e(instr, gen);

	//
	// note for debugging with unit testing
	// since i'm updating the stack in this unit test, here i store the original value in r2 so i can use that
	//
	auto load = g_irGen->m_builder->CreateLoad(g_irGen->m_builder->getInt32Ty(), getEA(gen, instr.ops[1], 2), "load");
	g_irGen->m_builder->CreateStore(load, g_irGen->getRegister("RR", 14));
}

inline void unit_lis(IRFunc* func)
{
	Instruction instr;
	instr.ops.push_back(21); // rD R21
	instr.ops.push_back(0x0); // rA
	instr.ops.push_back(21); // imm

	addis_e(instr, gen);
}

inline void unit_li(IRFunc* func)
{
	Instruction instr;
	instr.ops.push_back(21); // rD R21
	instr.ops.push_back(0x0); // rA
	instr.ops.push_back(211); // imm

	addi_e(instr, gen);
}

inline void unit_lwz(IRFunc* func)
{
	Instruction instr;
	instr.ops.push_back(21); // rD R21
	instr.ops.push_back(0xfff8); // displ
	instr.ops.push_back(1); // rA (stack pointer)

	// to test zero
	auto stack_val = llvm::ConstantInt::get(g_irGen->m_builder->getInt64Ty(), 0x1122334455667788, "stack_val");
	g_irGen->m_builder->CreateStore(stack_val, g_irGen->getRegister("RR", instr.ops[0]));

	unit_stw(gen); // load value in stack for testing
	lwz_e(instr, gen);
}*/