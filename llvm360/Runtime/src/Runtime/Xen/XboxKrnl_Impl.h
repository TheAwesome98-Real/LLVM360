#pragma once
#include "XenUtils.h"

void RtlInitAnsiString_X(X_ANSI_STRING* ansiStr, char* str)
{
	uint16_t length = strlen(str);
	ansiStr->pointer = (uint32_t)str;
	ansiStr->length = length;
	ansiStr->maximum_length = length + 1;
	printf("{RtlInitAnsiString_X} str: %s\n", str);
	return;
}