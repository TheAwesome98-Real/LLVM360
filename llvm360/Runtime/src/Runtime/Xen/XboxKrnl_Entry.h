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

	DLL_API void DbgPrint(XenonState* ctx, uint32_t lr)
	{
		DbgPrint_X();
		return;
	}

	DLL_API void HalReturnToFirmware(XenonState* ctx, uint32_t lr)
	{
		uint32_t rout = (uint32_t)ctx->gpr(3);
		HalReturnToFirmware_X(rout);
		return;
	}




	DLL_API void RtlEnterCriticalSection(XenonState* ctx, uint32_t lr)
	{
		X_LOG(X_INFO, "{RtlEnterCriticalSection}");
	}
	DLL_API void RtlLeaveCriticalSection(XenonState* ctx, uint32_t lr)
	{
		X_LOG(X_INFO, "{RtlLeaveCriticalSection} ");
	}
	DLL_API void XexCheckExecutablePrivilege(XenonState* ctx, uint32_t lr)
	{
		X_LOG(X_INFO, "{XexCheckExecutablePrivilege} ");
	}
	DLL_API void XGetAVPack(XenonState* ctx, uint32_t lr)
	{
		X_LOG(X_INFO, "{XGetAVPack} ");
	}
	DLL_API void ExGetXConfigSetting(XenonState* ctx, uint32_t lr)
	{
		X_LOG(X_INFO, "{ExGetXConfigSetting} ");
	}
	DLL_API void KeTlsAlloc(XenonState* ctx, uint32_t lr)
	{
		X_LOG(X_INFO, "{KeTlsAlloc} ");
	}
	DLL_API void KeTlsSetValue(XenonState* ctx, uint32_t lr)
	{
		X_LOG(X_INFO, "{KeTlsSetValue} ");
	}
	DLL_API void RtlInitUnicodeString(XenonState* ctx, uint32_t lr)
	{
		X_LOG(X_INFO, "{RtlInitUnicodeString} ");
	}
	DLL_API void RtlUnicodeStringToAnsiString(XenonState* ctx, uint32_t lr)
	{
		X_LOG(X_INFO, "{RtlUnicodeStringToAnsiString} ");
	}
	DLL_API void RtlFreeAnsiString(XenonState* ctx, uint32_t lr)
	{
		X_LOG(X_INFO, "{RtlFreeAnsiString} ");
	}
	DLL_API void RtlNtStatusToDosError(XenonState* ctx, uint32_t lr)
	{
		X_LOG(X_INFO, "{RtlNtStatusToDosError} ");
	}
	DLL_API void NtAllocateVirtualMemory(XenonState* ctx, uint32_t lr)
	{
		X_LOG(X_INFO, "{NtAllocateVirtualMemory} ");
	}
	DLL_API void NtFreeVirtualMemory(XenonState* ctx, uint32_t lr)
	{
		X_LOG(X_INFO, "{NtFreeVirtualMemory} ");
	}
	DLL_API void RtlCompareMemoryUlong(XenonState* ctx, uint32_t lr)
	{
		X_LOG(X_INFO, "{RtlCompareMemoryUlong} ");
	}
	DLL_API void XGetGameRegion(XenonState* ctx, uint32_t lr)
	{
		X_LOG(X_INFO, "{XGetGameRegion} ");
	}
	DLL_API void NtCreateEvent(XenonState* ctx, uint32_t lr)
	{
		X_LOG(X_INFO, "{NtCreateEvent} ");
	}
	DLL_API void XamShowMessageBoxUIEx(XenonState* ctx, uint32_t lr)
	{
		X_LOG(X_INFO, "{XamShowMessageBoxUIEx} ");
	}
	DLL_API void NtClose(XenonState* ctx, uint32_t lr)
	{
		X_LOG(X_INFO, "{NtClose} ");
	}
	DLL_API void RtlImageXexHeaderField(XenonState* ctx, uint32_t lr)
	{
		X_LOG(X_INFO, "{RtlImageXexHeaderField} ");
	}
	DLL_API void KeGetCurrentProcessType(XenonState* ctx, uint32_t lr)
	{
		X_LOG(X_INFO, "{KeGetCurrentProcessType} ");
	}
	DLL_API void NtQueryVirtualMemory(XenonState* ctx, uint32_t lr)
	{
		X_LOG(X_INFO, "{NtQueryVirtualMemory} ");
	}
	DLL_API void RtlInitializeCriticalSection(XenonState* ctx, uint32_t lr)
	{
		X_LOG(X_INFO, "{RtlInitializeCriticalSection} ");
	}
	DLL_API void KeTlsFree(XenonState* ctx, uint32_t lr)
	{
		X_LOG(X_INFO, "{KeTlsFree} ");
	}
	DLL_API void KeBugCheckEx(XenonState* ctx, uint32_t lr)
	{
		X_LOG(X_INFO, "{KeBugCheckEx} ");
	}
	DLL_API void NtWaitForSingleObjectEx(XenonState* ctx, uint32_t lr)
	{
		X_LOG(X_INFO, "{NtWaitForSingleObjectEx} ");
	}
	DLL_API void KeBugCheck(XenonState* ctx, uint32_t lr)
	{
		X_LOG(X_INFO, "{KeBugCheck} ");
	}
	DLL_API void KeTlsGetValue(XenonState* ctx, uint32_t lr)
	{
		X_LOG(X_INFO, "{KeTlsGetValue} ");
	}
	DLL_API void NtWriteFile(XenonState* ctx, uint32_t lr)
	{
		X_LOG(X_INFO, "{NtWriteFile} ");
	}
	DLL_API void RtlInitializeCriticalSectionAndSpinCount(XenonState* ctx, uint32_t lr)
	{
		X_LOG(X_INFO, "{RtlInitializeCriticalSectionAndSpinCount} ");
	}
	DLL_API void NtFlushBuffersFile(XenonState* ctx, uint32_t lr)
	{
		X_LOG(X_INFO, "{NtFlushBuffersFile} ");
	}
	DLL_API void NtDuplicateObject(XenonState* ctx, uint32_t lr)
	{
		X_LOG(X_INFO, "{NtDuplicateObject} ");
	}
	DLL_API void NtQueryDirectoryFile(XenonState* ctx, uint32_t lr)
	{
		X_LOG(X_INFO, "{NtQueryDirectoryFile} ");
	}
	DLL_API void NtOpenFile(XenonState* ctx, uint32_t lr)
	{
		X_LOG(X_INFO, "{NtOpenFile} ");
	}
	DLL_API void NtReadFileScatter(XenonState* ctx, uint32_t lr)
	{
		X_LOG(X_INFO, "{NtReadFileScatter} ");
	}
	DLL_API void NtSetInformationFile(XenonState* ctx, uint32_t lr)
	{
		X_LOG(X_INFO, "{NtSetInformationFile} ");
	}
	DLL_API void NtReadFile(XenonState* ctx, uint32_t lr)
	{
		X_LOG(X_INFO, "{NtReadFile} ");
	}
	DLL_API void NtQueryInformationFile(XenonState* ctx, uint32_t lr)
	{
		X_LOG(X_INFO, "{NtQueryInformationFile} ");
	}
	DLL_API void NtCreateFile(XenonState* ctx, uint32_t lr)
	{
		X_LOG(X_INFO, "{NtCreateFile} ");
	}
	DLL_API void NtQueryVolumeInformationFile(XenonState* ctx, uint32_t lr)
	{
		X_LOG(X_INFO, "{NtQueryVolumeInformationFile} ");
	}
	DLL_API void NtQueryFullAttributesFile(XenonState* ctx, uint32_t lr) 
	{
		X_LOG(X_INFO, "{NtQueryFullAttributesFile} ");
	}
	
}
