#ifndef __ESP01S_H__
#define __ESP01S_H__

#include "stm32f1xx_hal.h"
#include <string.h>

// =====================================================
// 第1步：配置参数（请修改为你自己的）
// =====================================================
#define WIFI_SSID       "u_Do"           // 你的WiFi名称
#define WIFI_PASSWORD   "13756232127"       // 你的WiFi密码

// 固定明文域名：mqtts.heclouds.com:1883（不启用TLS）
#define ONENET_SERVER   "mqtts.heclouds.com"
#define ONENET_PORT     1883

#define PRODUCT_ID      "16DDGX7Ej6"        // 产品ID（控制台可见）
#define DEVICE_NAME     "test"    // 设备名（创建设备的名称）
#define API_KEY         "version=2018-10-31&res=products%2F16DDGX7Ej6%2Fdevices%2Ftest&et=1797330262&method=md5&sign=AXeu4WlB9gMxIqmlBn4wgw%3D%3D"  // 使用登录令牌(token)
// =====================================================

// 定义网络初始化错误码
#define ESP_OK              0
#define ESP_WIFI_ERR        1
#define ESP_USERCFG_ERR     2
#define ESP_MQTT_CONN_ERR   3
#define ESP_MQTT_SUB_ERR    4

void ESP8266_Clear(void);
void ESP8266_Clearcmd(void);

void ESP8266_Init(void);
uint8_t ESP8266_SendCmd(char *cmd, char *res, uint32_t timeout);
uint8_t ESP8266_SendData(char *cmd, uint8_t *data, uint16_t len, uint32_t timeout);
uint8_t ESP8266_MQTT_UserCfg(char *client_id, char *username, char *password);
uint8_t ESP8266_MQTT_Conn(char *host, int port);
uint8_t ESP8266_MQTT_Sub(char *topic, int qos);
uint8_t ESP8266_MQTT_PubRaw(char *topic, char *payload);

uint8_t ESP8266_OneNet_Init(void);

#endif /* __ESP01S_H__ */