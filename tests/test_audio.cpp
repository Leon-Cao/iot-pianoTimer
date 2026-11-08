#include "PracticeTimer.h"
#include "PianoDetector.h"
#include <cassert>
#include <cstdio>
#include <random>
static PianoDetector detector;
static int16_t pcm[PianoDetector::N];
int main(){
 PracticeTimer t;
 t.add(true,0,4000000);t.add(false,4000000,4600000);assert(t.totalUs==4000000);
 t.add(true,4600000,7600000);assert(t.totalUs==7600000);
 t.reset();t.add(true,0,4000000);t.add(false,4000000,9000000);t.add(true,9000000,12000000);assert(t.totalUs==7000000);
 t.reset();for(unsigned i=0;i<10;i++){t.add(true,i*5000000ULL,i*5000000ULL+500000);t.add(false,i*5000000ULL+500000,(i+1)*5000000ULL);}assert(t.totalUs==5000000);
 t.reset();t.add(true,0,100);t.add(false,100,1000100);t.add(true,1000100,1000200);assert(t.totalUs==1000200); // exactly 1s
 t.reset();t.add(true,0,100);t.add(false,100,1000101);t.add(true,1000101,1000201);assert(t.totalUs==200);
 t.reset();t.add(true,0,100);t.add(false,100,500);t.reset();t.add(true,500,600);assert(t.totalUs==100);
 t.reset();t.add(true,0,100);t.discontinuity();t.add(true,200,300);assert(t.totalUs==200);
 t.add(true,200,300);assert(t.totalUs==200); // duplicate interval
 t.reset();t.add(true,0,1000000ULL*60*60*120);assert(t.totalUs/1000000==432000);
 detector.reset();assert(!detector.analyze(pcm).candidate);
 std::mt19937 rng(3);std::uniform_int_distribution<int> noise(-10000,10000);
 for(int f=0;f<10;f++){for(auto &x:pcm)x=noise(rng);assert(!detector.analyze(pcm).candidate);}
 detector.reset();for(size_t i=0;i<PianoDetector::N;i++)pcm[i]=10000*sin(2*3.141592653589793*440*i/16000);assert(!detector.analyze(pcm).candidate);
 detector.reset();int positives=0;
 for(int f=0;f<8;f++){for(size_t i=0;i<PianoDetector::N;i++){double time=(f*PianoDetector::N+i)/16000.0;pcm[i]=8000*exp(-time)*(sin(2*M_PI*440*time)+.5*sin(2*M_PI*880*time)+.25*sin(2*M_PI*1320*time));}positives+=detector.analyze(pcm).candidate;}
 assert(positives>=2);
 puts("PASS: gap boundary/reset/overlap/dropout/long duration; synthetic audio smoke tests (NOT real piano/speech accuracy).");
}
