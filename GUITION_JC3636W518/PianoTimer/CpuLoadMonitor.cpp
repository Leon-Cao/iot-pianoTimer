#include "CpuLoadMonitor.h"
#include <esp_timer.h>
#include <freertos/task.h>

#if CONFIG_FREERTOS_GENERATE_RUN_TIME_STATS && CONFIG_FREERTOS_USE_TRACE_FACILITY && \
    CONFIG_FREERTOS_RUN_TIME_STATS_USING_ESP_TIMER && !CONFIG_FREERTOS_UNICORE && !CONFIG_FREERTOS_SMP
#define PIANO_CPU_STATS_SUPPORTED 1
#else
#define PIANO_CPU_STATS_SUPPORTED 0
#endif

void CpuLoadMonitor::begin() {
    if (started) return;
    started=true;
#if PIANO_CPU_STATS_SUPPORTED
    static_assert(sizeof(configRUN_TIME_COUNTER_TYPE)==sizeof(uint32_t), "Review counter math for this SDK");
    for (unsigned i=0;i<2;i++) {
        cores[i].id=i;
        if (xTaskCreatePinnedToCore(sampleTask,i==0?"cpu_load_0":"cpu_load_1",
                                   2048,&cores[i],1,&cores[i].task,i)!=pdPASS) {
            Serial.printf("CPU%u monitor could not start\n",i);
        }
    }
#else
    Serial.println("Per-core runtime statistics unavailable in this SDK; CPU labels show --");
#endif
}

unsigned CpuLoadMonitor::percent(unsigned core) const {
    if (core>=2) return CpuLoadMath::Unavailable;
    unsigned value=cores[core].load.load(std::memory_order_acquire);
    uint32_t now=uint32_t(esp_timer_get_time()/1000);
    if (uint32_t(now-cores[core].sampledMs.load(std::memory_order_relaxed))>3000)
        return CpuLoadMath::Unavailable; // never display an indefinitely stale reading
    return value;
}

void CpuLoadMonitor::sampleTask(void *argument) {
#if PIANO_CPU_STATS_SUPPORTED
    auto &core=*static_cast<Core*>(argument);
    TaskHandle_t idle=xTaskGetIdleTaskHandleForCore(core.id);
    if (!idle) {vTaskDelete(nullptr);return;}
    TaskStatus_t status={};
    vTaskGetInfo(idle,&status,pdFALSE,eInvalid); // skip stack watermark scanning
    uint32_t previous=status.ulRunTimeCounter;
    int64_t previousTime=esp_timer_get_time();
    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(1000));
        vTaskGetInfo(idle,&status,pdFALSE,eInvalid);
        int64_t now=esp_timer_get_time();
        unsigned load=CpuLoadMath::percent(previous,status.ulRunTimeCounter,uint64_t(now-previousTime));
        previous=status.ulRunTimeCounter;previousTime=now;
        core.sampledMs.store(uint32_t(now/1000),std::memory_order_relaxed);
        core.load.store(load,std::memory_order_release);
    }
#else
    (void)argument;
    vTaskDelete(nullptr);
#endif
}
