#pragma once
#include "Runtime.h"
#include "../Graphics/DX12Manager.h"
#include "../Graphics/ImGuiDebugger.h"

bool d3dinit = false;

extern "C"
{
	DLL_API int dllHack()
	{
		if (!d3dinit)
		{
			XRuntime::g_runtime->debuggerEnabled = true;
			DirectX12Manager& inst = DirectX12Manager::getInstance();
			if(XRuntime::g_runtime->debuggerEnabled)
			{


				//create a new thread and call ImGuiDebugger::getInstance().initialize(nullptr);
				ImGuiDebugger::g_debugger->initialize();
				std::thread imguiThread([]() {
					printf("Starting ImGui Thread\n");

						ImGuiDebugger::g_debugger->render(); 
					
					});
				std::this_thread::sleep_for(std::chrono::seconds(1));

				imguiThread.detach();

			}

			d3dinit = true;
		}
		
		return 1;
	}

	DLL_API void RtlInitAnsiString()
	{
		printf("\n{RtlInitAnsiString}  Hello, Xbox 360!\n\n");
		return;
	}

	typedef void(__cdecl* x_func)(XenonState*, uint32_t);

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
}
