#pragma once
#include "XKernel.h"

class XeThread : KeObj
{
private:
	std::thread actual_thread;
	uint32_t thread_id;
};