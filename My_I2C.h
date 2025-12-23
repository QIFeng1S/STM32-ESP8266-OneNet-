/*
 * 软件 I2C 驱动头文件（HAL 库实现）
 * STM32F103 平台
 */

#ifndef __MY_I2C_H__
#define __MY_I2C_H__

/* ==================== 系统头文件 ==================== */
#include "stm32f1xx_hal.h"       // STM32F1 HAL 库
#include <stdint.h>              // 标准整数类型（uint8_t 等）

/* ==================== I2C 引脚定义 ==================== */
/* 这些可以在 main.h 中定义，或在这里定义 */
#define I2C_SCL_GPIO_PORT    GPIOB
#define I2C_SCL_GPIO_PIN     GPIO_PIN_8

#define I2C_SDA_GPIO_PORT    GPIOB
#define I2C_SDA_GPIO_PIN     GPIO_PIN_9

/* ==================== 函数声明 ==================== */
void My_I2C_Init(void);
void My_I2C_Start(void);
void My_I2C_Stop(void);
void MyI2C_SendByte(uint8_t Byte);
uint8_t MyI2C_ReceiveByte(void);
void MyI2C_SendAck(uint8_t AckBit);
uint8_t MyI2C_ReceiveAck(void);

#endif