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



void IRGenerator::embedDataSections()
{

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
  
    CxtSwapFunc();

    
   

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

    FR = m_builder->CreateStructGEP(XenonStateType, xCtx, 6, "reg.FR");
    RR = m_builder->CreateStructGEP(XenonStateType, xCtx, 5, "reg.RR");
}

void IRGenerator::Initialize() {
   InitLLVM();
}

bool first = true;
bool IRGenerator::EmitInstruction(Instruction instr) {
    // EEHHE can't use a std::string in a switch statement sooo...
    // let's map all the instruction names (as i already needed to do with a switch statement)
    // in a vector, so i can look up the instr opcode name in that vector and call the instruction
    // emitter

    // all emitter functions take as parameter <Instruction, IRGenerator>

  

    // if it's the first instruction place the first basic block
    if (first)
    {
        first = false;
        llvm::BasicBlock* bb = getCreateBBinMap(instr.address);
        m_builder->CreateBr(bb);
        m_builder->SetInsertPoint(bb);
    }
    // <name>_e = <name>_emitter
    static std::unordered_map<std::string, std::function<void(Instruction, IRGenerator *)>>
    instructionMap = 
    {
         {"mfspr", mfspr_e },
         {"stw", stw_e },
         {"stwu", stwu_e },
         {"li", addi_e }, // it's a simplified mnemonic
    };

    // check if at address a bb is present if true then set the insertPoint to it
    if(isBBinMap(instr.address))
    {
        llvm::BasicBlock* bb = getCreateBBinMap(instr.address);
        m_builder->SetInsertPoint(bb);
    }
    // simple function detection if a bb is not already generated
    if((strcmp(instr.opcName.c_str(), "nop") == 0 || strcmp(instr.opcName.c_str(), "bclr") == 0)
        && !isBBinMap(instr.address + 4)){
        llvm::BasicBlock* bb = getCreateBBinMap(instr.address + 4);
        return true;
    }

  if (instructionMap.find(instr.opcName) != instructionMap.end()) {
    instructionMap[instr.opcName](instr, this);
    return true;
  } else {
        printf("Instruction:   %s  not Implemented\n", instr.opcName.c_str());
  }

  printf("----IR DUMP----\n\n\n");
  llvm::raw_fd_ostream& out = llvm::outs();
  m_module->print(out, nullptr);

  return false;
}


llvm::BasicBlock* IRGenerator::createBasicBlock(llvm::Function* func, uint32_t address)
{
    std::ostringstream oss;
    oss << "bb_" << std::hex << std::setfill('0') << std::setw(8) << address;
    return llvm::BasicBlock::Create(m_module->getContext(), oss.str(), mainFn);
}

llvm::BasicBlock* IRGenerator::getCreateBBinMap(uint32_t address)
{
    if (isBBinMap(address)){
        return codeBlocks_map.at(address)->bb_Block;
    }
    CodeBlock* block;
    block->isFunc = false;
    block->bb_Block = createBasicBlock(mainFn, address);
    codeBlocks_map.try_emplace(address, block);
    return block->bb_Block;
}

bool IRGenerator::isBBinMap(uint32_t address)
{
    return codeBlocks_map.find(address) != codeBlocks_map.end();
}


//
//  Control flow pass
//

bool IRGenerator::pass_controlFlow()
{
    return true;
}


//
// HELPERS
//


void IRGenerator::writeIRtoFile()
{
    m_builder->CreateCall(dllTestFunc);

    m_builder->CreateRet(llvm::ConstantInt::get(llvm::Type::getInt32Ty(m_module->getContext()), 0));
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
// LR   -> 0
// CTR  -> 1
// MSR  -> 2
// XER  -> 3
// CR   -> 4
// RR   -> 5
// FR   -> 6
// VX   -> 7
//

llvm::Value* IRGenerator::getRegister(const std::string& regName, int index1, int index2) 
{

    if (regName == "LR" || regName == "CTR" || regName == "MSR" || regName == "XER") 
    {
        int fieldIndex = (regName == "LR") ? 0 :
            (regName == "CTR") ? 1 :
            (regName == "MSR") ? 2 : 3;

     
        return m_builder->CreateStructGEP(XenonStateType, xCtx, fieldIndex, "lr_ptr");
    }
    else if (regName == "CR") 
    {
        return m_builder->CreateStructGEP(XenonStateType, xCtx, 4, "reg.CR");
    }
    else if (regName == "RR") 
    {
        assert(index1 != -1 && "Index for RR must be provided.");
        return m_builder->CreateGEP(
            llvm::Type::getInt64Ty(m_builder->getContext()),
            m_builder->CreateStructGEP(XenonStateType, xCtx, 5, "reg.RR"),
            llvm::ConstantInt::get(m_builder->getInt32Ty(), index1),
            "reg.RR[" + std::to_string(index1) + "]");
    }
    else if (regName == "FR") 
    {  
        assert(index1 != -1 && "Index for FR must be provided.");
        return m_builder->CreateGEP(
            llvm::Type::getInt64Ty(m_builder->getContext()),
            FR,
            llvm::ConstantInt::get(m_builder->getInt32Ty(), index1),
            "reg.FR[" + std::to_string(index1) + "]");
    }
    else {
        llvm::errs() << "Unknown register name: " << regName << "\n";
        return nullptr;
    }
}

llvm::Value* IRGenerator::getSPR(uint32_t n)
{
    uint32_t spr4 = (n & 0b1111100000) >> 5;
    uint32_t spr9 = n & 0b0000011111;

    if (spr4 == 1) return this->getRegister("XER");
    if (spr4 == 8) return this->getRegister("LR");
    if (spr4 == 9) return this->getRegister("CTR");
    return NULL;
}