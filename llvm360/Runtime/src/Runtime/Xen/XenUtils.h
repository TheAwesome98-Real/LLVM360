#pragma once
#include "../Runtime.h"
#include "../..//Graphics/DX12Manager.h"
#include "../../Graphics/ImGuiDebugger.h"
#include <cstdint>
#include <type_traits>
#include <bit>
#include <string>
#include <cassert>
#include <wrl.h>
typedef void(__cdecl* x_func)(XenonState*, uint32_t);
#define DLL_API __declspec(dllexport)
bool d3dinit = false;

// optimize?
inline uint16_t ByteSwap16(uint16_t value) 
{
    return (value << 8) | (value >> 8);
}

// optimize?
inline uint32_t ByteSwap32(uint32_t value) 
{
    return ((value & 0x000000FF) << 24) |
        ((value & 0x0000FF00) << 8) |
        ((value & 0x00FF0000) >> 8) |
        ((value & 0xFF000000) >> 24);
}

// optimize?
inline uint64_t ByteSwap64(uint64_t value) 
{
    return ((value & 0x00000000000000FFULL) << 56) |
        ((value & 0x000000000000FF00ULL) << 40) |
        ((value & 0x0000000000FF0000ULL) << 24) |
        ((value & 0x00000000FF000000ULL) << 8) |
        ((value & 0x000000FF00000000ULL) >> 8) |
        ((value & 0x0000FF0000000000ULL) >> 24) |
        ((value & 0x00FF000000000000ULL) >> 40) |
        ((value & 0xFF00000000000000ULL) >> 56);
}

// https://github.com/rexdex/recompiler/blob/7cd1d5a33d6c02a13f972c6564550ea816fc8b5b/dev/src/xenon_launcher/xenonLib.h#L416
// https://github.com/xenia-canary/xenia-canary/blob/8926bdcdd695bcf7f8f95938f0e44c59fbf9be14/src/xenia/base/byte_order.h#L84
template<typename T>
struct BeField
{
    T value;

    BeField() : value(0)
    {
    }

    BeField(const T v)
    {
        set(v);
    }

    void set(const T v)
    {
        value = swap(v);
    }

    T get() const
    {
        return swap(value);
    }

    BeField& operator+(T v)
    {
        set(value + v);
        return *this;
    }

    BeField& operator=(T v)
    {
        set(v);
        return *this;
    }

    // https://github.com/xenia-canary/xenia-canary/blob/8926bdcdd695bcf7f8f95938f0e44c59fbf9be14/src/xenia/base/byte_order.h#L62
    static T swap(T value)
    {
        if (std::is_same_v<T, uint16_t>)
        {
            const uint16_t swapped = ByteSwap16(*reinterpret_cast<uint16_t*>(&value));
            return *reinterpret_cast<const T*>(&swapped);
        }
        else if (std::is_same_v<T, uint32_t>)
        {
            const uint32_t swapped = ByteSwap32(*reinterpret_cast<uint32_t*>(&value));
            return *reinterpret_cast<const T*>(&swapped);
        }
        else if (std::is_same_v<T, uint64_t>)
        {
            const uint64_t swapped = ByteSwap64(*reinterpret_cast<uint64_t*>(&value));
            return *reinterpret_cast<const T*>(&swapped);
        }
        else if (std::is_same_v<T, double>)
        {
            const uint64_t swapped = ByteSwap64(*reinterpret_cast<uint64_t*>(&value));
            return *reinterpret_cast<const T*>(&swapped);
        }
        else if (std::is_same_v<T, float>)
        {
            const uint32_t swapped = ByteSwap32(*reinterpret_cast<uint32_t*>(&value));
            return *reinterpret_cast<const T*>(&swapped);
        }
        else
        {
			printf("SWAP: Unsupported type\n");
            static_assert(std::is_integral_v<T>, "Unsupported type");
            DebugBreak();
            exit(1);
        }
    }
};

struct X_ANSI_STRING 
{
	BeField<uint16_t> length;
	BeField<uint16_t> maximum_length;
	BeField<uint32_t> pointer;
};

extern "C"
{
	DLL_API int dllHack()
	{
		if (!d3dinit)
		{
			XRuntime::g_runtime->debuggerEnabled = true;
			DirectX12Manager& inst = DirectX12Manager::getInstance();
			if (XRuntime::g_runtime->debuggerEnabled)
			{


				/*/create a new thread and call ImGuiDebugger::getInstance().initialize(nullptr);
				ImGuiDebugger::g_debugger->initialize();
				std::thread imguiThread([]() {
					printf("Starting ImGui Thread\n");

						ImGuiDebugger::g_debugger->render();

					});
				std::this_thread::sleep_for(std::chrono::seconds(1));

				imguiThread.detach();*/

			}

			d3dinit = true;
		}

		return 1;
	}
	
	DLL_API void HandleBcctrl(XenonState* ctx, uint32_t lr)
	{
		for (uint32_t i = 0; i < *XRuntime::g_runtime->exportedCount; i++)
		{
			if (ctx->CTR == XRuntime::g_runtime->exportedArray[i].xexAddress)
			{
				(*(x_func)XRuntime::g_runtime->exportedArray[i].funcPtr)(ctx, lr);
				return;
			}
		}
		printf("-------- {HandleBcctrl} ERROR: NO FUNCTION AT: %u \n", ctx->CTR);
		return;
	}

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
        if (IsDebuggerPresent() && true)
        {
            DebugBreak();
        }

    }
}
