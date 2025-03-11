#pragma once
#include <cstdint>
#include <cstdio>
#include <windows.h>
#include <iostream>

typedef struct {
    uint32_t xexAddress;
    void* funcPtr;
} X_Function;


struct XenonState {
    uint64_t LR;
    uint64_t CTR;
    uint64_t MSR;
    uint64_t XER;
    uint32_t CR;
    uint64_t RR[32];
    double FR[32];
};

class XRuntime
{
public:
	void init();


    typedef XenonState** (__cdecl* getXCtxAddressFunc)();
    getXCtxAddressFunc g_initTLS;
    HMODULE g_exeModule;


    XenonState* mainThreadState;
    uint8_t* mainStack;

    X_Function* exportedArray;
    int* exportedCount;

    static XRuntime* g_runtime;
    static void initGlobal();
};

