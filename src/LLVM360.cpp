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
#include <IR/IRFunc.h>


// Debug
bool printINST = true;
bool printFile = false;  // Set this flag to true or false based on your preference
bool genLLVMIR = true;
bool isUnitTesting = false;
bool doOverride = false; // if it should override the endAddress to debug
uint32_t overAddr = 0x82060150;

// Benchmark / static analysis
uint32_t instCount = 0;

// Naive+ stuff
XexImage* loadedXex;
IRGenerator* g_irGen;
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
	IRFunc* testFunc = gen->getCreateFuncInMap(loadedXex->GetEntryAddress());
    testFunc->genBody();
    gen->m_builder->SetInsertPoint(testFunc->codeBlocks.at(testFunc->start_address)->bb_Block);
    //unit_mfspr(testFunc, gen);
    //unit_stfd(gen);
    //unit_stwu(gen);
    //unit_lwz(gen);
	unit_li(testFunc, gen);
    unit_stw(testFunc, gen);
    unit_lwz(testFunc, gen);
    //unit_stwu(testFunc, gen);
    
    
	//unit_cmpw(testFunc, gen);

	gen->m_builder->CreateRetVoid();
    //
    // DUMP
    //
    printf("----IR DUMP----\n\n\n");
    llvm::raw_fd_ostream& out = llvm::outs();
    gen->m_module->print(out, nullptr);
    gen->writeIRtoFile();
}


inline uint32_t recursiveNopCheck(uint32_t address)
{
	const char* name = g_irGen->instrsList.at(address).opcName.c_str();
	if (strcmp(name, "nop") == 0)
	{
		recursiveNopCheck(address + 4);
	}
	else
	{
		return address;
	}
}

bool pass_Flow()
{
    bool ret = true;

    for (size_t i = 0; i < loadedXex->GetNumSections(); i++)
    {
        const Section* section = loadedXex->GetSection(i);

        if (!section->CanExecute())
        {
            continue;
        }
        printf("\nFlow for section %s\n\n", section->GetName().c_str());
        // compute code range
        const auto baseAddress = loadedXex->GetBaseAddress();
        const auto sectionBaseAddress = baseAddress + section->GetVirtualOffset();
        auto endAddress = baseAddress + section->GetVirtualOffset() + section->GetVirtualSize();
        auto address = sectionBaseAddress;

        // create the function for the first function
        
		IRFunc* first = g_irGen->getCreateFuncInMap(sectionBaseAddress);
        IRFunc* prevFunc = nullptr;
		IRFunc* currentFunc = first;
		

        while (address < endAddress)
        {
			const char* name = g_irGen->instrsList.at(address).opcName.c_str();
            if (strcmp(name, "bclr") == 0)
            {
				printf("Found end of function bounds at with BLR: %08X\n", address);
				currentFunc->end_address = address;
				prevFunc = currentFunc;
				if (address + 4 != endAddress)
				{
                    currentFunc = g_irGen->getCreateFuncInMap(recursiveNopCheck(address + 4));
				}
				    
            }

            /*/ todo, more Heuristic because tail calls
            ..
            ..
            B
            NOP

            also:
            
            ..
            ..
            B
            MFSPR .., LR
            
            the edge cases i can just let it inline the rest, cause they should not be big ass functions
            */
           
            address += 4; // always 4
        }
    }

    

    return ret;
}


bool pass_Decode()
{

    bool ret = true;

    // Open the output file if printFile is true
    std::ofstream outFile;
    if (printFile) {
        outFile.open("instructions_output.txt");
        if (!outFile.is_open()) {
            printf("Failed to open the file for writing instructions.\n");
            ret = false;
            return ret;
        }
    }

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
            g_irGen->instrsList.try_emplace(address, instruction);

            if (printINST) {
                // print raw PPC decoded instruction + operands
                std::ostringstream oss;
                oss << std::hex << std::uppercase << std::setfill('0');
                oss << std::setw(8) << address << ":   " << instruction.opcName;

                for (size_t i = 0; i < instruction.ops.size(); ++i) {
                    oss << " " << std::setw(2) << static_cast<int>(instruction.ops.at(i));
                }

                std::string output = oss.str();
                if (printFile) {
                    // Write output to file if printFile is true
                    outFile << output << std::endl;
                }
                else {
                    // Print output to the console
                    printf("%s\n", output.c_str());
                }
            }

            address += 4; // always 4
            instCount++; // instruction count
        }
    }

    // Close the output file if it was opened
    if (printFile) {
        outFile.close();
    }

    return ret;
}

bool pass_Emit()
{

    if (isUnitTesting)
	{
		unitTest(g_irGen);
		return true;
	}

    bool ret = true;

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

        
        for (const auto& pair : g_irGen->m_function_map) 
        {
            IRFunc* func = pair.second;
            if (genLLVMIR)
            {
				g_irGen->initFuncBody(func);
				ret = func->EmitFunction();
            }
        }
    }

    g_irGen->writeIRtoFile();

    return ret;
}



int main() 
{
    loadedXex = new XexImage(L"LLVMTest1.xex");
    loadedXex->LoadXex();
    g_irGen = new IRGenerator(loadedXex, mod, &builder);
    g_irGen->Initialize();

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

    
    if (!pass_Decode())
    {
        printf("something went wrong - Pass: DECODE\n");
        return -1;
    }

    if (!pass_Flow())
    {
        printf("something went wrong - Pass: FLOW\n");
        return -1;
    }

    // third recomp pass: Emit IR code
    if (!pass_Emit())
    {
        printf("something went wrong - Pass: EMIT\n");
        return -1;
    }

    // sections
    //saveSection("bin/Debug/rdata.bin", 0);
    //saveSection("bin/Debug/data.bin", 3);
   
    // Stop the timer
    auto end = std::chrono::high_resolution_clock::now();

    
	g_irGen->writeIRtoFile();
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


