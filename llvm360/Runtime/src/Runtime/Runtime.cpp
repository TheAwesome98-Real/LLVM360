#include "Runtime.h"
#include "../pch.h"


void* AllocateBelow4GB(HANDLE process, SIZE_T size) {
    const uintptr_t startAddress = 0x10000000; // Start at 256MB
    const uintptr_t maxAddress = 0xFFFFFFFF; // 4GB limit
    const SIZE_T stepSize = 0x10000;    // 64KB step

    for (uintptr_t addr = startAddress; addr < maxAddress; addr += stepSize) {
        void* ptr = VirtualAllocEx(process, (void*)addr, size, MEM_RESERVE | MEM_COMMIT | MEM_TOP_DOWN, PAGE_READWRITE);
        if (ptr != nullptr) {
            std::cout << "Allocated at: " << ptr << std::endl;
            return ptr; // Success
        }
    }

    std::cerr << "Failed to allocate below 4GB" << std::endl;
    return nullptr;
}

void initMainThread(XRuntime* run)
{

    if (run->g_initTLS)
    {
        // Allocate 512 kb for the main thread stack 
        constexpr size_t stackSize = 512 * 1024;



        void* ptr = AllocateBelow4GB(GetCurrentProcess(), stackSize);
        run->mainStack = (uint8_t*)ptr;

        MEMORY_BASIC_INFORMATION mbi;
        VirtualQuery(ptr, &mbi, sizeof(mbi));
        if ((uintptr_t)mbi.BaseAddress < 0x100000000)
        {
            printf("Allocation is in 32-bit addressable space\n");
        }
        else
        {
            printf("PANIC!! Stack allocated above 32bit range shutting down\n");
            return;
        }



        if (!run->mainStack)
        {
            std::cerr << "Failed to allocate stack memory\n";
            return;
        }


        run->mainThreadState = new XenonState();
        run->mainThreadState->LR = 0x3242;

        // PPC R1 points to the end of the allocated stack (aligned to 16 bytes)
        uintptr_t alignedStack = reinterpret_cast<uintptr_t>(run->mainStack) + stackSize;
        alignedStack &= ~0xF;  // align

        run->mainThreadState->RR[1] = alignedStack;


        // TODO add a check for the low 32 bits

        *run->g_initTLS() = run->mainThreadState;
        XenonState* state22 = *run->g_initTLS();
        printf("MainThread allocation done\n");
    }
}

#define MD_VER 4

void XRuntime::importMetadata(const char* path)
{
	std::ifstream ifs(path, std::ios::binary);
	if (!ifs.is_open()) {
		printf("Error opening file for reading\n");
		return;
	}

	EXPMD_Header header;
	ifs.read(reinterpret_cast<char*>(&header.magic), sizeof(uint32_t));
	ifs.read(reinterpret_cast<char*>(&header.version), sizeof(uint32_t));
	ifs.read(reinterpret_cast<char*>(&header.flags), sizeof(uint32_t));
	ifs.read(reinterpret_cast<char*>(&header.baseAddress), sizeof(uint32_t));
	ifs.read(reinterpret_cast<char*>(&header.numSections), sizeof(uint32_t));
	header.sections = new EXPMD_Section * [header.numSections];

	if (header.magic != 0x54535338)
	{
		printf("Invalid metadata file\n");
		return;
	}

    if (header.version != MD_VER)
    {
        printf("Invalid metadata file\n");
        return;
    }

    for (size_t i = 0; i < header.numSections; i++)
    {
        EXPMD_Section* sec = new EXPMD_Section;
        EXPMD_IMPVar impVar = *header.imp_Vars[i];
        uint32_t nameLength = impVar.name.size();
        binFile.write(reinterpret_cast<const char*>(&nameLength), sizeof(uint32_t));
        binFile.write(impVar.name.c_str(), impVar.name.size());

        binFile.write(reinterpret_cast<const char*>(&impVar.addr), sizeof(uint32_t));
    }

    for (size_t i = 0; i < header.numSections; i++)
    {
        EXPMD_Section* sec = new EXPMD_Section;
		ifs.read(reinterpret_cast<char*>(&sec->dat_offset), sizeof(uint32_t));
		ifs.read(reinterpret_cast<char*>(&sec->size), sizeof(uint32_t));
		ifs.read(reinterpret_cast<char*>(&sec->flags), sizeof(uint32_t));
		uint32_t nameLength;
		ifs.read(reinterpret_cast<char*>(&nameLength), sizeof(uint32_t));
		sec->sec_name.resize(nameLength);
		ifs.read(&sec->sec_name[0], nameLength);
		sec->data = new uint8_t[sec->size];
		ifs.read(reinterpret_cast<char*>(sec->data), sec->size);
		header.sections[i] = sec;
    }

	// copy the data in the &header to m_metadata otherwise it will be stripped
	m_metadata = new EXPMD_Header;
	*m_metadata = header;
}
void XRuntime::allocateSectionsData()
{
	if (!m_metadata)
	{
		printf("No metadata found\n");
		return;
	}

    size_t allocationSize = 0;

    for (size_t i = 0; i < m_metadata->numSections; i++)
    {
        if (i == 0)
        {
            allocationSize = m_metadata->sections[i]->dat_offset;
            continue;
        }

        if(m_metadata->sections[i]->dat_offset > m_metadata->sections[i - 1]->dat_offset)
        {
            allocationSize = m_metadata->sections[i]->dat_offset;
        }
    }

    void* baseAddr = VirtualAlloc(reinterpret_cast<void*>(m_metadata->baseAddress), allocationSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	if (!baseAddr || (uint32_t)baseAddr != m_metadata->baseAddress)
	{
		printf("Failed to reserve memory\n");
		return;
	}

	for (size_t i = 0; i < m_metadata->numSections; i++)
	{
		EXPMD_Section* sec = m_metadata->sections[i];
        memcpy((void*)((uint32_t)baseAddr + sec->dat_offset), sec->data, sec->size);
        printf("Allocated section memory at %08llX\n", (uint32_t)baseAddr + sec->dat_offset);
	}
}

void XRuntime::init()
{
    importMetadata("MD.tss");
    allocateSectionsData();

    g_exeModule = GetModuleHandleW(NULL);
    if (g_exeModule)
    {
        g_initTLS = (getXCtxAddressFunc)GetProcAddress(g_exeModule, "getXCtxAddress");
        exportedArray = (X_Function*)GetProcAddress(g_exeModule, "X_FunctionArray");
        exportedCount = (int*)GetProcAddress(g_exeModule, "X_FunctionArrayCount");
    }

    initMainThread(this);
    
	//m_graphics = new Graphics();
    //m_graphics->initGraphics();
}


XRuntime* XRuntime::g_runtime;
void XRuntime::initGlobal()
{
    g_runtime = new XRuntime();
}
