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