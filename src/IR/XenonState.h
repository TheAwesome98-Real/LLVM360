#pragma once

// Reference to registers and state of the Xenon CPU
// used as "reference" for LLVM IR code Generation
class XenonState {
public:
  llvm::Value *LR;
  llvm::Value *CTR;
  llvm::Value *MSR;
  llvm::Value *XER;

  llvm::Value *CR[8];

  llvm::Value *CR0_SO;
  llvm::Value *CR1_SO;
  llvm::Value *CR2_SO;
  llvm::Value *CR3_SO;
  llvm::Value *CR4_SO;
  llvm::Value *CR5_SO;
  llvm::Value *CR6_SO;
  llvm::Value *CR7_SO;

  llvm::Value *CR0_EQ;
  llvm::Value *CR1_EQ;
  llvm::Value *CR2_EQ;
  llvm::Value *CR3_EQ;
  llvm::Value *CR4_EQ;
  llvm::Value *CR5_EQ;
  llvm::Value *CR6_EQ;
  llvm::Value *CR7_EQ;

  /*static const uint32 FPSCR_ZE = 1U << 27;
  static const uint32 FPSCR_XE = 1U << 28;
  static const uint32 FPSCR_VE = 1U << 24;
  static const uint32 FPSCR_ZX = 1U << 5;
  static const uint32 FPSCR_VX = 1U << 2;
  static const uint32 FPSCR_XX = 1U << 6;

  uint32		FPSCR;
  uint32		SAT;*/

  llvm::Value *R0;
  llvm::Value *R1;
  llvm::Value *R2;
  llvm::Value *R3;
  llvm::Value *R4;
  llvm::Value *R5;
  llvm::Value *R6;
  llvm::Value *R7;
  llvm::Value *R8;
  llvm::Value *R9;
  llvm::Value *R10;
  llvm::Value *R11;
  llvm::Value *R12;
  llvm::Value *R13;
  llvm::Value *R14;
  llvm::Value *R15;
  llvm::Value *R16;
  llvm::Value *R17;
  llvm::Value *R18;
  llvm::Value *R19;
  llvm::Value *R20;
  llvm::Value *R21;
  llvm::Value *R22;
  llvm::Value *R23;
  llvm::Value *R24;
  llvm::Value *R25;
  llvm::Value *R26;
  llvm::Value *R27;
  llvm::Value *R28;
  llvm::Value *R29;
  llvm::Value *R30;
  llvm::Value *R31;
  llvm::Value *R32;

  // 32 Floating point Registers and
  // 128 VMX Vector Registers
  std::array<llvm::Value *, 32> FR;
  std::array<llvm::Value *, 128> VR;
};
