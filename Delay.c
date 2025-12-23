
#include "stm32f1xx_hal.h"
#include "core_cm3.h" // Cortex-M3 内核头文件，MDK/Keil/IAR 默认自带

// DWT 初始化，需在 main 初始化阶段调用一次
void DWT_Init(void)
{
	CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk; // 使能DWT
	DWT->CYCCNT = 0;
	DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

/**
  * @brief  延时模块初始化
  * @retval 无
  */
void Delay_Init(void)
{
	DWT_Init();
}

/**
  * @brief  微秒级延时
  * @param  xus 延时时长，范围：0~233015
  * @retval 无
  */
void Delay_us(uint32_t us)
{
	uint32_t cycles = us * (SystemCoreClock / 1000000);
	uint32_t start = DWT->CYCCNT;
	while ((DWT->CYCCNT - start) < cycles);
}

/**
  * @brief  毫秒级延时
  * @param  xms 延时时长，范围：0~4294967295
  * @retval 无
  */
void Delay_ms(uint32_t ms)
{
	while(ms--)
	{
		Delay_us(1000);
	}
}
 
/**
  * @brief  秒级延时
  * @param  xs 延时时长，范围：0~4294967295
  * @retval 无
  */
void Delay_s(uint32_t s)
{
	while(s--)
	{
		Delay_ms(1000);
	}
}
