#include "CpuLoadMath.h"
#include <cassert>
#include <cstdio>
int main(){
 using CpuLoadMath::percent;
 assert(percent(0,1000000,1000000)==0);
 assert(percent(20,20,1000000)==100);
 assert(percent(100,750100,1000000)==25);
 assert(percent(100,1000100,2000000)==50); // actual delayed window
 assert(percent(0xfffffff0u,0x10,64)==50); // wrap
 assert(percent(0,1000010,1000000)==0); // clamp tiny snapshot jitter
 assert(percent(0,0,0)==CpuLoadMath::Unavailable);
 assert(percent(0,0,uint64_t(UINT32_MAX)+1)==CpuLoadMath::Unavailable);
 puts("PASS: CPU percent idle/busy/partial/delayed/window wrap/invalid duration.");
}
