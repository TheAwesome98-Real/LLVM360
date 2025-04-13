#pragma once
#include "XenUtils.h"
#include "../MemoryManager.h"
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
	(X_INFO, "{HalReturnToFirmware_X} requested exit");
	exit(0);
	return;
}

X_STATUS NtAllocateVirtualMemory_X(uint32_t* basePtr, uint32_t* sizePtr, uint32_t allocType, uint32_t protect)
{
	const auto base = (uint32_t)*basePtr;
	const auto size = BeField<uint32_t>((uint32_t)*sizePtr).value;
	if (allocType & XMEM_RESERVE && (base != NULL))
	{
		X_LOG(X_ERROR, "MEM - TRIED TO ALLOCATE AT SPECIFIC ADDRESS");
		*basePtr = 0;
		return  X_STATUS_NOT_IMPLEMENTED;
	}

	// align Page
	uint32_t pageAlignment = 4096;
	if (allocType & XMEM_LARGE_PAGES) 
	{
		pageAlignment = 0x10000;
	}
	else if (allocType & XMEM_16MB_PAGES) 
	{
		pageAlignment = 0x1000000;
	}

	// align size
	uint32_t alignedSize = (size + (pageAlignment - 1)) & ~(pageAlignment - 1);
	if (alignedSize != size)
	{
		X_LOG(X_WARNING, "MEM - aligned to %08X->%08X.1", size, alignedSize);
	}

	void* allocated = 0;
	uint32_t allocTypeFixed = allocType;
	if (!(allocTypeFixed & XMEM_COMMIT))
	{
		allocated = XRuntime::g_runtime->m_memory->allocate((size_t)base, alignedSize, 0, protect);
		*basePtr = BeField<uint32_t>(XRuntime::g_runtime->m_memory->makeHostGuest(allocated)).value;
		if (!allocated)
		{
			X_LOG(X_ERROR, "MEM -  Allocation failed");
			*basePtr = 0;
			return  X_STATUS_NO_MEMORY;
		}
		X_LOG(X_INFO, "MEM - Allocated %04X bytes", alignedSize);
	}
	else
	{
		*basePtr = base;
		X_LOG(X_ERROR, "MEM -  Unknown allocType flag");
	}

	// save allocated memory & size
	
	*sizePtr = BeField<uint32_t>((uint32_t)alignedSize).value;

	// allocation was OK
	return X_STATUS_SUCCESS;
}

// STUB
uint32_t thisProcessType = X_PROCTYPE_USER;

uint32_t KeGetCurrentProcessType_X()
{
	return thisProcessType;
}



void RtlInitializeCriticalSection_X(XCRITICAL_SECTION* xctrs)
{
	//auto* cs = GPlatform.GetKernel().CreateCriticalSection();
	//auto* thread = xenon::KernelThread::GetCurrentThread();
	xctrs->LockCount = 1;
	xctrs->OwningThread = 2;
	xctrs->RecursionCount = 3;
	xctrs->Synchronization.RawEvent[0] = 4;
	xctrs->Synchronization.RawEvent[1] = 5;
	xctrs->Synchronization.RawEvent[2] = 6;
	xctrs->Synchronization.RawEvent[3] = 7;


	/*xctrs->Synchronization.RawEvent[0] = 0xDAAD0FF0;
	xctrs->Synchronization.RawEvent[1] = cs->GetHandle();
	xctrs->OwningThread = thread ? thread->GetHandle() : 0;
	xctrs->RecursionCount = 0;
	xctrs->LockCount = 0;*/

	/*xctrs->Synchronization.RawEvent[0] = 0;
	xctrs->LockCount = -1;
	xctrs->RecursionCount = 0;
	xctrs->OwningThread = GetCurrentThreadId();
	const uint32_t kernelMarker = xctrs->Synchronization.RawEvent[0];*/
}

X_STATUS RtlEnterCriticalSection_X(XCRITICAL_SECTION* xctrs)
{

	//uint32_t spin_count = xctrs->header.absolute * 256;
	//xctrs->recursion_count = xctrs->recursion_count + 1;
	//InterlockedIncrement(reinterpret_cast<volatile long*>(&xctrs->lock_count));

	X_LOG(X_ERROR, "RtlEnterCriticalSection_X, NOT IMPLEMENTED");
	return 0;
}