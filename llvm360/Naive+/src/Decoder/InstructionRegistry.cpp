#include "InstructionRegistry.h"


InstructionRegistry g_instrRegistry;


InstructionRegistry::InstructionRegistry()
{
	registerInstructions();
}



void InstructionRegistry::InitialiseOpCodeKey(uint32_t mainOP, uint32_t extOP, uint32_t extMASK)
{
	OpcodeKey val = { mainOP, extOP, extMASK };
	m_mainOPs.try_emplace(mainOP, val);
}

void InstructionRegistry::AddDescriptorToKey(uint32_t mainOP, std::string mnemonic, FormType type, uint32_t extOP)
{
	auto it = m_mainOPs.find(mainOP);
	if (it == m_mainOPs.end()) { printf("AddDescriptorToKey, Something is wrong :/, did you registerInstructions for this main Opcode? "); _CrtDbgBreak(); }
	OpcodeKey key = it->second;
	InstructionDescriptor desc = { mnemonic, type };

	key.m_descriptors.try_emplace(extOP, desc);
}


// search "MAIN OP: <n>"
void InstructionRegistry::registerInstructions()
{

	// MAIN OP: 14
	InitialiseOpCodeKey(14, 0, 0);
	AddDescriptorToKey(14, "addi", FormType::FORM_D);
	// AddDescriptorToKey(14, "addi", FormType::FORM_D);
	// AddDescriptorToKey(14, "addi", FormType::FORM_D);
	// AddDescriptorToKey(14, "addi", FormType::FORM_D);
	// AddDescriptorToKey(14, "addi", FormType::FORM_D);
	// ... ... ..
}
