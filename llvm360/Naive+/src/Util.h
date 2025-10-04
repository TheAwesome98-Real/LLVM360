#pragma once
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <sstream>
#include <iomanip>
#include "Decoder/Instruction.h"
#include "Decoder/InstructionDecoder.h"
#include <chrono>
#include "IR/IRGenerator.h"
#include <Xex/XexLoader.h>
#include <conio.h>  // for _kbhit
#include <IR/Unit/UnitTesting.h>
#include <IR/IRFunc.h>


enum LogLevel
{
	LOG_INFO,
	LOG_WARNING,
	LOG_ERROR,
	LOG_DEBUG,
	LOG_FATAL
};

const char* getColor(LogLevel level) 
{
    switch (level) 
    {
    case LogLevel::LOG_INFO:    return "\033[32m"; // Green
    case LogLevel::LOG_WARNING: return "\033[33m"; // Yellow
    case LogLevel::LOG_ERROR:   return "\033[31m"; // Red
    case LogLevel::LOG_DEBUG:   return "\033[36m"; // Cyan
    case LogLevel::LOG_FATAL:   return "\033[35m"; // Magenta
    default: return "";
    }
}

void LOG_PRINT(LogLevel level, const char* name, const char* fmt, ...)
{
    printf("%s<%s> -> ", getColor(level), name);
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    printf("\033[0m\n"); 
}

#define LOG_INFO(name, fmt, ...)    LOG_PRINT(LogLevel::LOG_INFO, name, fmt, __VA_ARGS__)
#define LOG_WARNING(name, fmt, ...) LOG_PRINT(LogLevel::LOG_WARNING, name, fmt, __VA_ARGS__)
#define LOG_ERROR(name, fmt, ...)   LOG_PRINT(LogLevel::LOG_ERROR, name, fmt, __VA_ARGS__)
#define LOG_DEBUG(name, fmt, ...)   LOG_PRINT(LogLevel::LOG_DEBUG, name, fmt, __VA_ARGS__)
#define LOG_FATAL(name, fmt, ...)   LOG_PRINT(LogLevel::LOG_FATAL, name, fmt, __VA_ARGS__)



// Debug
bool printINST = false;
bool printFile = false; 
bool genLLVMIR = true;
bool isUnitTesting = false;
bool doOverride = false; // if it should override the endAddress to debug
bool dbCallBack = true; // enables debug callbacks, break points etc
bool dumpIRConsole = false;
uint32_t overAddr = 0x82060150;

// Benchmark / static analysis
uint32_t instCount = 0;

// Naive+ stuff
XexImage* loadedXex;
IRGenerator* g_irGen;
llvm::LLVMContext cxt;
llvm::Module* mod = new llvm::Module("Xenon", cxt);
llvm::IRBuilder<llvm::NoFolder> builder(cxt);

Section* findSection(std::string name);

IRFunc* isInFuncBound(uint32_t addr)
{
    for (const auto& pair : g_irGen->m_function_map)
    {
        IRFunc* func = pair.second;
        if (addr > func->start_address && addr < func->end_address)
        {
            return func;
        }
    }
    return nullptr;
}

Section* findSection(std::string name)
{
    for (uint32_t i = 0; i < loadedXex->GetNumSections(); i++)
    {
        if (strcmp(loadedXex->GetSection(i)->GetName().c_str(), name.c_str()) == 0)
        {
            return loadedXex->GetSection(i);
        }
    }

    printf("No Section with name: s& found", name.c_str());
    return nullptr;
}


uint32_t Swap32(uint32_t val) {
    return ((((val) & 0xff000000) >> 24) | (((val) & 0x00ff0000) >> 8) | (((val) & 0x0000ff00) << 8) |
        (((val) & 0x000000ff) << 24));
}
