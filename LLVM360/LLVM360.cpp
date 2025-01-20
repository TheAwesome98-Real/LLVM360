#include <iostream>
#include <fstream> 
#include <stdio.h>
#include "Utils.h"
#include "XexLoader.h"
#include "Instruction.h"
#include "InstructionDecoder.h"




int main()
{
	// Parse the XEX file. and use as file path a xex file in a folder relative to the exetubable
	XexImage* xex = new XexImage(L"MinecraftX360.xex");
    xex->LoadXex();
	


	for (size_t i = 0; i < xex->GetNumSections(); i++)
	{

		const Section* section = xex->GetSection(i);

		if (!section->CanExecute())
		{
			continue;
		}

		printf("Processing section %s\n", section->GetName().c_str());

		// compute code range
		const auto baseAddress = xex->GetBaseAddress();
		const auto sectionBaseAddress = baseAddress + section->GetVirtualOffset();
		const auto endAddress = baseAddress + section->GetVirtualOffset() + section->GetVirtualSize();

		// process all instructions in the code range
		auto address = sectionBaseAddress;
		bool forceBlockStart = false;
		int lastCount = -1;

		InstructionDecoder decoder(section);

		while (address < endAddress)
		{
			Instruction instruction;
			const auto instructionSize = decoder.GetInstructionAt(address, instruction);


			// decode instruction bytes to Intruction struct for later use
			// use the instruction and emit LLVM IR code



			// print the instruction
			printf("%08X: %s\n", address, instruction.opcName.c_str());
			// move to the next instruction
			address += instructionSize;
		}
	}

	

    printf("Hello, World!\n");
}