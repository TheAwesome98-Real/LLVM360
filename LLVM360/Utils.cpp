#include "Utils.h"

uint32_t Swap32(uint32_t val) {
    return ((((val) & 0xff000000) >> 24) |
        (((val) & 0x00ff0000) >> 8) |
        (((val) & 0x0000ff00) << 8) |
        (((val) & 0x000000ff) << 24));
}