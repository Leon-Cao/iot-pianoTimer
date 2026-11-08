# JC3636W518C：Mac 上的 Arduino 安装与测试指南

适用设备：GUITION JC3636W518C（ESP32-S3、360×360 圆形触摸屏）。

目标：依次完成 Arduino 环境安装、屏幕与触摸测试、LVGL 界面测试。先完成每一步的检查，再继续下一步。

本指南根据下载的 `ESP32_Display_Panel_JC3636W518` 仓库整理。仓库的普通 Arduino 示例仍是通用模板，板型配置取自 `examples/PlatformIO/src`。这些配置需要通过实物测试确认；本指南不表示已经在你的板子上验证通过。

## 第 1 步：安装 Arduino IDE

1. 打开 [Arduino 官方下载页面](https://www.arduino.cc/en/software/)。
2. 下载 Arduino IDE 2 的 macOS 版本。如果页面区分芯片类型，点击苹果菜单 →“关于本机”：M 系列选择 Apple Silicon，Intel 选择 Intel。
3. 打开下载的 `.dmg` 文件。
4. 把 Arduino IDE 拖到“应用程序”文件夹。
5. 从“应用程序”打开 Arduino IDE。

**完成检查：** 能看到 Arduino IDE 的代码编辑窗口。

这条路线暂时不需要安装 VS Code、PlatformIO 或单独的 ESP-IDF。

## 第 2 步：设置 ESP32 开发板下载地址

1. 在屏幕顶部菜单点击 **Arduino IDE → Settings… / Preferences…（设置／偏好设置）**，通常也可以按 **⌘ + ,**。
2. 找到 **Additional boards manager URLs（附加开发板管理器网址）**。
3. 加入下面的地址。如果已有其他地址，使用旁边的列表按钮另起一行添加，保留原来的地址。

   ```text
   https://espressif.github.io/arduino-esp32/package_esp32_index.json
   ```

4. 点击 **OK（确定）**保存。

**完成检查：** 再次打开设置时，可以看到刚才添加的网址。

## 第 3 步：安装 ESP32 支持包

1. 点击 IDE 左侧的 **Boards Manager（开发板管理器）**，也可以从 **Tools → Board → Boards Manager** 打开。
2. 在搜索框中输入 `esp32`。
3. 找到发布者为 **Espressif Systems** 的 `esp32`。
4. 在版本下拉框中选择 **3.0.7**。
5. 点击 **Install（安装）**，等待下载和安装结束。
6. 按 **⌘ + Q** 完全退出 Arduino IDE，再重新打开。

**完成检查：** 开发板管理器中，该支持包显示已安装，版本为 `3.0.7`。

版本说明：仓库要求 `arduino-esp32 >= 3.0.0-alpha3`，没有锁定稳定版本。这里用 3.0.7 作为首次测试的固定版本，方便排查兼容问题；它不是仓库明确验证过的版本。

## 第 4 步：准备固定的工作文件夹

### ① 确认项目文件夹

1. 打开 **Finder（访达）**。
2. 按 **⌘ Command + Shift + G**，输入 `/Users/leon/iot-pianoTimer`，按回车。
3. 确认里面有 `PanelTest`、`libraries` 和本说明文件。当前项目已经迁移到此处，不用再创建一层工作文件夹。
4. 如果在另一台电脑使用，请先下载或克隆 `iot-pianoTimer` 仓库，并把本文中的 `/Users/leon/iot-pianoTimer` 替换成实际位置。

`libraries` 用于存放驱动库；`PanelTest` 是当前屏幕与触摸测试工程。

### ② 告诉 Arduino IDE 使用这个位置

1. 打开 Arduino IDE 的 **Settings / Preferences（设置／偏好设置）**。
2. 找到 **Sketchbook location（项目文件夹位置）**。
3. 将位置设置为：

   ```text
   /Users/leon/iot-pianoTimer
   ```

4. 点击 **OK（确定）**保存。

**完成检查：** Finder 中有下面的结构，IDE 设置中的项目文件夹指向 `iot-pianoTimer`，不是 `libraries`。

```text
iot-pianoTimer/
└── libraries/
```

## 第 5 步：把下载的显示库放进去，并确认 IDE 已识别

项目中已经包含显示库，先检查已有文件；只有库缺失时才需要重新复制。

1. 打开 Finder。
2. 按 **⌘ + Shift + G**，输入 `/Users/leon/iot-pianoTimer/libraries/ESP32_Display_Panel`，按回车。
3. 如果能打开该文件夹，直接继续第 8 项检查，不用重复安装。
4. 如果文件夹不存在，找到你下载并解压的 `ESP32_Display_Panel_JC3636W518` 文件夹，选中并按 **⌘ + C**。
5. 前往 `/Users/leon/iot-pianoTimer/libraries`。
6. 按 **⌘ + V** 粘贴。
7. 将粘贴进来的文件夹命名为 `ESP32_Display_Panel`。
8. 双击进入它，确认直接有 `library.properties`、`src`、`examples` 和 `README_CN.md`。如果只有另一个库文件夹，说明多套了一层，应把真正包含这些文件的文件夹放在 `libraries` 下，并命名为 `ESP32_Display_Panel`。
9. 在 Arduino IDE 中按 **⌘ + Q** 完全退出。
10. **重新打开 Arduino IDE，并按下面的方法完成检查：**
    - 点击 **Tools（工具）→ Board（开发板）→ esp32 → ESP32S3 Dev Module**。
    - 点击 **File（文件）→ Examples（示例）**。
    - 往菜单下方找 **ESP32_Display_Panel**，它可能位于“自定义库示例”一栏。
    - 鼠标移到它上面，应该能看到 `LCD`、`Panel`、`Touch`、`LVGL` 等示例分类。

**完成检查：** 能在 Examples 菜单中看到 `ESP32_Display_Panel` 及其示例，就可以继续第 6 步。

如果找不到，请检查两项：

- 库的位置应为 `/Users/leon/iot-pianoTimer/libraries/ESP32_Display_Panel`，进入后能直接看到 `library.properties`、`src`、`examples`。
- IDE 的 **Sketchbook location** 应为 `/Users/leon/iot-pianoTimer`，不要选择里面的 `libraries`。

修改后按 **⌘ + Q** 完全退出，再重新打开检查。

## 第 6 步：检查或安装 ESP32_IO_Expander 依赖库

项目已包含 `libraries/ESP32_IO_Expander`（0.0.4）。如果该文件夹中有 `library.properties` 和 `src`，可直接继续第 7 步；无需重复安装。缺失时按下面步骤安装。

1. 在 Arduino IDE 点击 **Tools（工具）→ Manage Libraries…（管理库）**，也可以点击左侧的 Library Manager 图标。
2. 搜索 `ESP32_IO_Expander`。
3. 找到名称完全相同、发布者为 Espressif 的库。
4. 在版本下拉框中选择以 **`0.0.`** 开头的版本。
5. 点击 **Install（安装）**。

当前显示库要求这个依赖满足 `>= 0.0.1 且 < 0.1.0`，不要选择 0.1.0 或更高版本。

**完成检查：** 显示 Installed（已安装），版本为 `0.0.x`。如果已经安装，确认版本即可。

## 第 7 步：复制并打开屏幕测试工程

“工程”就是一个文件夹，里面放着主程序和配置文件。

### ① 确认或复制整个测试工程

当前项目已经包含外层 `PanelTest`。如果 `/Users/leon/iot-pianoTimer/PanelTest/PanelTest.ino` 已存在，请跳过本小节的复制步骤，直接检查配置。不要用通用模板覆盖已经修正的工程。只有工程缺失时才按下面步骤复制。

1. 在 Finder 中按 **⌘ + Shift + G**，前往：

   ```text
   /Users/leon/iot-pianoTimer/libraries/ESP32_Display_Panel/examples/Panel
   ```

2. 选中 `PanelTest` 文件夹，按 **⌘ + C**。
3. 前往 `/Users/leon/iot-pianoTimer`。
4. 按 **⌘ + V**。

### ② 检查或替换为这块板的配置

如果工程已经采用下方核对示例中的 ST77916、360×360 配置，可以跳过复制。否则按下面步骤替换。

1. 在 Finder 中前往：

   ```text
   /Users/leon/iot-pianoTimer/libraries/ESP32_Display_Panel/examples/PlatformIO/src
   ```

2. 选中 `ESP_Panel_Board_Custom.h`，按 **⌘ + C**。
3. 前往：

   ```text
   /Users/leon/iot-pianoTimer/PanelTest
   ```

4. 按 **⌘ + V**。
5. 提示已有同名文件时，选择 **Replace（替换）**。

这里仅借用 PlatformIO 目录里的配置，不需要安装 PlatformIO。整份复制后，Custom 开关应为 `1`；工程中 `ESP_Panel_Board_Supported.h` 的 Supported 开关保持 `0`，两者不能同时启用。

**替换后的核对方法：** 打开工程后，点击 IDE 中的 `ESP_Panel_Board_Custom.h` 标签，确认能找到以下内容：

```cpp
#define ESP_PANEL_USE_CUSTOM_BOARD       (1)
#define ESP_PANEL_USE_LCD                (1)
#define ESP_PANEL_LCD_NAME               ST77916
#define ESP_PANEL_LCD_WIDTH              (360)
#define ESP_PANEL_LCD_HEIGHT             (360)
```

空格数量可以不同。如果仍看到 `ILI9341`、`320`、`240`，说明默认模板还没有被替换，请重新执行本节的复制和替换步骤。不要只把开关从 0 改成 1，因为引脚等配置也需要整份替换。两个开发板开关都为 0 时，会出现 `ESP_Panel does not name a type` 编译错误。

### ③ 打开工程并检查文件

1. 打开 Arduino IDE，在 Mac 屏幕顶部点击 **File（文件）→ Open…（打开）**，也可以按 **⌘ + O**。
2. 弹出文件选择窗口后，按 **⌘ + Shift + G**。
3. 粘贴下面的文件夹地址，按回车：

   ```text
   /Users/leon/iot-pianoTimer/PanelTest
   ```

4. 单击列表中的 **PanelTest.ino**，再点击右下角 **Open（打开）**。
5. 确认打开的是 `/Users/leon/iot-pianoTimer/PanelTest/PanelTest.ino`。如果目录中还有一个 `PanelTest` 子文件夹，不要进入它：`PanelTest/PanelTest/` 是嵌套旧工程。

`.ino` 是 Arduino 主程序文件。你只需要打开它，IDE 会自动识别同一个文件夹中的 `.h` 配置文件。

“保留整个文件夹”的意思是：**主程序与配置文件要继续放在一起，不要只把 `.ino` 单独拿出来。** 文件夹应包含：

```text
PanelTest/
├── PanelTest.ino                 ← 主程序
├── ESP_Panel_Board_Custom.h      ← 屏幕和引脚配置
├── ESP_Panel_Board_Supported.h   ← 开发板选择配置
├── ESP_Panel_Conf.h              ← 驱动配置
└── README.md                    ← 使用说明
```

**完成检查：** IDE 中出现 `PanelTest.ino` 及几个 `.h` 标签。如果你复制了整个文件夹并替换了指定配置文件，就已做对，不需要逐个打开 `.h` 文件。

外层 `iot-pianoTimer` 是整个项目和 Arduino 项目文件夹的名字；`PanelTest` 是其中的测试工程。未来正式计时器可以新建 `PianoTimer/PianoTimer.ino`，但它不是当前测试入口，现在不用创建。Arduino 工程文件夹与主 `.ino` 文件应同名，因此保留 `PanelTest/PanelTest.ino`。

## 第 8 步：选择开发板和编译参数

1. 点击 **Tools → Board → esp32 → ESP32S3 Dev Module**。
2. 再次打开 **Tools（工具）**，依次设置：

| 菜单项 | 选择值 |
| --- | --- |
| CPU Frequency | 240MHz |
| Flash Size | 16MB (128Mb) |
| Flash Mode | QIO 80MHz |
| PSRAM | OPI PSRAM |
| Partition Scheme | 以 16M Flash 开头的方案，例如 3MB APP/9.9MB FATFS |
| Upload Speed | 115200 |
| Erase All Flash Before Sketch Upload | Disabled |

使用开发板原生 USB 接口时，再设置：

| 菜单项 | 选择值 |
| --- | --- |
| USB Mode | Hardware CDC and JTAG |
| USB CDC On Boot | Enabled |

如果接口明确标记为 UART，或确认连接经过 USB 转串口芯片，将 `USB CDC On Boot` 设为 `Disabled`。如果无法判断，保留接口信息以便排查，不必自行安装驱动。

这些是依据硬件规格与仓库配置选定的初始参数；其他菜单项先保持默认。

**完成检查：** 开发板为 ESP32S3 Dev Module，Flash 为 16MB，PSRAM 为 OPI。如果看不到这些菜单项，先确认开发板是否选对。

## 第 9 步：先验证编译

这一步不需要连接开发板，也不会修改设备。

1. 点击 Arduino IDE 左上角的 **✓ Verify（验证）**。
2. 等待底部输出完成，第一次可能需要几分钟。
3. 成功时会显示程序占用空间等信息，不会出现编译失败提示。

“编译”是把代码转换成 ESP32 能执行的程序。

**完成检查：** 编译成功，再继续上传。

| 错误信息或现象 | 优先检查 |
| --- | --- |
| 找不到 ESP_Panel_Library.h | 第 5 步库的安装位置和文件夹层级 |
| 找不到 IO Expander 相关头文件 | 第 6 步依赖是否安装 |
| 没有启用开发板配置 | 第 7 步是否替换了 Custom 配置文件 |
| 配置版本不兼容 | 是否混用了其他版本库或配置文件 |
| Multiple libraries were found | 查看实际使用的库路径，确认采用本次下载的分支 |

如果失败，可点击 **Copy error messages（复制错误信息）**，或复制从第一条 `error:` 开始的内容用于排查。不要只看最后一句 `exit status 1`，真正原因通常在它前面。

## 第 10 步：连接、上传，并检查屏幕和触摸

上传会替换设备当前的出厂 Demo。如果需要恢复原来的界面，应先取得对应的恢复固件或完成备份。

### ① 连接并选择端口

1. 用支持数据传输的 USB 线连接 Mac 与开发板，只有充电功能的线不行。
2. macOS 如果提示是否允许配件连接，点击允许。
3. 点击 **Tools → Port（端口）**。
4. 选择新出现的端口，通常形如 `/dev/cu.usbmodem…` 或 `/dev/cu.usbserial…`。
5. 不确定时，先拔掉线查看列表，再插上线查看，新出现的端口通常就是开发板。

如果没有新端口，先检查数据线、转接器与 USB 接口。

### ② 上传程序

1. 点击左上角的 **→ Upload（上传）**。
2. 等待编译和上传结束，期间保持 USB 连接。
3. 如果显示 `Done uploading`，或最后出现类似 `Hard resetting via RTS pin...` 且无上传错误，通常表示上传完成。
4. 如果显示 `Failed uploading`，先处理上传错误。

如果一直停在 `Connecting...`，且板上有明确标记的 BOOT 和 RESET 键，可按住 BOOT、短按 RESET、松开 BOOT，再重新选择端口并上传。

### ③ 检查显示

1. 上传完成后等待几秒。
2. 观察开发板屏幕，正常应显示蓝、绿、红三个颜色区域。
3. 如果仍显示原来的画面，短按一次 RESET，再观察。

黑屏、白屏、花屏或始终停留在原来的界面，都还需要排查。

### ④ 检查触摸

1. 点击 **Tools → Serial Monitor（串口监视器）**。
2. 在下方消息窗口的波特率下拉框中选择 **115200 baud**。
3. 如果没有消息，短按一次 RESET。
4. 正常会看到类似输出：

   ```text
   Panel test example start
   Initialize display panel
   ...
   Panel test example end
   ```

5. 保持串口监视器打开，用手指点击屏幕中央、左边、右边。
6. 正常会出现类似：

   ```text
   Touch point(0): x 120, y 180, strength 0
   Touch point(0): x 60, y 175, strength 0
   Touch point(0): x 280, y 190, strength 0
   ```

数字不必与示例相同。不同位置的 `x`、`y` 会变化，就说明触摸有响应；`strength` 为 0 不一定异常。

这个程序只显示彩条并打印坐标，**点击后屏幕画面不变化是正常的**。

| 检查结果 | 下一步 |
| --- | --- |
| 有彩条，点击时有坐标 | 检查通过，继续第 11 步 |
| 有彩条，但没有触摸坐标 | 显示通过，继续排查触摸 |
| 黑屏或花屏 | 排查显示初始化与配置 |
| 串口完全无消息 | 检查端口、115200 波特率及 USB 设置 |

## 第 11 步：安装 LVGL 图形界面库

等屏幕彩条和触摸测试通过后，再进行本步骤。

1. 点击 **Tools → Manage Libraries…**。
2. 搜索 `lvgl`。
3. 找到名称为 `lvgl` 的库。
4. 选择版本 **8.3.11**。
5. 点击 **Install（安装）**。

这份示例要求 `>= 8.3.9 且 < 9`，不要选择 9.x。

**完成检查：** 库管理器显示 lvgl 8.3.11 已安装。

## 第 12 步：放好 LVGL 配置文件

1. 在 Finder 中前往：

   ```text
   /Users/leon/iot-pianoTimer/libraries/ESP32_Display_Panel/examples/PlatformIO/src
   ```

2. 选中 `lv_conf.h`，按 **⌘ + C**。
3. 前往：

   ```text
   /Users/leon/iot-pianoTimer/libraries
   ```

4. 按 **⌘ + V**。

**注意：** `lv_conf.h` 放在 `libraries` 里面，与 `lvgl` 文件夹同级，不要放进 `lvgl` 文件夹里面。

**完成检查：** 结构应类似下面这样，库管理器安装的依赖文件夹名称可能略有不同：

```text
iot-pianoTimer/
├── libraries/
│   ├── ESP32_Display_Panel/
│   ├── ESP32_IO_Expander/
│   ├── lvgl/
│   └── lv_conf.h
└── PanelTest/
```

## 第 13 步：复制并运行 LVGL 测试工程

### ① 复制工程

1. 在 Finder 中前往：

   ```text
   /Users/leon/iot-pianoTimer/libraries/ESP32_Display_Panel/examples/LVGL/v8
   ```

2. 复制整个 `Porting` 文件夹。
3. 粘贴到 `/Users/leon/iot-pianoTimer`。

### ② 复制已经用于屏幕测试的配置

1. 前往 `/Users/leon/iot-pianoTimer/PanelTest`。
2. 复制 `ESP_Panel_Board_Custom.h`。
3. 粘贴到 `/Users/leon/iot-pianoTimer/Porting`。
4. 遇到同名文件提示时，选择 **Replace（替换）**。

### ③ 编译并上传

1. 在 Arduino IDE 点击 **File → Open…**，打开：

   ```text
   /Users/leon/iot-pianoTimer/Porting/Porting.ino
   ```

2. 确认开发板仍为 ESP32S3 Dev Module，参数与第 8 步相同。
3. 其余代码先保持原样，特别是 `lvgl_port_v8.h` 中的下面设置应保持为 0：

   ```cpp
   #define LVGL_PORT_AVOID_TEARING_MODE (0)
   ```

4. 点击 **✓** 编译。
5. 编译成功后，选择开发板端口，点击 **→** 上传。

**完成检查：** 屏幕中央显示 `LVGL porting example`。这个示例主要显示文字，没有按钮是正常情况。

通过后，编译环境、显示、触摸和基础 LVGL 界面已完成测试，可以在此基础上制作 PianoTimer。

## 参考资料

- [Arduino 官方安装说明](https://support.arduino.cc/hc/en-us/articles/360019833020-Download-and-install-Arduino-IDE)
- [乐鑫 Arduino-ESP32 安装说明](https://docs.espressif.com/projects/arduino-esp32/en/latest/installing.html)
- [本地仓库 README_CN.md](/Users/leon/iot-pianoTimer/libraries/ESP32_Display_Panel/README_CN.md)
- [本地板型配置](/Users/leon/iot-pianoTimer/libraries/ESP32_Display_Panel/examples/PlatformIO/src/ESP_Panel_Board_Custom.h)
- [本地 PanelTest 说明](/Users/leon/iot-pianoTimer/libraries/ESP32_Display_Panel/examples/Panel/PanelTest/README.md)
- [本地 LVGL Porting 说明](/Users/leon/iot-pianoTimer/libraries/ESP32_Display_Panel/examples/LVGL/v8/Porting/README.md)
