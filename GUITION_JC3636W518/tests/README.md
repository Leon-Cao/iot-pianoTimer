# 本地逻辑测试

从仓库根目录运行：

```sh
c++ -std=c++17 -Wall -Wextra -fsanitize=address,undefined -I libraries/PianoAudio/src tests/test_audio.cpp -o /private/tmp/piano-audio-tests
/private/tmp/piano-audio-tests
```

覆盖 1 秒等号边界、超过 1 秒、等待期间 Reset、长停顿、周期性单音、重复区间、丢帧、120 小时累计；合成静音、白噪声、纯正弦和衰减谐波音只用于检测算法冒烟测试，不代表真实钢琴/人声准确率。


## 双核占用率计算

```sh
c++ -std=c++17 -Wall -Wextra -fsanitize=address,undefined -I PianoTimer tests/test_cpu_load.cpp -o /private/tmp/piano-cpu-tests
/private/tmp/piano-cpu-tests
```

覆盖完全空闲、完全忙碌、部分负载、延迟采样窗口、32 位计数器回绕、快照误差截断与无效时间窗口。实际采样任务和屏幕显示仍需烧录后验证。


## 离线录音与导出格式

```sh
PYTHONPYCACHEPREFIX=/private/tmp/piano-pycache python3 tests/test_recording.py
c++ -std=c++17 -Wall -Wextra -fsanitize=address,undefined -I PianoTimer tests/test_wav.cpp -o /private/tmp/piano-wav-tests
/private/tmp/piano-wav-tests
```

验证 WAV 头和长度、导出成功、CRC 损坏、传输截断、文件缺失和异常尺寸。触摸、Flash 写入/断电持久性和实际声音仍需设备验证。


## 背景校准回归测试

```sh
c++ -std=c++17 -Wall -Wextra -fsanitize=address,undefined -I PianoTimer tests/test_calibration.cpp -o /private/tmp/piano-calibration-test
/private/tmp/piano-calibration-test
```

覆盖启动瞬态、偶发大声干扰、重置和非有限输入。
