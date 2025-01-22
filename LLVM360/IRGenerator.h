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
	static std::unique_ptr<llvm::LLVMContext> TheContext;
	static std::unique_ptr<llvm::IRBuilder<>> Builder;
	static std::unique_ptr<llvm::Module> TheModule;
	
	// Xenon State stuff
	XexImage* m_xexImage;
	XenonState* xenonCPU;


	IRGenerator(XexImage* xex);
	void Initialize();
	bool EmitInstruction(Instruction instr);
private:
	void InitLLVM();
};