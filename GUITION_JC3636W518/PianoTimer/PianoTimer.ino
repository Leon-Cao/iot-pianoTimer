#include <Arduino.h>
#include <ESP_Panel_Library.h>
#include <lvgl.h>
#include "lvgl_port_v8.h"
#include <PianoMic.h>
#include <PianoDetector.h>
#include <PracticeTimer.h>
#include <atomic>
#include "CpuLoadMonitor.h"
#include "OfflineRecorder.h"
#include "NoiseCalibration.h"
PianoMic mic;
MicConfig config;
PianoDetector detector;
PracticeTimer timer;
std::atomic<bool> resetRequested{false};
lv_obj_t *clockLabel,*statusLabel,*levelBar,*cpuLabel;
CpuLoadMonitor cpuMonitor;
OfflineRecorder recorder;
lv_obj_t *recordButton,*resetButton;
std::atomic<bool> recordRequested{false},formatRequested{false},cancelStorage{false},recordBusy{false};
bool micVerified=false,wasReady=false;
uint32_t resumeAt=0;
char serialCommand[40]{};
size_t serialCommandLength=0;

bool ready=false,calibrating=true,held=false,previousCandidate=false;
unsigned logFrames=0,stuckFrames=0;
NoiseCalibration noiseCalibration;
uint64_t sampleClock=0,heldStart=0;
bool heldCandidate=false;
constexpr uint64_t FrameUs=uint64_t(PianoMic::Frame)*1000000/PianoMic::Rate;
void show(const char *status,float db=-120){
 if(!lvgl_port_lock(100))return;
 char text[32];uint64_t seconds=timer.totalUs/1000000;
 snprintf(text,sizeof(text),"%02llu:%02llu:%02llu",(unsigned long long)(seconds/3600),(unsigned long long)(seconds/60%60),(unsigned long long)(seconds%60));
 lv_label_set_text(clockLabel,text);lv_label_set_text(statusLabel,status);
 lv_bar_set_value(levelBar,constrain(int(db+80),0,80),LV_ANIM_OFF);lvgl_port_unlock();
}
// Invoked by LVGL while its task already holds the UI mutex.
void updateCpuLabels(lv_timer_t*) {
 char core0[8],core1[8],text[40];
 unsigned a=cpuMonitor.percent(0),b=cpuMonitor.percent(1);
 if(a<=100)snprintf(core0,sizeof(core0),"%u%%",a);else snprintf(core0,sizeof(core0),"--%%");
 if(b<=100)snprintf(core1,sizeof(core1),"%u%%",b);else snprintf(core1,sizeof(core1),"--%%");
 snprintf(text,sizeof(text),"CPU0 %s    CPU1 %s",core0,core1);
 lv_label_set_text(cpuLabel,text);
}
void resetEvent(lv_event_t*){if(!recordBusy.load())resetRequested.store(true);}
void setRecordingControls(bool busy){
 if(!lvgl_port_lock(-1))return;
 if(busy){lv_obj_add_state(recordButton,LV_STATE_DISABLED);lv_obj_add_state(resetButton,LV_STATE_DISABLED);}
 else{lv_obj_clear_state(recordButton,LV_STATE_DISABLED);lv_obj_clear_state(resetButton,LV_STATE_DISABLED);}
 lvgl_port_unlock();
}
void recordEvent(lv_event_t*){if(!recordBusy.exchange(true))recordRequested.store(true);}
void storageAnswer(lv_event_t *event){
 auto *box=static_cast<lv_obj_t*>(lv_event_get_current_target(event));
 const char *answer=lv_msgbox_get_active_btn_text(box);
 if(!answer)return;
 if(strcmp(answer,"Erase & init")==0)formatRequested.store(true);else cancelStorage.store(true);
 lv_msgbox_close(box);
}
void pauseForRecording(){
 wasReady=ready;ready=false;mic.stop();held=false;previousCandidate=false;detector.reset();timer.discontinuity();
 setRecordingControls(true);
}
void finishRecording(const char *message){
 recorder.release();show(message);resumeAt=millis()+4000;
}
void captureOffline(){
 if(!recorder.prepare()){finishRecording(recorder.error);return;}
 for(int i=3;i>0;i--){char msg[40];snprintf(msg,sizeof(msg),"Get ready: %d",i);show(msg);delay(1000);}
 if(!mic.begin(config)){finishRecording("Microphone start failed");return;}
 while(!recorder.complete()){
   char msg[40];snprintf(msg,sizeof(msg),"Recording: %u s",recorder.secondsLeft());show(msg);
   if(!mic.read()){mic.stop();finishRecording("Recording failed: audio lost");return;}
   recorder.append(mic.samples(),PianoMic::Frame);
 }
 mic.stop();show("Saving and checking...");
 if(recorder.save()){
   char msg[64];snprintf(msg,sizeof(msg),"Saved\n%s",recorder.filename()+1);finishRecording(msg);
 }else finishRecording(recorder.error);
}
void beginOfflineRecording(){
 pauseForRecording();
 if(!recorder.partitionOK()){finishRecording(recorder.error);return;}
 if(recorder.mount()){captureOffline();return;}
 show("Storage setup needed");
 lvgl_port_lock(-1);
 static const char *buttons[]={"Cancel","Erase & init",""};
 auto *box=lv_msgbox_create(nullptr,"Storage setup",
   "Storage cannot be opened.\nInitialize it? Existing files\nin data storage will be erased.",buttons,false);
 lv_obj_set_width(box,280);lv_obj_center(box);lv_obj_add_event_cb(box,storageAnswer,LV_EVENT_VALUE_CHANGED,nullptr);
 lvgl_port_unlock();
}
void serialActions(){
 while(Serial.available()){
   char c=Serial.read();
   if(c=='\r'||c=='\n'){
     serialCommand[serialCommandLength]=0;serialCommandLength=0;
     if(!serialCommand[0])continue;
     if(recordBusy.load()){Serial.println("RECORD_ERROR busy");continue;}
     if(strcmp(serialCommand,"list")==0||strncmp(serialCommand,"export ",7)==0){
       recordBusy=true;pauseForRecording();show("Exporting recordings...");
       if(strcmp(serialCommand,"list")==0)recorder.list();else recorder.exportFile(strtoul(serialCommand+7,nullptr,10));
       finishRecording("Transfer finished");
     }else if(strcmp(serialCommand,"verify")==0){
       // Explicit human confirmation after listening; recording alone does not verify audio.
       if(saveMicConfig(config)){micVerified=true;Serial.println("MIC_CONFIG_SAVED");show("Mic verified: restart device");}
       else Serial.println("MIC_CONFIG_SAVE_FAILED");
     }
   }else if(serialCommandLength<sizeof(serialCommand)-1)serialCommand[serialCommandLength++]=c;
   else serialCommandLength=0;
 }
}
void clearPipeline(){timer.reset();detector.reset();held=false;previousCandidate=false;sampleClock=0;}
void setup(){
 Serial.begin(115200);
 auto *panel=new ESP_Panel();
 if(!panel->init()||!panel->begin()||!lvgl_port_init(panel->getLcd(),panel->getTouch())){
   Serial.println("DISPLAY_INIT_FAILED");while(true)delay(1000);}
 lvgl_port_lock(-1);
 auto *screen=lv_scr_act();lv_obj_set_style_bg_color(screen,lv_color_hex(0x101820),0);
 lv_obj_set_style_text_color(screen,lv_color_hex(0xf4f5f7),0);
 auto *title=lv_label_create(screen);lv_label_set_text(title,"PIANO TIMER");lv_obj_align(title,LV_ALIGN_TOP_MID,0,42);
 auto *note=lv_label_create(screen);lv_label_set_text(note,"EXPERIMENTAL v0.1");lv_obj_align(note,LV_ALIGN_TOP_MID,0,68);
 clockLabel=lv_label_create(screen);lv_obj_set_style_text_font(clockLabel,&lv_font_montserrat_32,0);
 lv_label_set_text(clockLabel,"00:00:00");lv_obj_align(clockLabel,LV_ALIGN_CENTER,0,-25);
 statusLabel=lv_label_create(screen);lv_obj_set_width(statusLabel,280);lv_obj_set_style_text_align(statusLabel,LV_TEXT_ALIGN_CENTER,0);
 lv_label_set_text(statusLabel,"Starting...");lv_obj_align(statusLabel,LV_ALIGN_CENTER,0,20);
 levelBar=lv_bar_create(screen);lv_obj_set_size(levelBar,180,8);lv_bar_set_range(levelBar,0,80);lv_obj_align(levelBar,LV_ALIGN_CENTER,0,57);
 cpuLabel=lv_label_create(screen);lv_label_set_text(cpuLabel,"CPU0 --%    CPU1 --%");
 lv_obj_set_style_text_font(cpuLabel,&lv_font_montserrat_14,0);
 lv_obj_set_style_text_color(cpuLabel,lv_color_hex(0xa5becf),0);
 lv_obj_align(cpuLabel,LV_ALIGN_CENTER,0,76);
 lv_timer_create(updateCpuLabels,1000,nullptr);
 auto *button=lv_btn_create(screen);resetButton=button;lv_obj_set_size(button,112,44);lv_obj_align(button,LV_ALIGN_BOTTOM_MID,-62,-45);
 lv_obj_add_event_cb(button,resetEvent,LV_EVENT_CLICKED,nullptr);
 auto *label=lv_label_create(button);lv_label_set_text(label,"Reset");lv_obj_center(label);
 recordButton=lv_btn_create(screen);lv_obj_set_size(recordButton,112,44);lv_obj_align(recordButton,LV_ALIGN_BOTTOM_MID,62,-45);
 lv_obj_add_event_cb(recordButton,recordEvent,LV_EVENT_CLICKED,nullptr);
 auto *recordText=lv_label_create(recordButton);lv_label_set_text(recordText,"Record");lv_obj_center(recordText);
 lvgl_port_unlock();
 cpuMonitor.begin();
 micVerified=loadMicConfig(config);
 if(!micVerified){config={false,false};show("I2S left: tap Record\nVerify WAV before counting");Serial.println("MIC_NOT_VERIFIED: Record uses I2S left; listen to WAV then send verify");return;}
 ready=mic.begin(config);
 if(!ready){show("Microphone error");Serial.println(mic.error);return;}
 show("Keep quiet: calibrating");
 Serial.println("Experimental spectral detector; speech rejection NOT validated. CSV: db,flatness,tonal,peaks,candidate,total_us");
}
void loop(){
 serialActions();
 if(recordRequested.exchange(false)){beginOfflineRecording();return;}
 if(formatRequested.exchange(false)){
   show("Preparing storage...");
   if(recorder.mount(true))captureOffline();else finishRecording("Storage initialization failed");
   return;
 }
 if(cancelStorage.exchange(false)){finishRecording("Recording cancelled");return;}
 if(recordBusy.load()){
   if(resumeAt&&int32_t(millis()-resumeAt)>=0){
     resumeAt=0;held=false;previousCandidate=false;detector.reset();timer.discontinuity();
     if(calibrating)noiseCalibration.reset();
     ready=wasReady&&mic.begin(config);recordBusy=false;setRecordingControls(false);
     show(ready?(calibrating?"Keep quiet: calibrating":"Waiting for piano"):
         (micVerified?"Microphone not ready":"Tap Record to test audio"));
   }
   delay(20);return;
 }
 if(resetRequested.exchange(false)){
   clearPipeline();
   if(ready){ready=mic.begin(config);}
   show(ready?(calibrating?"Keep quiet: calibrating":"Waiting for piano"):"Microphone not ready");
 }
 if(!ready){delay(20);return;}
 if(!mic.read()){
   timer.discontinuity();held=false;ready=false;mic.stop();show("Audio error: counting stopped");Serial.println(mic.error);return;
 }
 // If Reset was clicked during capture, discard this block; reset drains DMA on next iteration.
 if(resetRequested.load())return;
 AudioFeatures f=detector.analyze(mic.samples());
 if(f.db<-100||f.clipping>0.10f)++stuckFrames;else stuckFrames=0;
 if(stuckFrames>=16){timer.discontinuity();held=false;ready=false;mic.stop();show("Invalid audio: check MicTest");Serial.println("INVALID_AUDIO: sustained silence/clipping");return;}
 if(calibrating){
   if(noiseCalibration.add(f.db)){
     float noise=noiseCalibration.noiseDb();
     if(noise>-20||noise<-100){ready=false;mic.stop();show("Check mic / quiet room\nRestart to calibrate");Serial.printf("Calibration rejected: noise=%.1f\n",noise);return;}
     detector.gateDb=max(-60.0f,noise+12.0f);detector.reset();calibrating=false;
     Serial.printf("Calibration noise=%.1f gate=%.1f dBFS\n",noise,detector.gateDb);
   }
   show(calibrating?"Keep quiet: calibrating":"Waiting for piano",f.db);return;
 }
 // One-frame lookahead confirms neighboring candidates and retroactively counts the first.
 bool accepted=false;
 if(held){accepted=heldCandidate&&(previousCandidate||f.candidate);
   timer.add(accepted,heldStart,heldStart+FrameUs);previousCandidate=heldCandidate;
 }
 held=true;heldCandidate=f.candidate;heldStart=sampleClock;sampleClock+=FrameUs;
 char diagnostic[96];
 if(accepted)snprintf(diagnostic,sizeof(diagnostic),"Piano candidate: counting");
 else if(f.db<=detector.gateDb)snprintf(diagnostic,sizeof(diagnostic),"Below threshold\nSound %.0f / gate %.0f dB",f.db,detector.gateDb);
 else if(f.clipping>=0.01f)snprintf(diagnostic,sizeof(diagnostic),"Audio clipping");
 else if(f.flatness>=0.12f||f.tonal<=0.55f||f.peaks<3)snprintf(diagnostic,sizeof(diagnostic),"Sound present\nNot piano-like yet");
 else snprintf(diagnostic,sizeof(diagnostic),"Sound present\nWaiting for stable piano");
 show(diagnostic,f.db);
 if(++logFrames%4==0)Serial.printf("db=%.1f gate=%.1f flat=%.3f tonal=%.3f peaks=%u candidate=%d total_us=%llu\n",f.db,detector.gateDb,f.flatness,f.tonal,f.peaks,int(f.candidate),(unsigned long long)timer.totalUs);
}
