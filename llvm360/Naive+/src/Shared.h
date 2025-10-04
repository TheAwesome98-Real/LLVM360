#pragma once


#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <string>
#include <stdint.h>
#include <stdio.h>
#include <sstream>
#include <iomanip>
#include <conio.h>  // for _kbhit
#include <chrono>

#include <Xex/XexLoader.h>

#include "Decoder/Instruction.h"
#include "Decoder/InstructionDecoder.h"

#include "IR/IRGenerator.h"
#include <IR/Unit/UnitTesting.h>
#include <IR/IRFunc.h>


// Naive+ stuff
XexImage* loadedXex;
IRGenerator* g_irGen;
llvm::LLVMContext cxt;
llvm::Module* mod = new llvm::Module("Xenon", cxt);
llvm::IRBuilder<llvm::NoFolder> builder(cxt);

uint32_t Swap32(uint32_t val) {
    return ((((val) & 0xff000000) >> 24) | (((val) & 0x00ff0000) >> 8) | (((val) & 0x0000ff00) << 8) |
        (((val) & 0x000000ff) << 24));
}


#define NAIVE_EXPORT

#ifdef NAIVE_EXPORT
#define NAIVE_EXPORT __declspec(dllexport)
#else
#define NAIVE_EXPORT __declspec(dllimport)
#endif


enum BinaryType
{
	BIN_XEX,
	BIN_PE,
	BIN_UNKNOWN
};


// Precompiled Binary Handle
// it's used to as communication between Naive+ and emulator to define a loaded binary
struct PBinaryHandle
{
	std::string m_imagePath;
	BinaryType m_type;
	uint32_t m_ID;
	std::vector<Instruction> m_binInstr;

	void LoadBinary();
	void RecompileBinary();
};

extern "C"
{

	// Translate a binary located at <path> and return a generated handle describing it
	// used also to load cached binaries <useCache> (true)
	// the PBinaryHandle is generated everytime the binary it's translated or reloaded from cache
	NAIVE_EXPORT PBinaryHandle* TranslateBinary(std::string path, bool useCache = false);
}