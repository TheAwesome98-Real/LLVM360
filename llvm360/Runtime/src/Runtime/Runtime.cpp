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

void XRuntime::init()
{
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
