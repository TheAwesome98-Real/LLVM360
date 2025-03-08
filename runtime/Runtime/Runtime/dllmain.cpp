
#include "pch.h"
#include <cstdio>
#include <windows.h>
#include <iostream>

typedef XenonState**(__cdecl* getXCtxAddressFunc)();
static getXCtxAddressFunc g_initTLS = nullptr;
static HMODULE g_exeModule = NULL;


XenonState* mainThreadState;

uint8_t* mainStack;

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

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) 
{
    switch (fdwReason) {
    case DLL_PROCESS_ATTACH:

        // get the function pointer to the tls value initializer
        g_exeModule = GetModuleHandleW(NULL);
        if (g_exeModule) 
        {
            g_initTLS = (getXCtxAddressFunc)GetProcAddress(g_exeModule, "getXCtxAddress");
        }

        if (g_initTLS) 
        {
            // Allocate 512 kb for the main thread stack 
            constexpr size_t stackSize = 512 * 1024;

            

            void* ptr = AllocateBelow4GB(GetCurrentProcess(), stackSize);
            mainStack = (uint8_t*)ptr;

            MEMORY_BASIC_INFORMATION mbi;
            VirtualQuery(ptr, &mbi, sizeof(mbi));
            if ((uintptr_t)mbi.BaseAddress < 0x100000000)
            {
                printf("Allocation is in 32-bit addressable space\n");
            }
            else
            {
                printf("PANIC!! Stack allocated above 32bit range shutting down\n");
                return -1;
            }
                


            if (!mainStack)
            {
                std::cerr << "Failed to allocate stack memory\n";
                return false;
            }

     
            mainThreadState = new XenonState();
            mainThreadState->LR = 0x3242;

            // PPC R1 points to the end of the allocated stack (aligned to 16 bytes)
            uintptr_t alignedStack = reinterpret_cast<uintptr_t>(mainStack) + stackSize;
            alignedStack &= ~0xF;  // align

            mainThreadState->RR[1] = alignedStack;


            // TODO add a check for the low 32 bits

            *g_initTLS() = mainThreadState;
            XenonState* state22 = *g_initTLS();
            printf("MainThread allocation done\n");
        }
        

        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        // Optional: Cleanup logic here

        
        break;
    case DLL_PROCESS_DETACH:
    {
        
        printf("LR:  %016llX\n", mainThreadState->LR);
        printf("CR:  %08llX\n", mainThreadState->CR);
        printf("XER: %08llX\n", mainThreadState->XER);

        printf("\n--- RR ---\n");
        for (size_t i = 0; i < 32; i++)
        {
            printf("R[%i]:  %016llX\n", i, mainThreadState->RR[i]);
        }
        printf("\n--- FR ---\n");
        for (size_t i = 0; i < 32; i++)
        {
            printf("FR[%i]:  %016llX\n", i, mainThreadState->FR[i]);
        }
        break;
    }
    }
    
    return TRUE;
}