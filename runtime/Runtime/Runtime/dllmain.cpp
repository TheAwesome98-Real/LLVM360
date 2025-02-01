
#include "pch.h"
#include <cstdio>
#include <windows.h>
#include <iostream>

typedef XenonState**(__cdecl* getXCtxAddressFunc)();
static getXCtxAddressFunc g_initTLS = nullptr;
static HMODULE g_exeModule = NULL;


XenonState* mainThreadState;

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
            // Allocate 64 KB for the stack (adjust as needed)
            constexpr size_t stackSize = 64 * 1024;
            void* stackMemory = VirtualAlloc(nullptr, stackSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

            if (!stackMemory) 
            {
                std::cerr << "Failed to allocate stack memory\n";
                return false;
            }

     
            mainThreadState = new XenonState();
            mainThreadState->LR = 0x3242;

            // PPC R1 points to the end of the allocated stack (aligned to 16 bytes)
            uintptr_t alignedStack = reinterpret_cast<uintptr_t>(stackMemory) + stackSize;
            alignedStack &= ~0xF;  // align

            mainThreadState->RR[1] = alignedStack;


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

        printf("LR:  %i\n", mainThreadState->LR);

        for (size_t i = 0; i < 32; i++)
        {
            printf("R[%i]:  %i\n", i, mainThreadState->RR[i]);
        }
        break;
    }
    }
    
    return TRUE;
}