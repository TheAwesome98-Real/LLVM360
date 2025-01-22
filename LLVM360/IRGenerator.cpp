#include "IRGenerator.h"
#include <string>
#include <functional> // for std::hash

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

#define EMIT_INST(emitter) emitter(instr);

bool IRGenerator::EmitInstruction(Instruction instr)
{
	// Create a hash function for strings
	std::hash<std::string> hash;

	switch (hash(instr))
	{
	default:
		printf("Instruction:   %s  not Implemented", instr.opcName.c_str());
		break;
	}

	return false;
}
