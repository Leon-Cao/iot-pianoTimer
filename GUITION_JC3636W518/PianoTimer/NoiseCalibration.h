#pragma once
#include <algorithm>
#include <math.h>
class NoiseCalibration {
public:
 void reset(){warmup=0;count=0;}
 bool add(float db){
   if(warmup<4){++warmup;return false;} // discard first 512ms after I2S starts
   if(!isfinite(db))return false;
   if(count<24)values[count++]=db;
   return count==24;
 }
 float noiseDb()const{
   if(count!=24)return NAN;
   float sorted[24];std::copy(values,values+24,sorted);std::sort(sorted,sorted+24);
   return sorted[6]; // lower quartile resists occasional handling/transient sounds
 }
private:
 unsigned warmup=0,count=0;float values[24]{};
};
