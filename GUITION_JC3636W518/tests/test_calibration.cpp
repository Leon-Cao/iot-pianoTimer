#include "NoiseCalibration.h"
#include <cassert>
#include <cmath>
#include <cstdio>
int main(){
 NoiseCalibration c;
 for(int i=0;i<4;i++)assert(!c.add(-23.8));
 for(int i=0;i<23;i++)assert(!c.add(i<3?-30:-72));
 assert(c.add(-72));assert(fabs(c.noiseDb()+72)<.01);
 c.reset();assert(std::isnan(c.noiseDb()));
 for(int i=0;i<4;i++)c.add(-40);
 assert(!c.add(NAN));
 for(int i=0;i<24;i++)c.add(-50);
 assert(fabs(c.noiseDb()+50)<.01);
 puts("PASS: startup transient excluded, occasional noise rejected, reset, invalid samples.");
}
