// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

// add headers that you want to pre-compile here
#include "framework.h"
#include <cstdint>

#ifdef DllTest_EXPORTS
#define DLL_API __declspec(dllexport)
#else
#define DLL_API __declspec(dllimport)
#endif

extern "C" {
    DLL_API int dllHack();
}

struct XenonState {
    uint64_t LR;
    uint64_t CTR;
    uint64_t MSR;
    uint64_t XER;
    uint32_t CR;
    uint64_t RR[32];
    double FR[32];
};


#endif //PCH_H
