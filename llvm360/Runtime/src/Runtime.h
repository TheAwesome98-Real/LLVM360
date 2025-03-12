#pragma once
#include <cstdint>
#include <cstdio>
#include <windows.h>
#include <iostream>

#define DLL_API __declspec(dllexport)

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
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <string>

struct Instruction {
    uint32_t address;
    uint32_t instrWord;
    std::string opcName;
    std::vector<uint32_t> ops;

    inline operator std::string() {
        return opcName;
    }
};

void deserialize(const std::string& filename, std::unordered_map<uint32_t, Instruction>& map) 
{
    std::ifstream ifs(filename, std::ios::binary);
    if (!ifs.is_open()) {
        printf("Error opening file for reading\n");
        return;
    }

    while (ifs.peek() != EOF) 
    {
        Instruction inst;

        // address and instrWord
        ifs.read(reinterpret_cast<char*>(&inst.address), sizeof(inst.address));
        ifs.read(reinterpret_cast<char*>(&inst.instrWord), sizeof(inst.instrWord));

        // opcName
        uint32_t nameLength;
        ifs.read(reinterpret_cast<char*>(&nameLength), sizeof(nameLength));
        inst.opcName.resize(nameLength);
        ifs.read(&inst.opcName[0], nameLength + 1); 

        // ops vector
        uint32_t opsSize;
        ifs.read(reinterpret_cast<char*>(&opsSize), sizeof(opsSize));
        inst.ops.resize(opsSize);
        if (opsSize > 0) {
            ifs.read(reinterpret_cast<char*>(inst.ops.data()), opsSize * sizeof(uint32_t));
        }

        map[inst.address] = inst;
    }

    ifs.close();
    printf("Deserialization completed successfully.\n");
}



extern "C"
{
	DLL_API void DebugCallBack(XenonState* ctx, uint32_t addr, char* name)
    {
        printf("DB");
        return;
    }
}


class XRuntime
{
public:
	void init();


    typedef XenonState** (__cdecl* getXCtxAddressFunc)();
    getXCtxAddressFunc g_initTLS;
    HMODULE g_exeModule;


    XenonState* mainThreadState;
    uint8_t* mainStack;

    std::unordered_map<uint32_t, Instruction> dbInstrMap;
    X_Function* exportedArray;
    int* exportedCount;
    bool debuggerEnabled;

    static XRuntime* g_runtime;
    static void initGlobal();
};

