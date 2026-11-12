#pragma once
#include <stdint.h>
namespace CpuLoadMath {
constexpr unsigned Unavailable = 101;
// Unsigned subtraction handles a single wrap of ESP-IDF's 32-bit us counter.
inline unsigned percent(uint32_t before, uint32_t after, uint64_t elapsedUs) {
    if (!elapsedUs || elapsedUs > UINT32_MAX) return Unavailable;
    uint64_t idleUs = uint32_t(after - before);
    if (idleUs > elapsedUs) idleUs = elapsedUs;
    return unsigned(((elapsedUs - idleUs) * 100 + elapsedUs / 2) / elapsedUs);
}
}
