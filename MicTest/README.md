# 麦克风验证：先做这一步

这是只输出串口数据的诊断程序，**不会初始化屏幕**；烧录后屏幕没有界面是正常情况。

## 打开和上传

1. Arduino IDE 的项目文件夹设为 `/Users/leon/iot-pianoTimer`。
2. 文件 → 打开，选择 `/Users/leon/iot-pianoTimer/MicTest/MicTest.ino`。
3. 开发板选择 ESP32S3 Dev Module；Flash 16MB；Partition Scheme 16M Flash (3MB APP/9.9MB FATFS)；PSRAM OPI。
4. 原生 USB：USB Mode 为 Hardware CDC and JTAG，USB CDC On Boot 为 Enabled。若用 UART 转串口，可关闭 CDC 并用对应端口。
5. 选择正确端口并上传。保持 Erase All Flash Before Sketch Upload 为 Disabled，以便后续保留麦克风设置。
6. 串口监视器选 **921600 baud**（本诊断程序与 PianoTimer 的 115200 不同）。

## 版本不明时如何测试

程序默认从 I2S 开始，也会读取以前保存的模式。在串口监视器顶部输入框输入一个字母，再按发送/回车：

| 命令 | 操作 |
| --- | --- |
| `i` | 切换为标准 I2S 麦克风 |
| `p` | 切换为 PDM 麦克风 |
| `l` | 选择左声道用于录音和计时 |
| `r` | 选择右声道用于录音和计时 |
| `s` | 在人工验证后保存当前模式与声道 |
| `?` | 显示帮助 |

屏幕文字不用于这一阶段。串口大约每 0.5 秒输出两边声道的 `db`、`peak`、`clip`、`changes`。

1. 先输入 `i`，安静 5 秒，再说话 5 秒，再弹琴 5 秒。
2. 观察左右声道：有效声道的 db 和 peak 应随声音变化，clip 应接近 0，changes 不应长期为 0。
3. 如果没有正常响应，用 `p` 重做同样的测试。
4. 根据有效声道发送 `l` 或 `r`。
5. **不要因有非零数据就认为成功。必须导出一段 WAV，确认可以听清说话或钢琴，速度音调正常且没有持续数字噪声。** 错误接口格式也可能产生变化数据。
6. 听录音确认后，重新打开串口监视器，发送正确模式、声道和 `s`，应显示 `Saved. Ready for PianoTimer.`。

模式与声道保存到板子的 NVS。重新刷写 PianoTimer 时使用相同分区且不要整片擦除。换模式或声道后需重新验证并保存。

## 导出 5 秒 WAV

关闭 Arduino 串口监视器，避免占用端口。在 Mac 终端运行（端口需替换为实际值）：

```sh
cd /Users/leon/iot-pianoTimer
python3 tools/capture_mic.py --port /dev/cu.usbmodem13301 --mode i2s --channel left --output /private/tmp/piano-test.wav
```

PDM 则将 `--mode i2s` 改成 `--mode pdm`；右声道将 `left` 改成 `right`。脚本开始后立即发声，约 5 秒后结束。成功会输出 `Saved`。脚本不会自动执行 `s`。

脚本依赖 pyserial。若提示找不到 serial，可创建虚拟环境：

```sh
python3 -m venv /private/tmp/piano-mic-venv
/private/tmp/piano-mic-venv/bin/pip install pyserial
/private/tmp/piano-mic-venv/bin/python tools/capture_mic.py --port /dev/cu.usbmodem13301 --mode i2s --channel left --output /private/tmp/piano-test.wav
```

在 Finder 前往 `/private/tmp`，打开 `piano-test.wav` 听取。失败或数据不完整时脚本不会保存 WAV，请检查模式、串口速度和错误消息。

建议分别保存安静、纯讲话、连续弹琴、轻声弹琴和讲话同时弹琴的录音。录音属于你的本地数据，不要直接提交到公开仓库。

## 引脚依据与限制

I2S：SCK=42、WS=45、SD=46；PDM：CLK=45、DATA=46。16 kHz 采样，双声道诊断，单声道输出。

依据 [xiaozhi-esp32 的该板配置](https://github.com/78/xiaozhi-esp32/blob/main/main/boards/taiji-pi-s3/config.h) 与 [硬件批次说明](https://github.com/78/xiaozhi-esp32/blob/main/main/boards/taiji-pi-s3/README.md)。后者说明批次大于 2528 应用 PDM。这里仍必须靠实际录音验证，未自动认定你的板子版本。
