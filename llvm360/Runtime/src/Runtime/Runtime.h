#pragma once
#include <cstdint>
#include <cstdio>
#include <windows.h>
#include <iostream>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <string>
#include <mutex>
#include "../Graphics/ImGuiDebugger.h"

#define DLL_API __declspec(dllexport)

typedef struct {
    uint32_t xexAddress;
    void* funcPtr;
} X_Function;

struct Instruction {
    uint32_t address;
    uint32_t instrWord;
    std::string opcName;
    std::vector<uint32_t> ops;

    inline operator std::string() {
        return opcName;
    }
};


struct XenonState {
    uint64_t LR;
    uint64_t CTR;
    uint64_t MSR;
    uint64_t XER;
    uint32_t CR;
    uint64_t RR[32];
    double FR[32];
};

struct EXPMD_Section
{
    uint32_t dat_offset;
    uint32_t size;
    uint32_t flags;
    std::string sec_name;
    uint8_t* data;
};

struct EXPMD_Header
{
    uint32_t magic;
    uint32_t version;
    uint32_t flags;
	uint32_t baseAddress;
    uint32_t numSections;
    EXPMD_Section** sections;
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

std::mutex g_mutex;

extern "C"
{
	DLL_API void DebugCallBack(XenonState* ctx, uint32_t addr, char* name)
    {
		// update atomic values to callback the debugger and update
		/*g_mutex.lock();
		g_continue = false;
        g_dBUpdating = true;
		g_dBlist.push_back(addr);
		g_mutex.unlock();
        while (g_continue)
        {
            Sleep(1);
        }*/
        if(IsDebuggerPresent())
        {
            DebugBreak();
        }
    		
    }
}


class XRuntime
{
public:
	void init();
	void importMetadata(const char* path);
	void allocateSectionsData();

    typedef XenonState** (__cdecl* getXCtxAddressFunc)();
    getXCtxAddressFunc g_initTLS;
    HMODULE g_exeModule;
	EXPMD_Header* m_metadata;

    XenonState* mainThreadState;
    uint8_t* mainStack;

    X_Function* exportedArray;
    int* exportedCount;
    bool debuggerEnabled;

	//Graphics* m_graphics;
    static XRuntime* g_runtime;
    static void initGlobal();
};

