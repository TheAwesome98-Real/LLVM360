#include "IRGenerator.h"
#include "InstructionEmitter.h"
#include <format>
#include <iomanip>
#include <sstream>


IRGenerator::IRGenerator(XexImage *xex, llvm::Module* mod, llvm::IRBuilder<llvm::NoFolder>* builder)
  : m_builder(builder)
  , m_module(mod) {
  m_xexImage = xex;
  xenonCPU = new XenonState();
}

void IRGenerator::InitLLVM() {

	// main function / entry point
    llvm::FunctionType* mainType = llvm::FunctionType::get(m_builder->getVoidTy(), false);
    mainFn = llvm::Function::Create(mainType, llvm::Function::ExternalLinkage, "main", m_module);
    //mainFn->setCallingConv(llvm::CCallConv)
    llvm::BasicBlock* entry = llvm::BasicBlock::Create(m_module->getContext(), "Entry", mainFn);
    m_builder->SetInsertPoint(entry);

    // Initialize SSA Registers values
    // General Regs
    for (size_t i = 0; i < xenonCPU->RR.size(); i++){
        xenonCPU->RR.at(i) = m_builder->CreateAlloca(m_builder->getInt64Ty(), nullptr, "r" + std::to_string(i));
    }
    // Float
    for (size_t f = 0; f < xenonCPU->FR.size(); f++) {
        xenonCPU->FR.at(f) = m_builder->CreateAlloca(m_builder->getDoubleTy(), nullptr, "fr" + std::to_string(f));
    }

    xenonCPU->LR = m_builder->CreateAlloca(m_builder->getInt64Ty(), nullptr, "lr");
    xenonCPU->CR = m_builder->CreateAlloca(m_builder->getInt32Ty(), nullptr, "cr");
    xenonCPU->CTR = m_builder->CreateAlloca(m_builder->getInt64Ty(), nullptr, "ctr");

    // add xex entrypoint basic block to list;
    llvm::BasicBlock* xex_entry = llvm::BasicBlock::Create(m_module->getContext(), "XEX_entry", mainFn);
    bb_map.try_emplace(m_xexImage->m_xexData.exe_entry_point, xex_entry);

    
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
        llvm::BasicBlock* bb = createBasicBlock(mainFn, instr.address);
        bb_map.try_emplace(instr.address, bb);
        m_builder->SetInsertPoint(bb);
    }
    // <name>_e = <name>_emitter
    static std::unordered_map<std::string, std::function<void(Instruction, IRGenerator *)>>
    instructionMap = 
    {
         {"mfspr", mfspr_e },
         {"bl", bl_e },
         {"stfd", stfd_e },
         {"stwu", stwu_e },
         {"addi", addi_e },
         {"li", addi_e }, // load immediate is just an addi but rA is 0
         {"lis", addis_e }, // same thing
         {"addis", addis_e },
         {"or", orx_e },
         {"orRC", orx_e },
         {"bc", bcx_e },
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
        return bb_map.at(address);
    }
    llvm::BasicBlock* bb = createBasicBlock(mainFn, address);
    bb_map.try_emplace(address, bb);
    return bb;
}

bool IRGenerator::isBBinMap(uint32_t address)
{
    return bb_map.find(address) != bb_map.end();
}