#pragma once
#include "XenUtils.h"
#include "XboxKrnl_Impl.h"
#include "../Runtime.h"


extern "C"
{
	DLL_API void RtlInitAnsiString(XenonState* ctx, uint32_t lr)
	{

		RtlInitAnsiString_X();
		return;
	}
}
