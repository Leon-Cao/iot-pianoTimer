# PianoTimer 开发进度与续接记录

记录日期：2026-11-08（Asia/Tokyo）。用户决定今天停止测试，明天再继续。本次仅记录进度，不设置提醒、不继续采音或烧录。

## 项目与目标

- GitHub 仓库名称：`iot-pianoTimer`。
- 本地目录：`/Users/leon/iot-pianoTimer`，也是 Arduino IDE 的项目文件夹位置。
- 硬件：GUITION JC3636W518C，ESP32-S3，360×360 圆形触摸屏；不是此前误参考的微雪 ESP32-S3-Touch-LCD-1.85。
- 使用真钢琴。第一版只用麦克风，不加振动传感器。
- 屏幕显示累计有效弹琴时间 `HH:MM:SS`，提供触摸 Reset。
- 钢琴声音及可识别余音计入；人声、环境噪声不应单独触发计时；钢琴与人声同时存在时仍应统计钢琴活动。
- 无声间隔 ≤1 秒且之后再次检测到钢琴，事后补计该短间隔；超过 1 秒时整段静音都排除，包括最初等待的 1 秒。
- 不在每次检测后固定赠送计时时间，避免稀疏敲键把长停顿连成整段练习。
- Reset 清零累计时间与待补计状态，随后继续监听。

## 已完成

### 环境与显示工程

- 已整理安装指南 `install_readme.md`，包含 1～13 步和新目录路径。
- 第 5 步包含通过 Arduino Examples 菜单确认库已识别的方法。
- 第 7 步详细说明打开 `PanelTest/PanelTest.ino`、保留整个工程文件夹及检查板型配置。
- 修复过 `ESP_Panel does not name a type`：当时工程仍是关闭 Custom 开关的 ILI9341、320×240 默认模板，已替换为仓库内 ST77916、360×360 配置并编译通过。
- `PanelTest` 与 `Porting` 原有工程保留。
- 早期安装指南建议 ESP32 支持包 3.0.7，但实际开发时电脑安装的是 **3.3.12**，第一版按实际版本编译验证。

### 第一版新增代码

| 文件/目录 | 用途 |
| --- | --- |
| `MicTest/MicTest.ino` | I2S/PDM、左右声道诊断，分贝/峰值/削波指标，录音导出，保存模式 |
| `MicTest/README.md` | 麦克风验证操作说明 |
| `PianoTimer/PianoTimer.ino` | 计时显示、Reset、背景噪声校准、音频检测流程 |
| `PianoTimer/README.md` | 使用步骤、识别限制、实机验收清单 |
| `libraries/PianoAudio/src/PianoMic.h` | 麦克风采集与模式存储 |
| `libraries/PianoAudio/src/PianoDetector.h` | 实验性频谱特征判别 |
| `libraries/PianoAudio/src/PracticeTimer.h` | 1 秒短停顿补计逻辑 |
| `tools/capture_mic.py` | 从串口导出 5 秒、16kHz、单声道 WAV |
| `tests/test_audio.cpp` | 计时边界与合成信号测试 |
| `tests/VALIDATION.md` | 软件验证结果与尚未验证事项 |

### 软件验证

- 开发环境：Arduino-ESP32 3.3.12、ESP32_Display_Panel 0.1.7、ESP32_IO_Expander 0.0.4、LVGL 8.3.11。
- MicTest 编译通过：356096 bytes Flash，47440 bytes 静态 RAM。
- PianoTimer 编译通过：669475 bytes Flash，115256 bytes 静态 RAM。
- C++ 主机测试通过，含 AddressSanitizer / UndefinedBehaviorSanitizer。
- 覆盖短停顿、长停顿、1 秒边界、Reset、周期性单音、重复区间、丢帧边界、长时长和合成音频冒烟测试。
- 保存后的代码与编译时使用的代码核对一致。

## 麦克风最新实测进度

用户已运行 MicTest，并报告：

- 输入 `i` 使用 **I2S** 模式时，`left_db` 随声音变化，声音越大数值越大。
- `right_db` 一直是 `-120`。
- 因此下一步使用 **I2S + 左声道**。

这已确认左声道对声音有响应，但**尚未完成 WAV 听检**，不能仅据此认定采样速度、格式、音调完全正常。

尚未确认用户是否已发送 `s` 保存配置。后续必须核实，不能假定已经保存。

## 今天停在哪里

用户运行以下命令时失败：

```sh
python3 tools/capture_mic.py --port /dev/cu.usbmodem13301 --mode i2s --channel left --output /private/tmp/piano-test.wav
```

错误：

```text
[Errno 16] Resource busy: '/dev/cu.usbmodem13301'
```

已用 `lsof /dev/cu.usbmodem13301` 查到占用进程：`serial-mo`，当时 PID 为 `94808`，即串口监视器进程。PID 和端口号可能在明天变化，不能直接沿用 PID 结束进程。

已告知用户：在 Arduino IDE 按 **⌘ + Q** 完全退出，保留 USB 连接，再运行录音脚本。只切换窗口不会释放串口。

**用户随后决定停止，尚未确认重试成功，没有确认生成有效的 WAV 文件。** 不要将此次录音标记为成功，也不要声称已听取录音或识别效果通过。

## 明天从这里继续

### 1. 完成 WAV 听检

1. 保持开发板运行 MicTest，不需要为串口占用问题重新烧录。
2. 完全退出 Arduino IDE，关闭其他占用串口的程序。
3. 确认当前端口。原来是 `/dev/cu.usbmodem13301`；如有变化，替换下面命令。
4. 在终端执行：

   ```sh
   cd /Users/leon/iot-pianoTimer
   python3 tools/capture_mic.py --port /dev/cu.usbmodem13301 --mode i2s --channel left --output /private/tmp/piano-test.wav
   ```

5. 执行后立即说话或弹琴，等待约 5 秒采音及保存完成。
6. 在 Finder 按 ⌘⇧G，前往 `/private/tmp`，打开 `piano-test.wav`。
7. 检查声音清楚、音调和速度正常、没有持续数字噪声。若仍提示串口占用，重新用 `lsof` 查占用者。

### 2. 保存 I2S 左声道配置

听检通过后：

1. 打开 Arduino IDE 的串口监视器，波特率 **921600**。
2. 依次发送 `i`、`l`、`s`，其中 `l` 是小写 L。
3. 确认输出 `Saved. Ready for PianoTimer.`。
4. 保持同一分区方案，烧录时不要整片擦除，否则保存的配置可能丢失。

### 3. 首次运行 PianoTimer

1. 打开 `/Users/leon/iot-pianoTimer/PianoTimer/PianoTimer.ino`。
2. 与 MicTest 使用相同的板型、分区和 USB 设置；Flash 16MB，PSRAM OPI。
3. `Erase All Flash Before Sketch Upload` 为 Disabled。
4. 编译并上传，保持环境安静约 3 秒进行背景校准。
5. PianoTimer 的串口波特率为 **115200**，与 MicTest 的 921600 不同。
6. 检查屏幕时间、状态、音量条与 Reset。

### 4. 识别与计时实测

- 安静和单独讲话时不应累计。
- 连续弹奏、轻声弹奏、低音和人声混合场景检查是否漏计。
- 短停约 0.5 秒后继续：间隔应补计。
- 停 3 秒后继续：停顿全部不计。
- 每 5 秒弹一个音：不能把全部间隔连起来；真实余音仍可计入。
- 等待补计期间按 Reset：不能把清零前的间隔补回来。
- 收集对应日志与实录，以误计时间、漏计时间评估效果。

## 重要限制与后续开发目标

- 当前采用 Hann 窗、FFT、噪声门限、谱平坦度、多峰结构、起音和频谱稳定性等**实验性特征规则**，不是训练好的声音分类模型。
- **可靠排除人声和噪声的最终要求尚未达成验证。** 人声或其他谐波声音可能误触发，轻声/低音/混合声音可能漏检。
- 单次起音超过约 8 秒的长余音可能被停止判为候选。
- 音频按 128ms 片段分析，存在判断延迟和时间边界误差；逻辑层精确的 1 秒阈值不等于实际识别能做到毫秒精度。
- 下一阶段基于本板实录评估，必要时训练小型声音分类模型。不能将编译成功或合成信号测试通过描述成实物识别准确。
- 断电不保留本次累计时长；仅麦克风配置持久保存。
- 当前尚未收到 PianoTimer 烧录运行、界面和 Reset 实测成功的反馈。

## Git 状态

本次检查时：`README.md` 已修改；`.gitignore`、`MicTest/`、`PianoTimer/`、`libraries/PianoAudio/`、`tests/`、`tools/` 为未跟踪的新内容，尚未由本次开发提交或推送。

另外 `.DS_Store` 已被 Git 跟踪并显示修改；新增 ignore 不会自动取消已有跟踪。后续提交时不要把这个系统文件的变化混入功能提交。

本进度记录也应在准备提交时一并检查。不要上传私人录音到公开仓库。


## 2026-11-11 更新：双核 CPU 占用率

- 用户要求界面显示两个核心各自的 CPU 占用率。
- 已在 PianoTimer 的音量条与 Reset 之间加入 `CPU0 xx%    CPU1 xx%`，每秒刷新一次。
- 使用 Arduino-ESP32 3.3.12 已启用的 FreeRTOS 运行时间统计，分别读取两核 idle 任务的运行时间差；每核一个轻量采样任务保证空闲统计已刷新。系统调度统计为近似负载，不是独立中断耗时剖析。
- UI 使用 LVGL 定时器，麦克风未配置、暂停、校准或采集错误时仍可更新。Reset 不影响 CPU 统计。
- 首个采样窗口完成前、统计不可用或读数过期时显示 `--%`。
- 代码：`PianoTimer/CpuLoadMonitor.h`、`CpuLoadMonitor.cpp`、`CpuLoadMath.h`；测试：`tests/test_cpu_load.cpp`。
- 编译通过：670739 bytes Flash，115288 bytes 静态 RAM；采样任务还需动态分配每核 2048 bytes 栈及任务控制块。
- 百分比边界、回绕和延迟窗口的主机测试通过（AddressSanitizer / UndefinedBehaviorSanitizer）。
- 本次未烧录设备，实物显示与负载变化尚未验证。麦克风 WAV 听检和声音识别实测仍按上文继续。


## 2026-11-11 更新：屏幕按钮离线录音

- 用户确认 I2S 模式下左声道随音量变化、右声道始终 -120；WAV 听检仍未完成。
- 用户说明钢琴旁不能通过串口下命令，并确认希望使用内置存储。
- 澄清原来的录音是麦克风听检，不是钢琴/人声标定；当前模型不会从单次录音自动学习。
- PianoTimer 增加 Record，与 Reset 并排：倒数准备 3 秒、录音 5 秒到 PSRAM、停止采样后保存 FFat，并逐字节读回校验，成功显示 Saved 和编号。
- 每次生成独立的 piano-0001.wav 等文件，不覆盖前次录音；保存后正常断电仍可保留。尚未实机测试持久性。
- 录音/保存/导出期间练琴计时暂停并打断待补计间隔，CPU 显示保留。
- 存储无法挂载时先弹出清除数据区的确认；只有用户点击 Erase & init 才允许初始化，且检查为指定的 16MB/3MB APP 分区布局。
- 未保存麦克风配置也能按 Record，用已确认有响应的 I2S 左声道录音；计时先禁用，回电脑听检后在 PianoTimer 串口115200发送 verify，保存配置并重启。
- 新增 tools/export_recording.py：通过 USB 提取已保存文件，验证 CRC/WAV 格式；不重新录音、不删除设备文件。运行时需关闭 Arduino 串口监视器。
- 操作指南：PianoTimer/RECORDING.md。现在下一步应上传新版 PianoTimer，现场按 Record，回到电脑导出听检；不再要求在钢琴旁操作串口。
- 固件编译通过：729747 bytes Flash，115680 bytes 静态 RAM；录音临时使用160000 bytes PSRAM。
- WAV 头测试、导出成功与4项错误路径测试通过。尚未烧录本次改动、未实测录音按钮与真实Flash读写。


## 2026-11-11 更新：弹琴不计时排查

- 用户反馈弹琴时不计时；尚未确认屏幕具体状态和录音类别。
- 本地已有 piano-0001.wav、piano-0002.wav、piano-test.wav；前两者PCM内容完全相同，不能当成独立样本。已询问用户各文件是琴声还是人声，等待确认。
- 发现校准易受启动瞬态影响：0001文件开头片段约-23.8dB，若用原算法将前24帧当背景，门限被抬到约-25.4dB，而后续音频多在-45dB附近，足以全部拒绝。这是回放演示，不代表已获取设备当时实际校准日志。
- 修复：跳过前4帧启动期，以后续24帧的下四分位数估计背景；保持原有琴声特征条件，不直接放宽人声过滤。校准总时长约3.6秒，需安静约4秒。
- 界面显示未计时原因与音量门限，方便区分未启用、音量不足、削波、特征不符或缺少稳定片段。
- 校准回归测试通过；Arduino-ESP32 3.3.12编译通过：728719 bytes Flash、115776 bytes静态RAM。
- 尚未烧录/实机验证修正；下一步重新上传，安静完成校准，观察弹琴时状态，并确认录音类别。最终人声排除效果仍待实测。


## 2026-11-11 范围收敛：先实现计时器

用户参考TECHNICAL_REVIEW.md，决定将计时与琴谱比对拆为独立需求，当前先完成计时器、暂不考虑琴谱比对。

- TECHNICAL_PLAN.md改为计时器专用方案；FUTURE_SCORE_SCOPE.md独立保留暂缓需求。
- 当前只判断是否包含钢琴，不要求完整88键/和弦转录。人声排除和钢琴混合人声时继续计时仍属于第一阶段。
- 默认16kHz，先用真实录音离线比较FFT、Goertzel/多速率滤波组及小模型，再按S3实测决定。
- 补充候选有界确认、累计只增不减、噪声冻结退出和工程验收；逐键自动频漂与完整低音精查不作为交付前提。
- 修正review事实：现有录音先缓存PSRAM，采集结束后会写Flash并读回验证。
- 本次只改技术文档，没有修改固件或烧录设备。
