# 本地逻辑测试

从仓库根目录运行：

```sh
c++ -std=c++17 -Wall -Wextra -fsanitize=address,undefined -I libraries/PianoAudio/src tests/test_audio.cpp -o /private/tmp/piano-audio-tests
/private/tmp/piano-audio-tests
```

覆盖 1 秒等号边界、超过 1 秒、等待期间 Reset、长停顿、周期性单音、重复区间、丢帧、120 小时累计；合成静音、白噪声、纯正弦和衰减谐波音只用于检测算法冒烟测试，不代表真实钢琴/人声准确率。
