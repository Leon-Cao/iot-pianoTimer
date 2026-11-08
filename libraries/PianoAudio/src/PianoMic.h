#pragma once
#include <Arduino.h>
#include <Preferences.h>
#include <driver/i2s_std.h>
#include <driver/i2s_pdm.h>
#include <atomic>
struct MicConfig {bool pdm=false,right=false;};
inline bool loadMicConfig(MicConfig &c) {
 Preferences p;if(!p.begin("piano-mic",true))return false;
 bool ok=p.getBool("verified",false);c.pdm=p.getBool("pdm",false);c.right=p.getBool("right",false);p.end();return ok;
}
inline bool saveMicConfig(MicConfig c) {
 Preferences p;if(!p.begin("piano-mic",false))return false;
 bool ok=p.putBool("pdm",c.pdm)>0;ok=(p.putBool("right",c.right)>0)&&ok;
 ok=(p.putBool("verified",ok)>0)&&ok;p.end();return ok;
}
class PianoMic {
 public:
 static constexpr unsigned Rate=16000,Frame=2048;
 int16_t left[Frame]{},right[Frame]{};
 std::atomic<bool> overflow{false};
 const char *error="none";
 ~PianoMic(){stop();}
 void stop(){if(rx){i2s_channel_disable(rx);i2s_del_channel(rx);rx=nullptr;}}
 bool begin(MicConfig config) {
   stop();overflow=false;c=config;
   i2s_chan_config_t channel=I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0,I2S_ROLE_MASTER);
   channel.dma_desc_num=8;channel.dma_frame_num=256;
   esp_err_t e=i2s_new_channel(&channel,nullptr,&rx);
   if(e!=ESP_OK){error=esp_err_to_name(e);return false;}
   if(c.pdm){
     i2s_pdm_rx_config_t cfg={};
     cfg.clk_cfg=I2S_PDM_RX_CLK_DEFAULT_CONFIG(Rate);
     cfg.slot_cfg=I2S_PDM_RX_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT,I2S_SLOT_MODE_STEREO);
     cfg.gpio_cfg.clk=GPIO_NUM_45;cfg.gpio_cfg.din=GPIO_NUM_46;
     e=i2s_channel_init_pdm_rx_mode(rx,&cfg);
   }else{
     i2s_std_config_t cfg={};
     cfg.clk_cfg=I2S_STD_CLK_DEFAULT_CONFIG(Rate);
     cfg.slot_cfg=I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_32BIT,I2S_SLOT_MODE_STEREO);
     cfg.gpio_cfg.mclk=I2S_GPIO_UNUSED;cfg.gpio_cfg.bclk=GPIO_NUM_42;
     cfg.gpio_cfg.ws=GPIO_NUM_45;cfg.gpio_cfg.dout=I2S_GPIO_UNUSED;cfg.gpio_cfg.din=GPIO_NUM_46;
     e=i2s_channel_init_std_mode(rx,&cfg);
   }
   if(e==ESP_OK){i2s_event_callbacks_t cb={};cb.on_recv_q_ovf=onOverflow;e=i2s_channel_register_event_callback(rx,&cb,this);}
   if(e==ESP_OK)e=i2s_channel_enable(rx);
   if(e!=ESP_OK){error=esp_err_to_name(e);stop();return false;}
   error="none";return true;
 }
 bool read() {
   if(!rx)return false;
   size_t want=Frame*2*(c.pdm?sizeof(int16_t):sizeof(int32_t)),done=0;
   while(done<want){size_t n=0;esp_err_t e=i2s_channel_read(rx,raw+done,want-done,&n,300);
     if(e!=ESP_OK||!n){error=esp_err_to_name(e);return false;}done+=n;}
   if(overflow.exchange(false)){error="RX overflow: audio dropped";return false;}
   if(c.pdm){auto *v=reinterpret_cast<int16_t*>(raw);for(unsigned i=0;i<Frame;i++){left[i]=v[2*i];right[i]=v[2*i+1];}}
   else{auto *v=reinterpret_cast<int32_t*>(raw);for(unsigned i=0;i<Frame;i++){left[i]=v[2*i]>>16;right[i]=v[2*i+1]>>16;}}
   return true;
 }
 const int16_t *samples()const{return c.right?right:left;}
 private:
 i2s_chan_handle_t rx=nullptr;MicConfig c;
 alignas(4) uint8_t raw[Frame*8]{};
 static bool IRAM_ATTR onOverflow(i2s_chan_handle_t,i2s_event_data_t*,void *ctx){static_cast<PianoMic*>(ctx)->overflow.store(true);return false;}
};
