这是一个基于 Arduino 的开源卫星/天线地面追踪站云台控制器。项目采用 EasyComm II 协议，通过 rotctld 与上游追踪软件无缝对接。

---
## 硬件架构与引脚映射

本系统设计支持两轴（方位角 Azimuth / 俯仰角 Elevation）独立闭环控制。

| 功能分类 | 引脚名称 | Arduino 物理引脚 | 备注说明 |
| :--- | :--- | :--- | :--- |
| **方位角轴 (AZ)** | `AZ_EN_PIN` | D3 | 电机驱动板使能引脚 (Enable) |
| | `AZ_DIR_PIN` | D4 | 电机方向引脚 (Direction) |
| | `AZ_STEP_PIN` | D5 | 电机脉冲引脚 (Step/Pulse) |
| | `AZ_ENC_PIN` | A2 | 方位角绝对值 PWM 编码器输入 |
| **俯仰角轴 (EL)** | `EL_EN_PIN` | D8 | 电机驱动板使能引脚 (Enable) |
| | `EL_DIR_PIN` | D7 | 电机方向引脚 (Direction) |
| | `EL_STEP_PIN` | D6 | 电机脉冲引脚 (Step/Pulse) |
| | `EL_ENC_PIN` | A1 | 俯仰角绝对值 PWM 编码器输入 |
| **通信接口** | `Hardware Serial` | D0 (RX) / D1 (TX) | 绑定电脑端，速率固定为 **9600 bps** |


### 1. 克隆与编译
1. 将本项目源码克隆至本地，并确保你的 Arduino 常用库路径下包含外部的 `easycomm.h` 库。
2. 使用 Arduino IDE 或 VS Code (PlatformIO) 打开项目。
3. 根据你的步进电机驱动器（如 A4988、TB6600 等）的细分设置，在代码中调整脉冲半周期参数：
   ```cpp
   const unsigned long STEP_INTERVAL_US = 800; // 数值越小，电机转速越快
4. 编译并烧录至你的 Arduino 开发板。

2. 电脑端连接测试 (Hamlib)
打开终端或命令提示符，直接调用 rotctld（以 Hamlib 4.7.1 为例）进行高强度通信测试：
    ```Bash
    rotctld -m 202 -r COM3 -s 9600 -T 127.0.0.1 -t 4533 -C timeout=5000 -C retry=0 -vvvvvvvv
参数说明：

-m 202：指定使用 EasycommII 协议模型。

-r COM3：修改为你的 Arduino 实际占用的串口号。

-s 9600：严格匹配波特率 9600。

-vvvvvvvv：开启最高级别调试日志，可实时观测代理机制的拦截与应答。