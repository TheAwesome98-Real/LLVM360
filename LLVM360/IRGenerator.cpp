#include "IRGenerator.h"
#include "InstructionEmitter.h"
#include <string>
#include <functional> 

IRGenerator::IRGenerator(XexImage* xex)
{
	m_xexImage = xex;
	xenonCPU = new XenonState();
}

void IRGenerator::InitLLVM()
{
}

void IRGenerator::Initialize()
{
	InitLLVM();
}



// emitter Caller
#define EMIT_INST(instructionName) emitter_##instructionName(instr);

bool IRGenerator::EmitInstruction(Instruction instr)
{
	// EEHHE can't use a std::string in a switch statement sooo...
	// let's map all the instruction names (as i already needed to do with a switch statement) 
	// in a vector, so i can look up the instr opcode name in that vector and call the instruction emitter


	// all emitter functions take as parameter <Instruction, IRGenerator>

    // <name>_e = <name>_emitter
    static std::unordered_map<std::string, std::function<void(Instruction, IRGenerator*)>> instructionMap =
    {
        {"mfspr", mfspr_e },
    };

    
    if (instructionMap.find(instr.opcName) != instructionMap.end())
    {
        instructionMap[instr.opcName](instr, this);
    }
    else 
    {
        printf("Instruction:   %s  not Implemented", instr.opcName.c_str());
    }


	return false;
}
