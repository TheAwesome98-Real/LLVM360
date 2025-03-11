#pragma once
#include "Runtime.h"


#define DLL_API __declspec(dllexport)


extern "C"
{
	DLL_API int dllHack()
	{
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
		return;
	}
}
