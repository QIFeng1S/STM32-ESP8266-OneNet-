# STM32-ESP8266-OneNet- 🌡️

基于 STM32F1 + ESP8266 + DHT22 的物联网温湿度监测系统，通过 MQTT 协议连接中国移动 OneNET 云平台实现数据上报与远程监控。

## 📋 项目简介

本项目是一个完整的嵌入式物联网解决方案，实现了温湿度数据的采集、本地显示和云端上报功能。系统使用 STM32F103 作为主控芯片，通过 DHT22 传感器采集环境数据，使用 ESP01S（ESP8266）WiFi 模块连接互联网，并将数据上传至 OneNET 物联网平台。

### ✨ 核心功能

- 🌡️ **实时温湿度监测**：使用 DHT22 高精度传感器采集环境数据
- 📡 **WiFi 联网**：通过 ESP8266 模块连接无线网络
- ☁️ **云平台对接**：MQTT 协议连接 OneNET 物联网平台
- 🖥️ **本地显示**：OLED 屏幕实时显示温湿度及网络状态
- 🔄 **自动重连**：网络异常时自动尝试重新连接
- 📊 **数据上报**：每 2 秒上传一次数据到云端

## 🛠️ 硬件清单

| 硬件模块 | 型号/说明 | 数量 |
|---------|----------|------|
| 主控芯片 | STM32F103 系列 | 1 |
| WiFi 模块 | ESP01S (ESP8266) | 1 |
| 温湿度传感器 | DHT22 (AM2302) | 1 |
| 显示屏 | OLED 128x64 (I2C) | 1 |
| 其他 | 杜邦线、面包板等 | 若干 |

## 📐 硬件连接

### ESP8266 连接（USART2）
```
ESP8266 TX  -> STM32 RX (USART2)
ESP8266 RX  -> STM32 TX (USART2)
ESP8266 RST -> STM32 PA0 (GPIO 控制复位)
ESP8266 VCC -> 3.3V
ESP8266 GND -> GND
```

### DHT22 连接
```
DHT22 DATA -> STM32 GPIO (具体引脚见 DHT22.c)
DHT22 VCC  -> 3.3V/5V
DHT22 GND  -> GND
```

### OLED 连接（I2C）
```
OLED SCL -> STM32 I2C SCL
OLED SDA -> STM32 I2C SDA
OLED VCC -> 3.3V
OLED GND -> GND
```

### 指示灯（可选）
```
LED -> STM32 PC13 (板载 LED，发布成功时翻转)
```

## 🔧 软件配置

### 1. WiFi 配置

在 `ESP01s.h` 文件中修改以下参数：

```c
#define WIFI_SSID       "你的WiFi名称"
#define WIFI_PASSWORD   "你的WiFi密码"
```

### 2. OneNET 平台配置

#### 登录 OneNET 平台
1. 访问 [OneNET 官网](https://open.iot.10086.cn/)
2. 注册/登录账号

#### 创建产品
1. 进入"多协议接入" -> "MQTT 物模型"
2. 创建新产品，获取 **产品 ID**

#### 创建设备
1. 在产品下创建设备，获取 **设备名称**

#### 生成鉴权信息
1. 使用 [OneNET 鉴权工具](https://open.iot.10086.cn/doc/mqtt/book/manual/auth/tool.html) 生成 Token
2. 将生成的 Token 填入配置文件

#### 修改配置文件

在 `ESP01s.h` 中修改：

```c
#define PRODUCT_ID      "你的产品ID"
#define DEVICE_NAME     "你的设备名称"
#define API_KEY         "你生成的Token"
```

#### 定义物模型

在 OneNET 平台定义以下数据点（物模型功能点）：

| 标识符 | 数据类型 | 说明 |
|--------|---------|------|
| temp   | int32   | 温度值（℃） |
| humi   | int32   | 湿度值（%RH） |
| led    | bool    | LED 状态 |

### 3. MQTT 主题配置

在 `main.c` 中修改发布主题（替换产品 ID 和设备名）：

```c
ESP8266_MQTT_PubRaw("$sys/你的产品ID/你的设备名/thing/property/post", json_buf)
```

在 `ESP01s.c` 中修改订阅主题（用于接收平台回复）：

```c
ESP8266_MQTT_Sub("$sys/你的产品ID/你的设备名/thing/property/post/reply", 0)
```

## 📂 项目结构

```
STM32-ESP8266-OneNet-/
├── main.c                  # 主程序入口
├── ESP01s. c / ESP01s.h     # ESP8266 驱动及 MQTT 协议
├── DHT22.c / DHT22.h       # DHT22 温湿度传感器驱动
├── OLED.c / OLED.h         # OLED 显示驱动
├── OLED_data.c / OLED_data.h # OLED 字库数据
├── My_I2C.c / My_I2C.h     # 软件 I2C 实现
├── Delay.c / Delay.h       # 微秒级延时函数
├── usart. c                 # 串口初始化
├── gpio.c                  # GPIO 初始化
├── stm32f1xx_it.c          # 中断处理函数
├── stm32f1xx_hal_msp.c     # HAL 库底层初始化
└── system_stm32f1xx.c      # 系统时钟配置
```

## 🚀 编译与烧录

### 环境要求
- **开发环境**：Keil MDK-ARM 或 STM32CubeIDE
- **HAL 库**：STM32F1xx HAL Driver

### 编译步骤
1. 用 Keil 或 CubeIDE 打开项目
2. 检查芯片型号配置（STM32F103）
3. 编译生成 .hex 或 .bin 文件

### 烧录步骤
1. 使用 ST-Link 或 J-Link 连接开发板
2. 通过 ST-Link Utility 或 Keil 直接下载程序
3. 复位开发板，观察 OLED 显示

## 📊 运行流程

### 启动流程
```
系统初始化
    ↓
OLED 显示 "System Init..."
    ↓
ESP8266 硬复位 + 软复位
    ↓
连接 WiFi -> 显示 "WiFi:  OK"
    ↓
配置 MQTT 用户信息 -> 显示 "User:  OK"
    ↓
连接 MQTT 服务器 -> 显示 "Conn: OK"
    ↓
订阅回复主题 -> 显示 "Sub: OK"
    ↓
显示 "NetInit OK!" 并进入主循环
```

### 主循环逻辑
```
采集温湿度数据（DHT22）
    ↓
OLED 显示 "T: 25 C  H:60 %RH"
    ↓
封装 JSON 数据
    ↓
通过 MQTT 发布到 OneNET
    ↓
发布成功？
    ├─ 是：显示 "Pub OK!  Pub Times: X" + 翻转 LED
    └─ 否：显示 "Pub:  Fail" -> 尝试重连
        ↓
    等待 2 秒后循环
```

## 📝 数据格式

### 上报数据（JSON）

```json
{
  "id": "0",
  "version": "1.0",
  "params": {
    "temp": {
      "value": 25
    },
    "humi":  {
      "value": 60
    },
    "led":  {
      "value": true
    }
  }
}
```

### MQTT 主题规范

| 用途 | 主题格式 |
|------|---------|
| 属性上报 | `$sys/{产品ID}/{设备名}/thing/property/post` |
| 上报回复 | `$sys/{产品ID}/{设备名}/thing/property/post/reply` |

## 🐛 常见问题

### 1. WiFi 连接失败
- 检查 SSID 和密码是否正确
- 确认 ESP8266 供电稳定（建议独立供电）
- 检查串口连接是否正确（TX-RX 交叉连接）

### 2.  MQTT 连接失败
- 确认 OneNET 配置参数正确（产品 ID、设备名、Token）
- 检查 Token 是否过期（重新生成）
- 确认设备在平台上已创建且状态正常

### 3. 数据上报失败
- 检查 MQTT 主题是否正确
- 确认 JSON 格式与物模型定义一致
- 查看串口调试信息（可接 USART1 到电脑）

### 4. DHT22 读取失败
- 检查数据线连接
- 确认延时函数精度足够（DHT22 时序要求严格）
- 尝试更换传感器

## 📄 许可证

本项目代码基于 STMicroelectronics HAL 库开发，STM32 相关部分版权归 STMicroelectronics 所有。

其他自编代码部分可自由使用和修改。

## 🤝 贡献

欢迎提交 Issue 和 Pull Request！

## 📧 联系方式

- **作者**：QIFeng1S
- **GitHub**：[QIFeng1S/STM32-ESP8266-OneNet-](https://github.com/QIFeng1S/STM32-ESP8266-OneNet-)

---

⭐ 如果这个项目对你有帮助，请给个 Star 支持一下！
