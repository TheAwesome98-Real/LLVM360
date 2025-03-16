#pragma once
#include "XenUtils.h"
#include "XboxKrnl_Impl.h"
#include "../Runtime.h"


extern "C"
{
	DLL_API void RtlInitAnsiString(XenonState* ctx, uint32_t lr)
	{
		X_ANSI_STRING* ansiStr = (X_ANSI_STRING*)ctx->gpr(3);
		char* str = (char*)ctx->gpr(4);
		RtlInitAnsiString_X(ansiStr, str);
		return;
	}
}
