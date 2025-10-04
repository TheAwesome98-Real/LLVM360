#include "Util.h"





//void unitTest(IRGenerator* gen)
//{
//    IRFunc* func = gen->getCreateFuncInMap(loadedXex->GetEntryAddress());
//    func->genBody();
//    gen->m_builder->SetInsertPoint(func->codeBlocks.at(func->start_address)->bb_Block);
//
//    //
//    // code
//    //
//
//    /*li r4, -1
//  li r5, 1
//  divdu r3, r4, r5
//  blr
//  #_ REGISTER_OUT r3 0xFFFFFFFFFFFFFFFF
//  #_ REGISTER_OUT r4 0xFFFFFFFFFFFFFFFF
//  #_ REGISTER_OUT r5 1*/
//    
//
//    // 0x0000000100000000
//    BUILD->CreateStore(i64Const(0x8000000000000000), func->getRegister("RR", 4));
//    //unit_li(func, gen, { 4, 0, (uint32_t)-1, });
//    unit_li(func, gen, { 5, 0, (uint32_t) - 1,});
//    unit_divdu(func, gen, { 3, 4, 5, });
//
//    unit_bclr(func, gen, {});
//    
//
//    //
//    // DUMP
//    //
//    printf("----IR DUMP----\n\n\n");
//    gen->writeIRtoFile();
//}



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

    //if (isUnitTesting)
	//{
	//	unitTest(g_irGen);
	//	return true;
	//}

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
            if (genLLVMIR && !func->emission_done)
            {
				g_irGen->initFuncBody(func);
				ret = func->EmitFunction();
                func->emission_done = true;
            }
        }
    }

    g_irGen->writeIRtoFile();

    return ret;
}



int main(int argc, char* argv[])
{
    std::vector<std::string> filePaths;

    if (argc < 2) 
    {
		LOG_FATAL("MAIN", "Usage: %s <path_to_xex_file>", argv[0]);
        return 1;
    }

    std::string txtFilePath = argv[1];
    std::ifstream file(txtFilePath);

    if (!file.is_open()) 
    {
		LOG_FATAL("MAIN", "Error: Could not open file %s", txtFilePath.c_str());
        return 1;
    }

    std::string line;
    while (std::getline(file, line)) 
    {
        if (!line.empty()) 
        {
			LOG_INFO("MAIN", "Found file path: %s", line.c_str());
			filePaths.push_back(line);
        }
    }
    file.close();






    loadedXex = new XexImage(L"LLVMTest1.xex");
    loadedXex->LoadXex();
    g_irGen = new IRGenerator(loadedXex, mod, &builder);
    g_irGen->Initialize();
    g_irGen->m_dbCallBack = dbCallBack;
    g_irGen->m_dumpIRConsole = dumpIRConsole;

    



    // Splash texts
    printf("Hello, World!\n");
    printf("Say hi to the new galaxy note\n");

    // 19/02/2025
	printf("Trans rights!!!  @permdog99\n");
    printf("Live and learn  @ashrindy\n");
    printf("On dog  @.nover.\n");
	printf("Gotta Go Fast  @neoslyde\n");
}


