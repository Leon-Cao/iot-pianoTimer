#include <PianoMic.h>
#include <math.h>
PianoMic mic;
MicConfig config;
bool ready=false;
unsigned frames=0;
void help(){Serial.println("\nMicTest: i=I2S, p=PDM, l=left, r=right, s=save VERIFIED choice, w=5s PCM capture, ?=help");
 Serial.println("Check quiet/speech/piano response AND listen to exported WAV before s. Nonzero samples alone prove nothing.");}
void restart(){ready=mic.begin(config);Serial.printf("mode=%s channel=%s init=%s\n",config.pdm?"PDM":"I2S",config.right?"right":"left",ready?"OK":mic.error);}
void stats(const char *name,const int16_t *v){double sum=0,sq=0;int peak=0;unsigned clips=0,changes=0;
 for(unsigned i=0;i<PianoMic::Frame;i++){sum+=v[i];sq+=double(v[i])*v[i];peak=max(peak,abs(int(v[i])));clips+=abs(int(v[i]))>=32700;if(i&&v[i]!=v[i-1])++changes;}
 double mean=sum/PianoMic::Frame,var=max(0.0,sq/PianoMic::Frame-mean*mean);
 Serial.printf("%s_db:%.1f %s_peak:%d %s_clip:%.3f %s_changes:%u ",name,10*log10(var/(32768.0*32768.0)+1e-12),name,peak,name,double(clips)/PianoMic::Frame,name,changes);
}
void capture(){
 if(!mic.begin(config)){Serial.println("CAPTURE_ERROR");ready=false;return;}
 const uint32_t bytes=5*PianoMic::Rate*2;
 Serial.print("PCM_BEGIN ");Serial.println(bytes);
 uint32_t remaining=bytes;
 while(remaining){if(!mic.read()){ready=false;mic.stop();return;}
   size_t n=min(remaining,uint32_t(PianoMic::Frame*2));
   if(Serial.write(reinterpret_cast<const uint8_t*>(mic.samples()),n)!=n){ready=false;mic.stop();return;}remaining-=n;}
 Serial.println("\nPCM_END");restart();
}
void setup(){Serial.begin(921600);delay(1000);loadMicConfig(config);help();restart();}
void loop(){
 if(Serial.available()){char cmd=Serial.read();
  if(cmd=='i'||cmd=='p'){config.pdm=cmd=='p';restart();}
  else if(cmd=='l'||cmd=='r'){config.right=cmd=='r';restart();}
  else if(cmd=='s')Serial.println(ready&&saveMicConfig(config)?"Saved. Ready for PianoTimer.":"NOT SAVED");
  else if(cmd=='w'&&ready)capture();else if(cmd=='?')help();}
 if(!ready){delay(50);return;}
 if(!mic.read()){Serial.printf("MIC_ERROR: %s\n",mic.error);ready=false;mic.stop();return;}
 if(++frames%4==0){stats("left",mic.left);stats("right",mic.right);Serial.println();}
}
