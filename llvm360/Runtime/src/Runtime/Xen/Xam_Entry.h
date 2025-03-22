#pragma once
#include "XenUtils.h"
#include "Xam_Impl.h"
#include "../Runtime.h"


extern "C"
{
	DLL_API void XamLoaderTerminateTitle(XenonState* ctx, uint32_t lr)
	{
		XamLoaderTerminateTitle_X();
		return;
	}
}
