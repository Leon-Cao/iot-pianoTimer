# iot-pianoTimer

为了记录女儿弹琴时间而创建的识别琴声和弹琴时长的计时器。

## 第一版开发入口

- `MicTest/MicTest.ino`：麦克风模式/声道诊断；先读 [采音步骤](MicTest/README.md)。
- `PianoTimer/PianoTimer.ino`：实验性琴声检测、HH:MM:SS、1 秒短停顿补计、Reset；读 [运行与限制](PianoTimer/README.md)。
- `libraries/PianoAudio`：本项目自己的采音、检测与计时逻辑。
- `tools/capture_mic.py`：将设备采音保存成 5 秒 WAV，用于听检和后续评估。
- `tests`：计时边界与合成信号测试。

当前是实验原型：识别规则还未通过实录验证，不代表已经可靠排除人声。请先完成麦克风验证，再评价计时结果。原有 `PanelTest`、`Porting` 保留用于屏幕与触摸排查。

本次开发环境：Arduino-ESP32 3.3.12、ESP32_Display_Panel 0.1.7、ESP32_IO_Expander 0.0.4、LVGL 8.3.11。旧安装指南的 3.0.7 为早期建议，本版在实际安装的 3.3.12 上编译验证。
