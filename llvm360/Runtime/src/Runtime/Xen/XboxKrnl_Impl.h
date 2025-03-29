#pragma once
#include "XenUtils.h"

void RtlInitAnsiString_X(X_ANSI_STRING* ansiStr, char* str)
{
	uint16_t length = strlen(str);
	ansiStr->pointer = (uint32_t)str;
	ansiStr->length = length;
	ansiStr->maximum_length = length + 1;
	X_LOG(X_INFO, "{RtlInitAnsiString_X} str: %s\n", str);
	return;
}

void DbgPrint_X()
{
	X_LOG(X_WARNING, "{DbgPrint_X} not implemented");
	return;
}

void HalReturnToFirmware_X(uint32_t routine)
{
	if (routine != 1) DebugBreak();
	X_LOG(X_INFO, "{HalReturnToFirmware_X} requested exit");
	exit(0);
	return;
}

uint32_t NtAllocateVirtualMemory_X(uint32_t* basePtr, uint32_t* sizePtr, uint32_t allocType, uint32_t protect)
{
	const auto base = (uint32_t)*basePtr;
	const auto size = (uint32_t)*sizePtr;

	/*/ invalid type
	if (allocType & XMEM_RESERVE && (base != NULL))
	{
		GLog.Err("VMEM: Trying to reserve predefined memory region. Not supported.");
		*basePtr = 0;
		return  X_STATUS_NOT_IMPLEMENTED;
	}*/

	// determine aligned size
	uint32_t pageAlignment = 4096;
	//if (allocType & XMEM_LARGE_PAGES) pageAlignment = 64 << 10;
	//else if (allocType & XMEM_16MB_PAGES) pageAlignment = 16 << 20;

	// align size
	uint32_t alignedSize = (size + (pageAlignment - 1)) & ~(pageAlignment - 1);
	if (alignedSize != size)
		printf("VMEM: Aligned memory allocation %08X->%08X.\n", size, alignedSize);

	// allocate memory from system
	//void* allocated = XRuntime::g_runtime->m_memory.Allocate(alignedSize, base);
	void* allocated = 0;
	if (!allocated)
	{
		printf("VMEM: Allocation failed.");
		*basePtr = 0;
		return  1;
	}

	// save allocated memory & size
	*basePtr = (uint32_t)allocated;
	*sizePtr = (uint32_t)alignedSize;

	// allocation was OK
	return 0;
}