#include "my_i2c.h"
#include "Delay.h"

static void I2C_SCL_Write(uint8_t value)
{
    HAL_GPIO_WritePin(I2C_SCL_GPIO_PORT, I2C_SCL_GPIO_PIN, 
                      (GPIO_PinState)value);
    Delay_us(10);
}

static void I2C_SDA_Write(uint8_t value)
{
    HAL_GPIO_WritePin(I2C_SDA_GPIO_PORT, I2C_SDA_GPIO_PIN, 
                      (GPIO_PinState)value);
    Delay_us(10);
}

static uint8_t I2C_SDA_Read(void)
{
    return HAL_GPIO_ReadPin(I2C_SDA_GPIO_PORT, I2C_SDA_GPIO_PIN);  
}

void My_I2C_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct ;
    
    /* 启用 GPIO 时钟 */
    __HAL_RCC_GPIOB_CLK_ENABLE();
    
    /* 配置 SCL 和 SDA 为开漏输出 */
    GPIO_InitStruct.Pin = I2C_SCL_GPIO_PIN | I2C_SDA_GPIO_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;      // 开漏输出
    GPIO_InitStruct.Pull = GPIO_NOPULL;               // 无上拉
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;     // 高速
    HAL_GPIO_Init(I2C_SCL_GPIO_PORT, &GPIO_InitStruct);
    
    /* 释放 I2C 总线 */
    I2C_SCL_Write(1);
    I2C_SDA_Write(1);
}

void My_I2C_Start(void)
{
    I2C_SDA_Write(1);
    I2C_SCL_Write(1);
    I2C_SDA_Write(0);
    I2C_SCL_Write(0);
}

void My_I2C_Stop(void)
{
    I2C_SDA_Write(0);
    I2C_SCL_Write(1);
    I2C_SDA_Write(1);
}

void MyI2C_SendByte(uint8_t Byte)
{
	uint8_t i;
	for (i = 0; i < 8; i ++)				
	{
		I2C_SDA_Write(Byte & (0x80 >> i));	
		I2C_SCL_Write(1);						
		I2C_SCL_Write(0);						
	}
}

uint8_t MyI2C_ReceiveByte(void)
{
	uint8_t i, Byte = 0x00;					
	I2C_SDA_Write(1);							
	for (i = 0; i < 8; i ++)				
	{
		I2C_SCL_Write(1);						
		if (I2C_SDA_Read() == 1){Byte |= (0x80 >> i);}	
														
		I2C_SCL_Write(0);						
	}
	return Byte;							
}

void MyI2C_SendAck(uint8_t AckBit)
{
	I2C_SDA_Write(AckBit);					//主机把应答位数据放到SDA线
	I2C_SCL_Write(1);							//释放SCL，从机在SCL高电平期间，读取应答位
	I2C_SCL_Write(0);							//拉低SCL，开始下一个时序模块
}

uint8_t MyI2C_ReceiveAck(void)
{
	uint8_t AckBit;							//定义应答位变量
	I2C_SDA_Write(1);							//接收前，主机先确保释放SDA，避免干扰从机的数据发送
	I2C_SCL_Write(1);							//释放SCL，主机机在SCL高电平期间读取SDA
	AckBit = I2C_SDA_Read();					//将应答位存储到变量里
	I2C_SCL_Write(0);							//拉低SCL，开始下一个时序模块
	return AckBit;							//返回定义应答位变量
}