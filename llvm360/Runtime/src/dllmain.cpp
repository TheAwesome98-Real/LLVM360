#include "pch.h"


BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) 
{
    switch (fdwReason) {
    case DLL_PROCESS_ATTACH:

        XRuntime::initGlobal();
        XRuntime::g_runtime->init();
        ImGuiDebugger::initStatic();
        

        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        
        
        break;
    case DLL_PROCESS_DETACH:
    {
        
        printf("---- Main Thread ----");
        printf("LR:  %016llX\n", XRuntime::g_runtime->mainThreadState->LR);
        printf("CR:  %08llX\n", XRuntime::g_runtime->mainThreadState->CR);
        printf("XER: %08llX\n", XRuntime::g_runtime->mainThreadState->XER);

        printf("\n--- RR ---\n");
        for (size_t i = 0; i < 32; i++)
        {
            printf("R[%i]:  %016llX\n", i, XRuntime::g_runtime->mainThreadState->RR[i]);
        }
        printf("\n--- FR ---\n");
        for (size_t i = 0; i < 32; i++)
        {
            printf("FR[%i]:  %016llX\n", i, XRuntime::g_runtime->mainThreadState->FR[i]);
        }
        break;
    }
    }
    
    return TRUE;
}