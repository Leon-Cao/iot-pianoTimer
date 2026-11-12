# v0.1 验证记录

日期：2026-11-08（Asia/Tokyo）。

## 已完成的软件检查

- Arduino-ESP32 3.3.12，ESP32_Display_Panel 0.1.7，ESP32_IO_Expander 0.0.4，LVGL 8.3.11。
- FQBN：`esp32:esp32:esp32s3:FlashSize=16M,PartitionScheme=app3M_fat9M_16MB,PSRAM=opi,CDCOnBoot=cdc`。
- MicTest 编译成功：356096 bytes Flash，47440 bytes 静态 RAM。
- PianoTimer 编译成功：669475 bytes Flash，115256 bytes 静态 RAM。
- C++ 主机测试通过，启用 AddressSanitizer、UndefinedBehaviorSanitizer：短停顿、长停顿、1秒边界、Reset、重复区间、丢帧边界、长时长与合成信号测试。

## 尚未完成的实物检查

- 未烧录本版程序，未确认实际麦克风模式与声道。
- 未获得本板实录 WAV，尚不能确认采样正确。
- 未实测屏幕布局、触摸 Reset、噪声校准和持续运行。
- 未测量真实钢琴召回率、人声误检率、混合声音检测表现及累计时间误差。

**编译和合成信号通过不代表钢琴识别要求已经达标。** 下一步先完成 MicTest 的录音听检，再按 PianoTimer/README.md 的矩阵验收。当前实现采用实验性特征规则，可靠排除人声仍是待完成目标。
