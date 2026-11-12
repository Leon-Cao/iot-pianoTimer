#pragma once
#include <stdint.h>
#include <string.h>
namespace WavFormat {
constexpr uint32_t SampleRate=16000, Samples=5*SampleRate, DataBytes=Samples*2, FileBytes=DataBytes+44;
inline void u16(uint8_t *p,uint16_t v){p[0]=v;p[1]=v>>8;}
inline void u32(uint8_t *p,uint32_t v){for(unsigned i=0;i<4;i++)p[i]=v>>(i*8);}
inline void header(uint8_t *h){
 memset(h,0,44);memcpy(h,"RIFF",4);u32(h+4,FileBytes-8);memcpy(h+8,"WAVEfmt ",8);
 u32(h+16,16);u16(h+20,1);u16(h+22,1);u32(h+24,SampleRate);u32(h+28,SampleRate*2);
 u16(h+32,2);u16(h+34,16);memcpy(h+36,"data",4);u32(h+40,DataBytes);
}
}
