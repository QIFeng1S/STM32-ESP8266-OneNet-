#ifndef __DHT22_H__
#define __DHT22_H__

#define DHT22_DATA_PIN        GPIO_PIN_1
#define DHT22_DATA_PORT       GPIOA

#define	DHT22_DQ_OUT_OFF   	HAL_GPIO_WritePin( DHT22_DATA_PORT , DHT22_DATA_PIN , GPIO_PIN_RESET)
#define	DHT22_DQ_OUT_ON   	HAL_GPIO_WritePin( DHT22_DATA_PORT , DHT22_DATA_PIN , GPIO_PIN_SET)
#define	DHT22_DQ_IN  		HAL_GPIO_ReadPin( DHT22_DATA_PORT , DHT22_DATA_PIN )  

void DHT22_Rst(void);
uint8_t DHT22_Check(void);
uint8_t DHT22_Read_Bit(void);   
uint8_t DHT22_Read_Byte(void);
uint8_t DHT22_Read_Data(uint8_t *temperature,uint8_t *humidity);


#endif