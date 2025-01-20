#pragma once
#include <cstdint>
#include <string>


struct Instruction
{
	uint32_t address;
	uint32_t completeInstr; 
	std::string opcName;

	uint32_t op1;
	uint32_t op2;
	uint32_t op3;
	uint32_t op4;
	uint32_t op5;
	uint32_t op6;
};