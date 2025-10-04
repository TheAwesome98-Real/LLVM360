#include "Shared.h"
#include "Logger.h"





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

           
        

            address += 4; // always 4
            instCount++; // instruction count
        }
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
            if (!func->emission_done)
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


void LoadBinary()
{
	
}



PBinaryHandle* TranslateBinary(std::string path, bool useCache)
{
	
	PBinaryHandle* handle = new PBinaryHandle();
	handle->m_imagePath = path;
	handle->m_type = BIN_UNKNOWN;
	handle->m_ID = -1;
    

	if (useCache == false)
	{
        // load and decode instructions
        handle->LoadBinary();

        // recompile into dym lib
		handle->RecompileBinary();
		
        return handle;
    }

	LOG_ERROR("TranslateBinary", "Caching NYI");
    // the cache is very simple in practice, it's just a way to store already recompiled modules, 
    // it doesn't matter where they are located 
	// so all cached binaries will be located in ./cache/<hash>

	return handle;
}

int main(int argc, char* argv[])
{
   

    loadedXex = new XexImage(L"LLVMTest1.xex");
    loadedXex->LoadXex();
    g_irGen = new IRGenerator(loadedXex, mod, &builder);
    g_irGen->Initialize();
    
    



    // Splash texts
    printf("Hello, World!\n");
    printf("Say hi to the new galaxy note\n");

    // 19/02/2025
	printf("Trans rights!!!  @permdog99\n");
    printf("Live and learn  @ashrindy\n");
    printf("On dog  @.nover.\n");
	printf("Gotta Go Fast  @neoslyde\n");
}


