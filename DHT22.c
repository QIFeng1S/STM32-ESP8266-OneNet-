#include "stm32f1xx_hal.h"
#include "Delay.h"
#include <stdint.h>
#include "DHT22.h"

void GPIO_mode_out()
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
        
    GPIO_InitStruct.Pin = DHT22_DATA_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(DHT22_DATA_PORT, &GPIO_InitStruct);
}

void GPIO_mode_in()
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    GPIO_InitStruct.Pin = DHT22_DATA_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(DHT22_DATA_PORT, &GPIO_InitStruct);

}

//Reset DHT22
void DHT22_Rst(void)	   
{                 
    GPIO_mode_out(); 	//SET OUTPUT
    DHT22_DQ_OUT_OFF; 	//GPIOA.0=0
    HAL_Delay(20);    	//Pull down Least 18ms
    DHT22_DQ_OUT_ON; 	//GPIOA.0=1 
    Delay_us(30);     	//Pull up 20~40us
}

uint8_t DHT22_Check(void) 	   
{   
    uint8_t retry=0;
    GPIO_mode_in();//SET INPUT	 
    while (DHT22_DQ_IN&&retry<100)//DHT11 Pull down 40~80us
    {
    retry++;
    Delay_us(1);
    };	 
    if(retry>=100)
    return 1;
    else 
    retry=0;
    while (!DHT22_DQ_IN&&retry<100)//DHT11 Pull up 40~80us
    {
    retry++;
    Delay_us(1);
    };
    if(retry>=100) return 1;//chack error	    
    return 0;
}

uint8_t DHT22_Read_Bit(void) 			 
{
 	uint8_t retry=0;
	while(DHT22_DQ_IN&&retry<100)//wait become Low level
	{
		retry++;
		Delay_us(1);
	}
	retry=0;
	while(!DHT22_DQ_IN&&retry<100)//wait become High level
	{
		retry++;
		Delay_us(1);
	}
	Delay_us(40);//wait 40us
	if(DHT22_DQ_IN)
		return 1;
	else 
		return 0;		   
}

uint8_t DHT22_Read_Byte(void)    
{        
    uint8_t i,dat;
    dat=0;
	for (i=0;i<8;i++) 
	{
   		dat<<=1; 
	    dat|=DHT22_Read_Bit();
    }						    
    return dat;
}

uint8_t DHT22_Read_Data(uint8_t *temperature,uint8_t *humidity)    
{        
 	uint8_t buf[5];
	uint8_t i;
	DHT22_Rst();
	if(DHT22_Check()==0)
	{
		for(i=0;i<5;i++)
		{
			buf[i]=DHT22_Read_Byte();
		}
		if((buf[0]+buf[1]+buf[2]+buf[3])==buf[4])
		{
			*humidity=((buf[0]<<8)+buf[1])/10;
			*temperature=((buf[2]<<8)+buf[3])/10;
		}
	}
	else return 1;
	return 0;	    
}