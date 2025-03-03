#pragma once
#include <string>
#include <functional>
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include <llvm/IR/NoFolder.h>
#include "llvm/IR/LLVMContext.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/raw_ostream.h"

#include "Xex/XexLoader.h"
#include "Decoder/Instruction.h"
#include <Windows.h>
#include <map>

class IRFunc;



class IRGenerator {
public:
  // llvm references
  llvm::IRBuilder<llvm::NoFolder>* m_builder;
  llvm::Module* m_module;
  // Xenon State stuff
  XexImage *m_xexImage;

  IRGenerator(XexImage *xex, llvm::Module* mod, llvm::IRBuilder<llvm::NoFolder>* builder);
  void Initialize();
  bool EmitInstruction(Instruction instr, IRFunc* func);

  llvm::Value* getRegister(const std::string& regName, int arrayIndex = -1, int index2 = -1);
  llvm::Value* getSPR(uint32_t n);
  void InitLLVM();
  void writeIRtoFile();
  void CxtSwapFunc();


  // TESTT
    // This is just for testing, to force the compiler to link the dll
    // 
    // Define the type of the variable (e.g., a pointer to the XenonState struct)
  llvm::FunctionType* funcType = llvm::FunctionType::get(
      llvm::Type::getVoidTy(m_module->getContext()), // Return type (void in this case)
      false                           // No parameters
  );

  llvm::Function* dllTestFunc = llvm::Function::Create(
      funcType,
      llvm::Function::ExternalLinkage,
      "dllHack",  
      m_module      
  );
  ////
  
  llvm::Value* xCtx;
  
  llvm::GlobalVariable* tlsVariable;
  llvm::StructType* XenonStateType = llvm::StructType::create(
      m_builder->getContext(), {
          llvm::Type::getInt64Ty(m_builder->getContext()),  // LR
          llvm::Type::getInt64Ty(m_builder->getContext()),  // CTR
          llvm::Type::getInt64Ty(m_builder->getContext()),  // MSR
          llvm::Type::getInt64Ty(m_builder->getContext()),  // XER
          llvm::Type::getInt32Ty(m_builder->getContext()),  // CR
          llvm::ArrayType::get(llvm::Type::getInt64Ty(m_builder->getContext()), 32),  // RR
          llvm::ArrayType::get(llvm::Type::getDoubleTy(m_builder->getContext()), 32),  // FR
          //llvm::ArrayType::get(llvm::ArrayType::get(llvm::Type::getInt64Ty(m_builder->getContext()), 2), 128) // VR
      }, "xenonState");

  void initFuncBody(IRFunc* func);
  IRFunc* getCreateFuncInMap(uint32_t address);
  bool isIRFuncinMap(uint32_t address);

  llvm::Function* mainFn;
  std::unordered_map<uint32_t, IRFunc*> m_function_map;
  std::unordered_map<uint32_t, Instruction> instrsList;
};
