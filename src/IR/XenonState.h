#pragma once

// Reference to registers and state of the Xenon CPU
// used as "reference" for LLVM IR code Generation
class XenonState {
public:
  llvm::Value *LR;
  llvm::Value *CTR;
  llvm::Value *MSR;
  llvm::Value *XER;
  llvm::Value *CR;


  /*static const uint32 FPSCR_ZE = 1U << 27;
  static const uint32 FPSCR_XE = 1U << 28;
  static const uint32 FPSCR_VE = 1U << 24;
  static const uint32 FPSCR_ZX = 1U << 5;
  static const uint32 FPSCR_VX = 1U << 2;
  static const uint32 FPSCR_XX = 1U << 6;

  uint32		FPSCR;
  uint32		SAT;*/

  // uint64 regs (r0, r1, r23 etc)
  std::array<llvm::Value*, 32> RR;

  // 32 Floating point Registers and
  // 128 VMX Vector Registers
  std::array<llvm::Value *, 32> FR;
  std::array<llvm::Value *, 128> VR;



 

  llvm::Value* getRR(uint32_t value)
  {
	  return RR[value];
  }

  llvm::Value* getSPR(uint32_t n)
  {
      uint32_t spr4 = (n & 0b1111100000) >> 5;
      uint32_t spr9 = n & 0b0000011111;
      
	  if (spr4 == 1) return XER;
	  if (spr4 == 8) return LR;
	  if (spr4 == 9) return CTR;
	  return NULL;
  }

  llvm::Value* getFR(uint32_t n)
  {
	  return FR[n];
  }
};
