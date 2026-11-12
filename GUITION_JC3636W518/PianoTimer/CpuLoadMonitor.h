#pragma once
#include <Arduino.h>
#include <atomic>
#include "CpuLoadMath.h"

// Static-lifetime object: each sampler is pinned to the core it observes, waking
// once per second so that core's idle task is switched out and its counter flushed.
class CpuLoadMonitor {
public:
    void begin();
    unsigned percent(unsigned core) const;
private:
    struct Core {
        std::atomic<unsigned> load{CpuLoadMath::Unavailable};
        std::atomic<uint32_t> sampledMs{0};
        unsigned id=0;
        TaskHandle_t task=nullptr;
    } cores[2];
    bool started=false;
    static void sampleTask(void *argument);
};
