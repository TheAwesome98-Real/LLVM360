#include <iostream>
#include <fstream>
#include <stdio.h>
#include <sstream>
#include <iomanip>
#include "misc/Utils.h"
#include "Decoder/Instruction.h"
#include "Decoder/InstructionDecoder.h"
#include <chrono>
#include "IR/IRGenerator.h"
#include <Xex/XexLoader.h>
#include <conio.h>  // for _kbhit
#include <IR/Unit/UnitTesting.h>

// Debug
bool printINST = true;
bool genLLVMIR = true;
bool isUnitTesting = true;
bool doOverride = false; // if it should override the endAddress to debug
uint32_t overAddr = 0x82060150;

// Benchmark / static analysis
uint32_t instCount = 0;

// Naive+ stuff
std::unordered_map<uint32_t, Instruction> instructionsList;
XexImage* loadedXex;
IRGenerator* irGen;
llvm::LLVMContext cxt;
llvm::Module* mod = new llvm::Module("Xenon", cxt);
llvm::IRBuilder<llvm::NoFolder> builder(cxt);




void SaveSectionToBin(const char* filename, const uint8_t* dataPtr, size_t dataSize) 
{
    std::ofstream binFile(filename, std::ios::binary);
    if (!binFile) 
    {
        printf("Error: Cannot open file %s for writing\n", filename);
        return;
    }
    binFile.write(reinterpret_cast<const char*>(dataPtr), dataSize);
    binFile.close();
}

void saveSection(const char* path, uint32_t idx)
{
    const Section* section = loadedXex->GetSection(idx);
    const auto baseAddress = loadedXex->GetBaseAddress();
    const auto sectionBaseAddress = baseAddress + section->GetVirtualOffset();
    auto endAddress = baseAddress + section->GetVirtualOffset() + section->GetVirtualSize();
    auto address = sectionBaseAddress;
    const uint8_t* m_imageDataPtr = (const uint8_t*)section->GetImage()->GetMemory() + (section->GetVirtualOffset() - section->GetImage()->GetBaseAddress());
    const uint64_t offset = address - section->GetVirtualOffset();

    SaveSectionToBin(path, (uint8_t*)m_imageDataPtr + offset, section->GetVirtualSize());
}

void unitTest(IRGenerator* gen)
{
    //unit_mfspr(gen);
    //unit_stfd(gen);
    unit_stwu(gen);
    //
    // DUMP
    //
    printf("----IR DUMP----\n\n\n");
    llvm::raw_fd_ostream& out = llvm::outs();
    gen->m_module->print(out, nullptr);
    gen->writeIRtoFile();
}


bool pass_Decode()
{

    bool ret = true;

    for (size_t i = 0; i < loadedXex->GetNumSections(); i++)
    {
        const Section* section = loadedXex->GetSection(i);

        if (!section->CanExecute()) 
        {
            continue;
        }
        printf("Decoding Instructions for section %s\n", section->GetName().c_str());
        // compute code range
        const auto baseAddress = loadedXex->GetBaseAddress();
        const auto sectionBaseAddress = baseAddress + section->GetVirtualOffset();
        auto endAddress = baseAddress + section->GetVirtualOffset() + section->GetVirtualSize();
        auto address = sectionBaseAddress;

        InstructionDecoder decoder(section);
        if (doOverride) endAddress = overAddr;
        while (address < endAddress)
        {
            Instruction instruction;
            const auto instructionSize = decoder.GetInstructionAt(address, instruction);
            if (instructionSize == 0)
            {
                printf("Failed to decode instruction at %08X\n", address);
                ret = false;
                break;
            }

            // add instruction to map
            instructionsList.try_emplace(address, instruction);

            if (printINST) {
                // print raw PPC decoded instruction + operands
                std::ostringstream oss;
                oss << std::hex << std::uppercase << std::setfill('0');
                oss << std::setw(8) << address << ":   " << instruction.opcName;

                for (size_t i = 0; i < instruction.ops.size(); ++i) {
                    oss << " " << std::setw(2) << static_cast<int>(instruction.ops.at(i));
                }

                std::string output = oss.str();
                printf("%s\n", output.c_str());
            }

            address += instructionSize; // always 4
            instCount++; // instruction count
        }
    }

    return ret;
}

bool pass_Emit()
{
    bool ret = true;
    uint32_t addrOverrider = 0x82060150;

    for (size_t i = 0; i < loadedXex->GetNumSections(); i++)
    {
        const Section* section = loadedXex->GetSection(i);

        if (!section->CanExecute())
        {
            continue;
        }
        printf("Emitting IR for section %s\n", section->GetName().c_str());
        // compute code range
        const auto baseAddress = loadedXex->GetBaseAddress();
        const auto sectionBaseAddress = baseAddress + section->GetVirtualOffset();
        auto endAddress = baseAddress + section->GetVirtualOffset() + section->GetVirtualSize();
        auto address = sectionBaseAddress;

        if (doOverride) endAddress = overAddr;
        while (address < endAddress)
        {
            // print LLVM IR Output
            if (genLLVMIR)
            {
                bool result;
                if (isUnitTesting)
                {
                    unitTest(irGen);
                    result = false;
                }
                else
                {
                    result = irGen->EmitInstruction(instructionsList.at(address));
                }

                if (!result) {
                    __debugbreak();
                    return 1;
                }
            }

            address += 4;
        }
    }

    irGen->writeIRtoFile();

    return ret;
}



int main() 
{
    loadedXex = new XexImage(L"LLVMTest1.xex");
    loadedXex->LoadXex();
    irGen = new IRGenerator(loadedXex, mod, &builder);
    irGen->Initialize();

    printf("\n\n\n");
    auto start = std::chrono::high_resolution_clock::now();


    // 
    // --- Passes ---
    // 
    // Passes are *Not* optimization to speed up recompilation but rather to have a more
    // managable enviroment to scale up and keep each phase of the recompilation well
    // organized. they actually slow down a bit (literally seconds or tenths of a second)
    // recompilation compared to the more naive approach from before e.g just feeding the 
    // decoded instruction to the emitter and not store all instrs in a map and Then emit them
    // so it will slow down but it keeps emission phase good and easier to work with :}
    // 
    // Decode: decode ppc instructions and convert them into a Emitter friendly
    //         format and add them into an unordered map, the map slow down a bit from
    //         0.3 seconds to 0.9 seconds but it's useful for later, before i was just
    //         feeding instruction per instruction to the emitter.
    // 
    // ContrFlow: TODO, this is a control flow pass that detects part of the visible branching
    //            before emitting and the general skeleton of the executable, This can help with 
    //            Emission phase that could miss some branch labels etc, like it will help detect
    //            the code block of a VTable function (if this pass find it).
    //
    // Emit: this is the big and complex one, it take every instructions and emit equivalent IR Code
    //       compatible with the CPU Context, this will also resolve and patch all memory related operations
    //       e.g Load / Store in .rdata or .data section and more stuff.
    //

    // first recomp pass: Decode xex instructions
    if(!pass_Decode())
    {
        printf("something went wrong - Pass: DECODE\n");
        return -1;
    }

    irGen->instrsList = instructionsList;

    if (!irGen->pass_controlFlow())
    {
        printf("something went wrong - Pass: CONTROLFLOW\n");
        return -1;
    }

    // third recomp pass: Emit IR code
    if (!pass_Emit())
    {
        printf("something went wrong - Pass: EMIT\n");
        return -1;
    }

    // sections
    saveSection("bin/Debug/rdata.bin", 0);
    saveSection("bin/Debug/data.bin", 3);
   
    // Stop the timer
    auto end = std::chrono::high_resolution_clock::now();

    

    // Calculate the duration
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    printf("\n\n\nDecoding process took: %f seconds\n", duration.count() / 1000000.0);
    printf("Decoded %i PPC Instructions\n", instCount);
  
    // Splash texts
    printf("Hello, World!\n");
    printf("Say hi to the new galaxy note\n");

    // 19/02/2025
	printf("Trans rights!!!  @permdog99\n");
    printf("Live and learn  @ashrindy\n");
    printf("On dog  @.nover.\n");
	printf("Gotta Go Fast  @neoslyde\n");
}


