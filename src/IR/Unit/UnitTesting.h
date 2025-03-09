#pragma once
#include <IR/InstructionEmitter.h>
//#include "../InstructionEmitter.h"

//
// This header contains all the various unit tests of every single insturction to ensure
// it the instruction works exaclty how it should
// i'll add unit tests each time i implement a new instruction emitter
// i can view the register dump via the runtime CpuContext dumper
//

inline void unit_mfspr(IRFunc* func, IRGenerator* gen, std::vector<uint32_t> ops)
{
	Instruction instr;
	instr.ops = ops;
	mfspr_e(instr, func);
}

inline void unit_stfd(IRFunc* func, IRGenerator* gen, std::vector<uint32_t> ops)
{
	Instruction instr;
	instr.ops = ops;
	stfd_e(instr, func);
}

inline void unit_stw(IRFunc* func, IRGenerator* gen, std::vector<uint32_t> ops)
{
	Instruction instr;
	instr.ops = ops;
	stw_e(instr, func);
}

inline void unit_stwu(IRFunc* func, IRGenerator* gen, std::vector<uint32_t> ops)
{
	Instruction instr;
	instr.ops = ops;
	stwu_e(instr, func);
}

inline void unit_lis(IRFunc* func, IRGenerator* gen, std::vector<uint32_t> ops)
{
	Instruction instr;
	instr.ops = ops;
	addis_e(instr, func);
}

inline void unit_li(IRFunc* func, IRGenerator* gen, std::vector<uint32_t> ops)
{
	Instruction instr;
	instr.ops = ops;
	addi_e(instr, func);
}

inline void unit_divw(IRFunc* func, IRGenerator* gen, std::vector<uint32_t> ops)
{
	Instruction instr;
	instr.ops = ops;
	divwx_e(instr, func);
}

inline void unit_mulli(IRFunc* func, IRGenerator* gen, std::vector<uint32_t> ops)
{
	Instruction instr;
	instr.ops = ops;
	mulli_e(instr, func);
}

inline void unit_subf(IRFunc* func, IRGenerator* gen, std::vector<uint32_t> ops)
{
	Instruction instr;
	instr.ops = ops;
	subf_e(instr, func);
}

inline void unit_lwz(IRFunc* func, IRGenerator* gen, std::vector<uint32_t> ops)
{
	Instruction instr;
	instr.ops = ops;
	lwz_e(instr, func);
}

inline void unit_cmpw(IRFunc* func, IRGenerator* gen, std::vector<uint32_t> ops)
{
	Instruction instr;
	instr.ops = ops;
	cmpw_e(instr, func);
}

inline void unit_bclr(IRFunc* func, IRGenerator* gen, std::vector<uint32_t> ops)
{
	Instruction instr;
	instr.ops = ops;
	bclr_e(instr, func);
}

inline void unit_srawi(IRFunc* func, IRGenerator* gen, std::vector<uint32_t> ops)
{
	Instruction instr;
	instr.ops = ops;
	srawi_e(instr, func);
}