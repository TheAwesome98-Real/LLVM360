#pragma once
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/IRBuilder.h"
#include "XenonState.h"
#include "XexLoader.h"
#include "Instruction.h"


class IRGenerator
{
public:
	// llvm references
	static std::unique_ptr<llvm::LLVMContext> m_llvmCxt;
	static std::unique_ptr<llvm::IRBuilder<>> m_builder;
	static std::unique_ptr<llvm::Module> m_module;
	
	// Xenon State stuff
	XexImage* m_xexImage;
	XenonState* xenonCPU;


	IRGenerator(XexImage* xex);
	void Initialize();
	bool EmitInstruction(Instruction instr);
private:
	void InitLLVM();
};