#pragma once
#include <Arduino.h>
#include <algorithm>
#include <FFat.h>
#include <esp_partition.h>
#include <esp_heap_caps.h>
#include "WavFormat.h"

class OfflineRecorder {
public:
 const char *error="none";
 bool partitionOK(){
   const auto *p=esp_partition_find_first(ESP_PARTITION_TYPE_DATA,ESP_PARTITION_SUBTYPE_DATA_FAT,"ffat");
   if(!p||p->address!=0x610000||p->size!=0x9E0000){error="Use 16M / 3MB APP partition";return false;}
   return true;
 }
 bool mount(bool eraseConfirmed=false){
   if(mounted)return true;
   if(!partitionOK())return false;
   // Formatting is only possible after the user explicitly confirms on screen.
   mounted=FFat.begin(eraseConfirmed);
   if(!mounted)error="Storage not ready";
   return mounted;
 }
 bool prepare(){
   release();
   if(!mounted){error="Storage not mounted";return false;}
   if(FFat.freeBytes()<WavFormat::FileBytes+8192){error="Storage full: export files";return false;}
   bool found=false;
   for(unsigned i=1;i<=9999;i++){snprintf(path,sizeof(path),"/piano-%04u.wav",i);if(!FFat.exists(path)){found=true;break;}}
   if(!found){error="Recording limit reached";return false;}
   pcm=static_cast<int16_t*>(heap_caps_malloc(WavFormat::DataBytes,MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT));
   if(!pcm){error="Recording needs OPI PSRAM";return false;}
   return true;
 }
 void append(const int16_t *data,size_t count){
   size_t n=std::min(count,size_t(WavFormat::Samples)-used);
   if(pcm&&n){memcpy(pcm+used,data,n*2);used+=n;}
 }
 unsigned secondsLeft()const{return (WavFormat::Samples-used+WavFormat::SampleRate-1)/WavFormat::SampleRate;}
 bool complete()const{return used==WavFormat::Samples;}
 const char *filename()const{return path;}
 bool save(){
   if(!pcm||!complete()){error="Incomplete audio";return false;}
   constexpr const char *temp="/piano-recording.tmp";
   FFat.remove(temp); // only our own interrupted temporary recording
   File file=FFat.open(temp,FILE_WRITE);
   if(!file){error="Cannot create recording";return false;}
   uint8_t header[44];WavFormat::header(header);
   bool ok=file.write(header,44)==44;
   const uint8_t *data=reinterpret_cast<const uint8_t*>(pcm);
   for(size_t offset=0;ok&&offset<WavFormat::DataBytes;){
     size_t n=std::min(size_t(4096),size_t(WavFormat::DataBytes)-offset);
     ok=file.write(data+offset,n)==n;offset+=n;delay(1);
   }
   file.flush();file.close();
   // Read every byte back before reporting Saved. Capture is stopped during flash I/O.
   file=FFat.open(temp,FILE_READ);
   ok=ok&&bool(file)&&file.size()==WavFormat::FileBytes;
   uint8_t check[512];
   if(ok)ok=file.read(check,44)==44&&memcmp(check,header,44)==0;
   for(size_t offset=0;ok&&offset<WavFormat::DataBytes;){
     size_t n=std::min(sizeof(check),size_t(WavFormat::DataBytes)-offset);
     ok=file.read(check,n)==int(n)&&memcmp(check,data+offset,n)==0;offset+=n;delay(1);
   }
   file.close();
   if(ok)ok=!FFat.exists(path)&&FFat.rename(temp,path);
   if(!ok){FFat.remove(temp);error="Save/verify failed";}
   return ok;
 }
 void release(){if(pcm){free(pcm);pcm=nullptr;}used=0;}
 void list(){
   if(!mount()){Serial.println("RECORD_ERROR storage unavailable");return;}
   File root=FFat.open("/");File f=root.openNextFile();
   while(f){String name=f.name();if(name.startsWith("piano-")&&name.endsWith(".wav"))Serial.printf("RECORD %s %u\n",f.name(),unsigned(f.size()));f.close();f=root.openNextFile();}
   Serial.println("RECORD_LIST_END");
 }
 void exportFile(unsigned number){
   if(number<1||number>9999||!mount()){Serial.println("RECORD_ERROR invalid number or storage");return;}
   char name[24];snprintf(name,sizeof(name),"/piano-%04u.wav",number);
   File file=FFat.open(name,FILE_READ);
   if(!file||file.size()!=WavFormat::FileBytes){Serial.println("RECORD_ERROR missing or incomplete file");return;}
   Serial.printf("WAV_BEGIN %u\n",WavFormat::FileBytes);
   uint8_t bytes[512];uint32_t crc=0xffffffff;
   while(file.available()){
     int n=file.read(bytes,sizeof(bytes));if(n<=0){file.close();return;}
     for(int i=0;i<n;i++){crc^=bytes[i];for(int bit=0;bit<8;bit++)crc=(crc>>1)^(0xedb88320u&uint32_t(-int32_t(crc&1)));}
     if(Serial.write(bytes,n)!=size_t(n)){file.close();return;}delay(1);
   }
   file.close();Serial.printf("\nWAV_END %08lx\n",static_cast<unsigned long>(crc^0xffffffff));
 }
private:
 bool mounted=false;
 int16_t *pcm=nullptr;size_t used=0;
 char path[24]{};
};
