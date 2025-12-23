/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "OLED.h"
#include "Delay.h"
#include "DHT22.h"
#include "ESP01s.h"
#include <math.h>
#include <string.h>
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
extern uint8_t OLED_DisplayBuf[8][128];
uint8_t temperature;
uint8_t humidity;

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/**
 * @brief  将温湿度数据封装成 OneNet 物模型 JSON 格式
 * @param  temp: 温度值
 * @param  humi: 湿度值
 * @param  buffer: 输出缓冲区 (由调用者提供内存)
 */
void Packet_JSON_Data(uint8_t temp, uint8_t humi, char *buffer)
{
    // 这里的格式必须严格匹配 OneNet 物模型直连协议
    sprintf(buffer, 
            "{\"id\":\"0\",\"version\":\"1.0\",\"params\":{\"temp\":{\"value\":%d},\"humi\":{\"value\":%d},\"led\":{\"value\":true}}}", 
            temp, humi);
}

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
  Delay_Init();
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();

  OLED_Init(); 
  /* USER CODE BEGIN 2 */

  OLED_Clear();                      
  OLED_ShowString(0, 0, "System Init...", OLED_6X8);                    
  OLED_UpdateArea(0, 0, 128, 8); // 必须调用 Update 才能显示
  HAL_Delay(1000);
  
  // 初始连接
  uint8_t net_status = ESP8266_OneNet_Init();
  if(net_status != ESP_OK)
  {
      // 如果开机就失败，显示错误码并停在这里（或者进入下面的循环重试）
      char err_buf[20];
      sprintf(err_buf, "Init Err: %d", net_status);
      OLED_ShowString(0, 56, err_buf, OLED_6X8);
      OLED_UpdateArea(0, 56, 128, 8);
  }

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  char json_buf[256];  
  char display_buf[20];

  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

    DHT22_Read_Data(&temperature, &humidity);

    sprintf(display_buf, "T:%d C  H:%d %%RH", temperature, humidity);
    OLED_ShowString(0, 16, display_buf, OLED_8X16);
    OLED_UpdateArea(0, 16, 128, 16); // 局部刷新，提升效率

    memset(json_buf, 0, sizeof(json_buf)); // 先清空，好习惯
    Packet_JSON_Data(temperature, humidity, json_buf);

    static uint16_t Pub_times = 0;// 发布计数
    if(ESP8266_MQTT_PubRaw("xxx", json_buf) == 1)
    { 
        char pub_buf[35];
        Pub_times++;
        sprintf(pub_buf, "Pub OK! Pub Times:%d", Pub_times);
        OLED_ShowString(0, 40, pub_buf, OLED_6X8);
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
        OLED_UpdateArea(0, 40, 128, 8);
        // 清除可能的重连提示
        OLED_ShowString(0, 56, "                ", OLED_6X8);
    }
    else
    {
        // 失败处理
        OLED_ShowString(0, 40, "Pub: Fail       ", OLED_6X8);
        OLED_ShowString(0, 56, "Reconnecting... ", OLED_6X8);
        OLED_Update();
        
        ESP8266_Clear();
        ESP8266_Clearcmd();
        
        // 尝试重连
        net_status = ESP8266_OneNet_Init();
        
        if(net_status != ESP_OK)
        {
            char err_buf[20];
            sprintf(err_buf, "Err Code: %d    ", net_status);
            OLED_ShowString(0, 56, err_buf, OLED_6X8);
        }
        else
        {
            OLED_ShowString(0, 56, "Reconn Success  ", OLED_6X8);
        }
    }
    OLED_Update();
    HAL_Delay(2000); 
  }
    /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

