#include "IRGenerator.h"
#include "InstructionEmitter.h"
#include <format>
#include <iomanip>
#include <sstream>


IRGenerator::IRGenerator(XexImage *xex, llvm::Module* mod, llvm::IRBuilder<llvm::NoFolder>* builder)
  : m_builder(builder)
  , m_module(mod) {
  m_xexImage = xex;
}


void IRGenerator::CxtSwapFunc()
{
    tlsVariable = new llvm::GlobalVariable(
        *m_module,
        XenonStateType->getPointerTo(),
        false,
        llvm::GlobalValue::ExternalLinkage,
        llvm::ConstantPointerNull::get(XenonStateType->getPointerTo()),
        "xCtx",
        nullptr,
        llvm::GlobalValue::GeneralDynamicTLSModel, 
        0
    );

    // getXCtxAddress func
    llvm::Type* retType = tlsVariable->getType();

    llvm::FunctionType* funcType = llvm::FunctionType::get(retType, false);
    llvm::Function* getXCtxAddressFn = llvm::Function::Create(
        funcType,
        llvm::GlobalValue::ExternalLinkage,
        "getXCtxAddress",
        m_module
    );

    getXCtxAddressFn->setDLLStorageClass(llvm::GlobalValue::DLLExportStorageClass);

    llvm::BasicBlock* entryBlock = llvm::BasicBlock::Create(m_builder->getContext(), "entry", getXCtxAddressFn);
    m_builder->SetInsertPoint(entryBlock);
    m_builder->CreateRet(tlsVariable);
}



void IRGenerator::InitLLVM() {
  
    this->CxtSwapFunc();

	// main function / entry point
    llvm::FunctionType* mainType = llvm::FunctionType::get(m_builder->getInt32Ty(), false);
    mainFn = llvm::Function::Create(mainType, llvm::Function::ExternalLinkage, "main", m_module);
    llvm::BasicBlock* entry = llvm::BasicBlock::Create(m_module->getContext(), "Entry", mainFn);
    m_builder->SetInsertPoint(entry);

    xCtx = m_builder->CreateLoad(
        tlsVariable->getType()->getPointerTo(),
        tlsVariable,
        "xCtx"
    );

    //
    // JUMP TO XEX ENTRY POINT
    //

    // add xex entry point function
	IRFunc* xex_entry = getCreateFuncInMap(m_xexImage->GetEntryAddress());
	initFuncBody(xex_entry);
    m_builder->SetInsertPoint(entry);
	m_builder->CreateCall(xex_entry->m_irFunc, { xCtx, llvm::ConstantInt::get(m_builder->getInt32Ty(), m_xexImage->GetEntryAddress())});

    
	m_builder->SetInsertPoint(entry);
    
    m_builder->CreateCall(dllTestFunc);

	m_builder->CreateRet(llvm::ConstantInt::get(m_builder->getInt32Ty(), 0));

	
}

void IRGenerator::Initialize() {
   InitLLVM();
}

bool first = true;
bool IRGenerator::EmitInstruction(Instruction instr, IRFunc* func) {
    // EEHHE can't use a std::string in a switch statement sooo...
    // let's map all the instruction names (as i already needed to do with a switch statement)
    // in a vector, so i can look up the instr opcode name in that vector and call the instruction
    // emitter
    // all emitter functions take as parameter <Instruction, IRGenerator>

  
    // <name>_e = <name>_emitter
    static std::unordered_map<std::string, std::function<void(Instruction, IRFunc*)>>
    instructionMap = 
    {
         {"nop", nop_e },
         {"twi", twi_e },
         {"mfspr", mfspr_e },
         {"stw", stw_e },
         {"stwu", stwu_e },
         {"lis", addis_e }, // it's a simplified mnemonic
         {"addis", addis_e },
         {"li", addi_e },
         {"addi", addi_e },
         {"lwz", lwz_e },
         {"mtspr", mtspr_e },
         {"or", orx_e },
         {"sth", sth_e },
         {"b", b_e },
         {"bl", bl_e },
         {"bclr", bclr_e },
         {"bcctr", bcctr_e },
         {"lhz", lhz_e },
         {"cmpw", cmpw_e},
         {"bc", bcx_e},
         {"add", add_e},
    };


  if (instructionMap.find(instr.opcName) != instructionMap.end()) {
    instructionMap[instr.opcName](instr, func);
    return true;
  } else {
        printf("Instruction:   %s  not Implemented\n", instr.opcName.c_str());
  }

  printf("----IR DUMP----\n\n\n");
  llvm::raw_fd_ostream& out = llvm::outs();
  m_module->print(out, nullptr);

  return false;
}




void IRGenerator::writeIRtoFile()
{
    

    printf("----IR DUMP----\n\n\n");
    llvm::raw_fd_ostream& out = llvm::outs();
    m_module->print(out, nullptr);

    std::error_code EC;
    llvm::raw_fd_ostream OS("bin/Debug/output.ll", EC);
    if (EC) {
        llvm::errs() << "Error writing to file: " << EC.message() << "\n";
    }
    else {
        m_module->print(OS, nullptr);
    }
    OS.close();
}



//
// Func Helpers
//

void IRGenerator::initFuncBody(IRFunc* func)
{
	if (func->m_irFunc != nullptr) 
    {
		printf("Function Body already exist\n");
        return;
	}

    func->genBody();
}

IRFunc* IRGenerator::getCreateFuncInMap(uint32_t address)
{
    if (isIRFuncinMap(address)) {
        return m_function_map.at(address);
    }
    IRFunc* func = new IRFunc();
	func->start_address = address;
	func->m_irGen = this;
    m_function_map.try_emplace(address, func);
    return func;
}

bool IRGenerator::isIRFuncinMap(uint32_t address)
{
    return m_function_map.find(address) != m_function_map.end();
}