#include <Arduino.h>
#include <ESP_Panel_Library.h>
#include <lvgl.h>
#include "lvgl_port_v8.h"
#include <PianoMic.h>
#include <PianoDetector.h>
#include <PracticeTimer.h>
#include <atomic>
PianoMic mic;
MicConfig config;
PianoDetector detector;
PracticeTimer timer;
std::atomic<bool> resetRequested{false};
lv_obj_t *clockLabel,*statusLabel,*levelBar;
bool ready=false,calibrating=true,held=false,previousCandidate=false;
unsigned calibrationFrames=0,logFrames=0,stuckFrames=0;
float calibrationPower=0;
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
void resetEvent(lv_event_t*){resetRequested.store(true);}
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
 auto *button=lv_btn_create(screen);lv_obj_set_size(button,130,48);lv_obj_align(button,LV_ALIGN_BOTTOM_MID,0,-45);
 lv_obj_add_event_cb(button,resetEvent,LV_EVENT_CLICKED,nullptr);
 auto *label=lv_label_create(button);lv_label_set_text(label,"Reset");lv_obj_center(label);
 lvgl_port_unlock();
 if(!loadMicConfig(config)){show("Run MicTest first\nVerify audio, then send s");Serial.println("MIC_NOT_VERIFIED: run MicTest and save mode");return;}
 ready=mic.begin(config);
 if(!ready){show("Microphone error");Serial.println(mic.error);return;}
 show("Keep quiet: calibrating");
 Serial.println("Experimental spectral detector; speech rejection NOT validated. CSV: db,flatness,tonal,peaks,candidate,total_us");
}
void loop(){
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
   calibrationPower+=powf(10.0f,f.db/10.0f);
   if(++calibrationFrames>=24){
     float noise=10*log10f(calibrationPower/calibrationFrames+1e-12f);
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
 show(accepted?"Piano candidate: counting":(timer.totalUs?"Paused / waiting":"Waiting for piano"),f.db);
 if(++logFrames%4==0)Serial.printf("%.1f,%.3f,%.3f,%u,%d,%llu\n",f.db,f.flatness,f.tonal,f.peaks,int(f.candidate),(unsigned long long)timer.totalUs);
}
