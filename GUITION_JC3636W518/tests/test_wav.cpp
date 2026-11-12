#include "WavFormat.h"
#include <cassert>
#include <cstdio>
int main(){
 uint8_t h[44];WavFormat::header(h);
 assert(memcmp(h,"RIFF",4)==0 && memcmp(h+8,"WAVEfmt ",8)==0);
 assert(memcmp(h+36,"data",4)==0 && h[20]==1 && h[22]==1 && h[34]==16);
 auto read32=[](const uint8_t*p){return uint32_t(p[0])|(uint32_t(p[1])<<8)|(uint32_t(p[2])<<16)|(uint32_t(p[3])<<24);};
 assert(read32(h+4)==160036 && read32(h+24)==16000 && read32(h+28)==32000 && read32(h+40)==160000);
 puts("PASS: WAV header size/rate/format/channels.");
}
