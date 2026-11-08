#pragma once
#include <stdint.h>
#include <stddef.h>
#include <math.h>
#include <algorithm>
struct AudioFeatures {
 float db=-120, flatness=1, tonal=0, centroid=0, clipping=0;
 unsigned peaks=0;
 bool candidate=false;
};
// EXPERIMENTAL spectral rules, not a trained classifier. Speech rejection is unvalidated.
// Isolated tone, broadband noise and transient rejection; decaying/stable rich tones favored.
class PianoDetector {
 public:
 static constexpr size_t N=2048;
 float gateDb=-50;
 void reset() { previousDb=-120; previousCentroid=0; age=1000; }
 AudioFeatures analyze(const int16_t *pcm) {
   AudioFeatures f;
   double mean=0,power=0;unsigned clipped=0;
   for(size_t i=0;i<N;i++) {mean+=pcm[i];clipped+=(pcm[i]>=32700||pcm[i]<=-32700);}
   mean/=N;
   for(size_t i=0;i<N;i++) {
     float v=(pcm[i]-mean)/32768.0f;power+=v*v;
     re[i]=v*(0.5f-0.5f*cosf(6.28318530718f*i/(N-1)));im[i]=0;
   }
   f.db=10*log10f(float(power/N)+1e-12f);f.clipping=float(clipped)/N;
   // In-place radix-2 FFT; arrays live in the object, not task stack.
   for(size_t i=1,j=0;i<N;i++) {size_t bit=N>>1;for(;j&bit;bit>>=1)j^=bit;j^=bit;if(i<j){std::swap(re[i],re[j]);std::swap(im[i],im[j]);}}
   for(size_t len=2;len<=N;len<<=1){float ar=cosf(-6.28318530718f/len),ai=sinf(-6.28318530718f/len);
     for(size_t i=0;i<N;i+=len){float wr=1,wi=0;for(size_t j=0;j<len/2;j++){
       size_t a=i+j,b=a+len/2;float tr=wr*re[b]-wi*im[b],ti=wr*im[b]+wi*re[b];
       re[b]=re[a]-tr;im[b]=im[a]-ti;re[a]+=tr;im[a]+=ti;
       float next=wr*ar-wi*ai;wi=wr*ai+wi*ar;wr=next;
     }}
   }
   const int lo=5,hi=768;float sum=0,logs=0,weighted=0,maxp=0;
   for(int k=0;k<=hi+1;k++){re[k]=re[k]*re[k]+im[k]*im[k];}
   for(int k=lo;k<=hi;k++){float p=re[k]+1e-12f;sum+=p;logs+=logf(p);weighted+=p*k*16000.0f/N;maxp=std::max(maxp,p);}
   f.flatness=expf(logs/(hi-lo+1))/(sum/(hi-lo+1)+1e-12f);
   f.centroid=weighted/(sum+1e-12f);
   float tonalPower=0;
   for(int k=lo+1;k<hi;k++) if(re[k]>re[k-1]&&re[k]>=re[k+1]&&re[k]>maxp*0.025f){
     ++f.peaks;tonalPower+=re[k-1]+re[k]+re[k+1];
   }
   f.tonal=std::min(1.0f,tonalPower/(sum+1e-12f));
   bool richTone=f.db>gateDb&&f.flatness<0.12f&&f.tonal>0.55f&&f.peaks>=3&&f.clipping<0.01f;
   bool stable=previousCentroid>0&&fabsf(f.centroid-previousCentroid)<std::max(100.0f,previousCentroid*0.25f);
   bool onset=f.db-previousDb>3.0f;
   if(richTone&&onset) age=0;else ++age;
   // Require an attack followed by stable/decaying tone; no volume-only trigger.
   f.candidate=richTone&&age<64&&(onset||(stable&&f.db-previousDb<2.0f));
   previousDb=f.db;previousCentroid=f.centroid;return f;
 }
 private:
 float re[N]{},im[N]{};
 float previousDb=-120,previousCentroid=0;
 unsigned age=1000;
};
