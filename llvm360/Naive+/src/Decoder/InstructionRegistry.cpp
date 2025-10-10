#include "InstructionRegistry.h"


InstructionRegistry g_instrRegistry;


InstructionRegistry::InstructionRegistry()
{
	registerInstructions();
}



void InstructionRegistry::InitialiseOpCodeKey(uint32_t mainOP, uint32_t extMASK)
{
	OpcodeKey val = { mainOP, extMASK };
	m_mainOPs.try_emplace(mainOP, val);
}

void InstructionRegistry::AddDescriptorToKey(uint32_t mainOP, std::string mnemonic, FormType type, uint32_t extOP)
{
	auto it = m_mainOPs.find(mainOP);
	if (it == m_mainOPs.end()) { printf("AddDescriptorToKey, Something is wrong :/, did you registerInstructions for this main Opcode? "); _CrtDbgBreak(); }
	OpcodeKey& key = it->second;
	uint32_t shift = __builtin_ctz(key.m_extMASK);
	InstructionDescriptor desc = { mnemonic, type };

	key.m_descriptors.try_emplace(extOP << shift, desc);
}


// search "MAIN OP: <n>"
void InstructionRegistry::registerInstructions()
{
	// MAIN OP: 0
	InitialiseOpCodeKey(0, 0);
	AddDescriptorToKey(0, "PADDING", FormType::FORM_PADDING);


	// MAIN OP: 14
	InitialiseOpCodeKey(14, 0);
	AddDescriptorToKey(14, "addi", FormType::FORM_D);

	// MAIN OP: 17
	InitialiseOpCodeKey(17, 0);
	AddDescriptorToKey(17, "sc", FormType::FORM_SC);

	// MAIN OP: 19
	InitialiseOpCodeKey(19, 0x7FE);
	AddDescriptorToKey(19, "bclrx", FormType::FORM_UNK, 16);



}
