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

bool printINST = true;
bool printLLVMIR = true;

int main() {
  // Parse the XEX file. and use as file path a xex file in a folder relative to the exetubable
  
    XexImage *xex = new XexImage(L"MinecraftX360.xex");
  
    xex->LoadXex();

  
    for (size_t i = 0; i < xex->GetNumSections(); i++) {

    const Section *section = xex->GetSection(i);

    if (!section->CanExecute()) {
      
       continue;
    }

    printf("Processing section %s\n", section->GetName().c_str());

    // compute code range
    const auto baseAddress = xex->GetBaseAddress();
    const auto sectionBaseAddress = baseAddress + section->GetVirtualOffset();
    auto endAddress = baseAddress + section->GetVirtualOffset() + section->GetVirtualSize();

    // process all instructions in the code range
    auto address = sectionBaseAddress;
    bool forceBlockStart = false;
    int lastCount = -1;


    llvm::LLVMContext cxt;
    llvm::Module* mod = new llvm::Module("Xenon", cxt);
    llvm::IRBuilder<llvm::NoFolder> builder(cxt);

    InstructionDecoder decoder(section);
    IRGenerator* irGen = new IRGenerator(xex, mod, &builder);
    irGen->Initialize();

    printf("\n\n\n");
    uint32_t instCount = 0;
    auto start = std::chrono::high_resolution_clock::now();

    //uint32_t addrOverrider = 0x82060150;
    uint32_t addrOverrider = 0x82060004;
    endAddress = addrOverrider;
    while (address < endAddress) 
    {
      Instruction instruction;
      const auto instructionSize = decoder.GetInstructionAt(address, instruction);
      if (instructionSize == 0) 
      {
          printf("Failed to decode instruction at %08X\n", address);
          break;
      }

      // decode instruction bytes to Intruction struct for later use
      // use the instruction and emit LLVM IR code

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

      // print LLVM IR Output
      if (printLLVMIR) {
          bool result = irGen->EmitInstruction(instruction);

          if (!result) {
              __debugbreak();
              return 1;
          }
      }

      // move to the next instruction
      address += instructionSize;
      instCount++;
    }
    // Stop the timer
    auto end = std::chrono::high_resolution_clock::now();

    irGen->writeIRtoFile();

    // Calculate the duration
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    printf("\n\n\nDecoding process took: %f seconds\n", duration.count() / 1000000.0);
    printf("Decoded %i PPC Instructions\n", instCount);
  }

    // Splash texts
    printf("Hello, World!\n");
    printf("Say hi to the new galaxy note\n");
}
