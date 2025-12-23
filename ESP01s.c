// HAL 版本，移除非 HAL 依赖与冗余注释
#include "stm32f1xx_hal.h"
#include "ESP01s.h"
#include "OLED.h"
#include <string.h>
#include <stdio.h>


extern UART_HandleTypeDef huart2;
uint8_t aRxBuffer; // 临时存放 1 个字节

uint8_t esp_rx_buf[1024];   //接收缓冲区
uint16_t esp_rx_cnt = 0;   //接收缓冲区计数器

char esp_cmd_buf[1024];     //命令发送缓冲区

/*
* @brief  ESP8266 清除接收缓冲区
*/
void ESP8266_Clear(void)
{
    esp_rx_cnt = 0;
    memset(esp_rx_buf, 0, sizeof(esp_rx_buf));
}

/*
* @brief  清空命令缓冲区
*/
void ESP8266_Clearcmd(void)
{
    memset(esp_cmd_buf, 0, sizeof(esp_cmd_buf));
}

/*
* @brief  发送命令并等待响应
* @param  cmd: 要发送的命令字符串 
* @param  res: 期待的响应字符串
* @param  timeout: 超时时间，单位毫秒
* @retval 1: 成功收到期待响应，0: 超时未收到
*/
uint8_t ESP8266_SendCmd(char *cmd, char *res, uint32_t timeout)
{
    ESP8266_Clear();
    HAL_UART_Transmit(&huart2, (uint8_t *)cmd, strlen(cmd), 1000);
    uint32_t tickstart = HAL_GetTick();
    while((HAL_GetTick() - tickstart) < timeout)
    {
        if(strstr((char *)esp_rx_buf, res) != NULL)
        {
            return 1; // 成功
        }
    }
    return 0; // 超时失败
}

/**
 * @brief  两段式数据发送函数 (专用于 AT+MQTTPUBRAW 或 AT+CIPSEND)
 * @param  cmd: 第一步的命令头 (例如 "AT+MQTTPUBRAW=0,\"topic\",100,0,0\r\n")
 * @param  data: 第二步要发送的实际数据 (JSON字符串)
 * @param  len: 实际数据的长度
 * @param  timeout: 等待最终结果的超时时间
 * @retval 1: 成功, 0: 失败
 */
uint8_t ESP8266_SendData(char *cmd, uint8_t *data, uint16_t len, uint32_t timeout)
{
    // === 第一阶段：发送命令头，申请发送权限 ===
    ESP8266_Clear();
    HAL_UART_Transmit(&huart2, (uint8_t *)cmd, strlen(cmd), 1000);
    
    // 3. 等待 ESP8266 回复 ">" 符号
    // 这一步非常关键！如果不等 ">" 直接发数据，ESP8266 会把数据当成普通指令处理，导致报错
    uint32_t tickstart = HAL_GetTick();
    uint8_t got_prompt = 0; // 标志位：是否收到了 ">"
    
    while((HAL_GetTick() - tickstart) < 2000) // 给它 2秒时间准备
    {
        if(strstr((char *)esp_rx_buf, ">") != NULL)
        {
            got_prompt = 1; // 收到 ">" 了！
            break;          // 跳出等待循环
        }
    }
    
    // 如果等了 2秒还没收到 ">"，说明命令头有问题，直接报错退出
    if(got_prompt == 0) return 0; 

    // === 第二阶段：发送实际数据 ===
    
    // 4. 再次清空接收区 (为了接收发送完成后的 "+MQTTPUB:OK")
    ESP8266_Clear();
    
    // 5. 发送实际的数据包 (JSON)
    // 这里必须用 len，不能用 strlen，因为有时候数据里可能包含 0x00
    HAL_UART_Transmit(&huart2, data, len, 1000);
    
    // === 第三阶段：等待发送结果 ===
    
    // 6. 等待最终的成功信号
    // 对于 MQTTPUBRAW，成功通常返回 "+MQTTPUB:OK"
    // 对于 CIPSEND，成功通常返回 "SEND OK"
    // 为了通用，我们只要检测到 "OK" 就认为成功
    tickstart = HAL_GetTick();
    while((HAL_GetTick() - tickstart) < timeout)
    {
        if(strstr((char *)esp_rx_buf, "OK") != NULL)
        {
            return 1; // 成功！
        }
    }
    
    return 0; // 超时还没收到 OK，失败
}

void ESP8266_Init()
{  
    HAL_UART_Receive_IT(&huart2, &aRxBuffer, 1);

    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET); 
    HAL_Delay(100); // 保持低电平 100ms
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);
    HAL_Delay(500); // 等待模块启动 (ESP8266启动会吐乱码，多等一会)
    
    // 发送 AT+RST 确保软复位 (解决 STM32 复位但 ESP8266 未复位导致的状态残留问题)
    ESP8266_SendCmd("AT+RST\r\n", "OK", 2000);
    ESP8266_Clear();
    HAL_Delay(2000); // 等待重启完成

    ESP8266_SendCmd("AT\r\n", "OK", 500);     
    ESP8266_SendCmd("AT+CWMODE=1\r\n", "OK", 500);
    ESP8266_Clearcmd(); // 清空命令区
    sprintf(esp_cmd_buf, "AT+CWJAP=\"%s\",\"%s\"\r\n", WIFI_SSID, WIFI_PASSWORD);
    ESP8266_SendCmd(esp_cmd_buf, "WIFI CONNECTED", 10000);

}

/**
 * @brief  配置 MQTT 用户信息
 * @param  client_id: 客户端ID
 * @param  username:  用户名 (OneNET通常是产品ID)
 * @param  password:  密码 (OneNET通常是鉴权信息)
 * @return 1:成功, 0:失败
 */
uint8_t ESP8266_MQTT_UserCfg(char *client_id, char *username, char *password)
{
    ESP8266_Clearcmd();
    // 格式: AT+MQTTUSERCFG=0,1,"Client_ID","User","Pass",0,0,""
    // 注意：字符串参数必须加双引号，所以用了 \" 转义
    sprintf(esp_cmd_buf, "AT+MQTTUSERCFG=0,1,\"%s\",\"%s\",\"%s\",0,0,\"\"\r\n", 
            client_id, username, password);
            
    return ESP8266_SendCmd(esp_cmd_buf, "OK", 1000);
}

/**
 * @brief  连接 MQTT 服务器
 * @param  host: 服务器地址 (例如 "mqtts.heclouds.com")
 * @param  port: 端口号 (例如 1883)
 * @return 1:成功, 0:失败
 */
uint8_t ESP8266_MQTT_Conn(char *host, int port)
{
    ESP8266_Clearcmd();
    // 格式: AT+MQTTCONN=0,"Host",Port,1
    sprintf(esp_cmd_buf, "AT+MQTTCONN=0,\"%s\",%d,0\r\n", host, port);
    
    // 连接服务器可能需要一点时间，给 5秒 超时
    return ESP8266_SendCmd(esp_cmd_buf, "OK", 5000);
}

/**
 * @brief  订阅 MQTT 主题
 * @param  topic: 主题名称
 * @param  qos:   服务质量 (通常填 0 或 1)
 * @return 1:成功, 0:失败
 */
uint8_t ESP8266_MQTT_Sub(char *topic, int qos)
{
    ESP8266_Clearcmd();
    // 格式: AT+MQTTSUB=0,"Topic",QoS
    sprintf(esp_cmd_buf, "AT+MQTTSUB=0,\"%s\",%d\r\n", topic, qos);
    
    return ESP8266_SendCmd(esp_cmd_buf, "OK", 2000);
}

/**
 * @brief  发布 MQTT 消息 (使用 RAW 模式)
 * @param  topic:   主题名称
 * @param  payload: 要发送的数据内容 (JSON字符串)
 * @return 1:成功, 0:失败
 */
uint8_t ESP8266_MQTT_PubRaw(char *topic, char *payload)
{
    ESP8266_Clearcmd();
    
    // 1. 准备命令头
    // 格式: AT+MQTTPUBRAW=0,"Topic",DataLen,QoS,Retain
    // 注意：DataLen 是 payload 的长度
    sprintf(esp_cmd_buf, "AT+MQTTPUBRAW=0,\"%s\",%d,0,0\r\n", topic, strlen(payload));
    
    // 2. 调用通用的 SendData 函数发送
    // 注意：payload 这里强转为 uint8_t*
    return ESP8266_SendData(esp_cmd_buf, (uint8_t*)payload, strlen(payload), 5000);
}

/**
 * @brief  初始化并连接 OneNET 平台
 */
uint8_t ESP8266_OneNet_Init(void)
{
    uint8_t i;

    // ==========================================
    // 1. 初始化并连接 WiFi
    // ==========================================
    OLED_ShowString(0, 8, "WiFi Connecting", OLED_6X8);
    OLED_UpdateArea(0, 8, 128, 8);

    ESP8266_Init(); // 先执行一次硬复位+软复位+wifi连接

    for(i = 0; i < 5; i++)
    {
        HAL_Delay(1000); // 给一点时间让路由器分配IP
        // 检查 WiFi是否真的连上了 (查 IP)
        if(ESP8266_SendCmd("AT+CWSTATE?\r\n", "+CWSTATE:2", 2000) == 1)
        {
            OLED_ShowString(0, 8, "WiFi: OK       ", OLED_6X8);
            OLED_UpdateArea(0, 8, 128, 8);
            break; 
        }
        else
        {
            OLED_ShowString(0, 8, "WiFi: Retrying ", OLED_6X8);
            OLED_UpdateArea(0, 8, 128, 8);
            ESP8266_Clear();
            ESP8266_Clearcmd();
        }
    }
    if(i == 5) return ESP_WIFI_ERR;

    // ==========================================
    // 2. 配置 MQTT 用户信息
    // ==========================================
    OLED_ShowString(0, 16, "User Cfg...    ", OLED_6X8);
    OLED_UpdateArea(0, 16, 128, 8);

    for(i = 0; i < 5; i++)
    {
        if(ESP8266_MQTT_UserCfg(DEVICE_NAME, PRODUCT_ID, API_KEY) == 1)
        {
            OLED_ShowString(0, 16, "User: OK       ", OLED_6X8);
            OLED_UpdateArea(0, 16, 128, 8);
            break;
        }
        else
        {
            OLED_ShowString(0, 16, "User: Retrying ", OLED_6X8);
            OLED_UpdateArea(0, 16, 128, 8);
            ESP8266_Clear();
            ESP8266_Clearcmd();
            HAL_Delay(1000);
        }
    }
    if(i == 5) return ESP_USERCFG_ERR;

    // ==========================================
    // 3. 连接 MQTT 服务器
    // ==========================================
    OLED_ShowString(0, 24, "MQTT Connecting", OLED_6X8);
    OLED_UpdateArea(0, 24, 128, 8);

    for(i = 0; i < 5; i++)
    {
        if(ESP8266_MQTT_Conn(ONENET_SERVER, ONENET_PORT) == 1)
        {
            OLED_ShowString(0, 24, "Conn: OK       ", OLED_6X8);
            OLED_UpdateArea(0, 24, 128, 8);
            break;
        }
        else
        {
            OLED_ShowString(0, 24, "Conn: Retrying ", OLED_6X8);
            OLED_UpdateArea(0, 24, 128, 8);
            ESP8266_Clear();
            ESP8266_Clearcmd();
            HAL_Delay(1000);
        }
    }
    if(i == 5) return ESP_MQTT_CONN_ERR;

    // ==========================================
    // 4. 订阅测试主题
    // ==========================================
    OLED_ShowString(0, 32, "Subscribing... ", OLED_6X8);
    OLED_UpdateArea(0, 32, 128, 8);

    for(i = 0; i < 5; i++)
    {
        if(ESP8266_MQTT_Sub("$sys/16DDGX7Ej6/test/thing/property/post/reply", 0) == 1)
        {
            OLED_ShowString(0, 32, "Sub: OK        ", OLED_6X8);
            OLED_UpdateArea(0, 32, 128, 8);
            break;
        }
        else
        {
            OLED_ShowString(0, 32, "Sub: Retrying  ", OLED_6X8);
            OLED_UpdateArea(0, 32, 128, 8);
            ESP8266_Clear();
            ESP8266_Clearcmd();
            HAL_Delay(1000);
        }
    }
    if(i == 5) return ESP_MQTT_SUB_ERR;
    
    // ==========================================
    // 全部完成
    // ==========================================
    HAL_Delay(500);
    OLED_Clear();
    OLED_ShowString(0, 0, "NetInit OK!", OLED_8X16);
    OLED_Update(); // 最后一次全屏刷新，清除所有过程信息

    return ESP_OK; // 返回成功
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart->Instance == USART2)
    {
        esp_rx_buf[esp_rx_cnt++] = aRxBuffer;
        if(esp_rx_cnt >= 512) esp_rx_cnt = 0;
        HAL_UART_Receive_IT(&huart2, &aRxBuffer, 1);
    }
}

