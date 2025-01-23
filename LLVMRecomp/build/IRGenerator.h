#pragma once
#include <string>
#include <functional>
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include <llvm/IR/NoFolder.h>
#include "XenonState.h"
#include "XexLoader.h"
#include "Instruction.h"
#include <Windows.h>

class IRGenerator {
public:
  // llvm references
  llvm::IRBuilder<llvm::NoFolder> &m_builder;
  llvm::Module &m_module;

  // Xenon State stuff
  XexImage *m_xexImage;
  XenonState *xenonCPU;

  IRGenerator(XexImage *xex, llvm::Module &mod, llvm::IRBuilder<llvm::NoFolder> &builder);
  void Initialize();
  bool EmitInstruction(Instruction instr);

private:
  void InitLLVM();
};
