#pragma once
#include <string>
#include <functional>
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include <llvm/IR/NoFolder.h>
#include "XenonState.h"
#include "Xex/XexLoader.h"
#include "Decoder/Instruction.h"
#include <Windows.h>

class IRGenerator {
public:
  // llvm references
  llvm::IRBuilder<llvm::NoFolder>* m_builder;
  llvm::Module* m_module;

  // Xenon State stuff
  XexImage *m_xexImage;
  XenonState *xenonCPU;

  IRGenerator(XexImage *xex, llvm::Module* mod, llvm::IRBuilder<llvm::NoFolder>* builder);
  void Initialize();
  bool EmitInstruction(Instruction instr);


  void InitLLVM();
  llvm::BasicBlock* createBasicBlock(llvm::Function* mainFn, uint32_t address);
  llvm::BasicBlock* getCreateBBinMap(uint32_t address);
  bool isBBinMap(uint32_t address);

  llvm::Function* mainFn;
  std::unordered_map<uint32_t, llvm::BasicBlock*> bb_map;
};
