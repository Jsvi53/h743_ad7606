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
#include "spi.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "bsp/bsp_ad7606.h"
#include "bsp/bsp_uart.h"
#include <string.h>  /* 用于strlen */
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* AD7606采集数据缓冲区 */
static uint16_t ad7606_channels[AD7606_CH_NUM];
static uint32_t sample_count = 0;
static uint32_t last_print_time = 0;  /* 上次打印时间（毫秒） */
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MPU_Config(void);
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

  /* MPU Configuration--------------------------------------------------------*/
  MPU_Config();

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_SPI1_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  
  /* 串口初始化后立即测试 - 使用HAL函数直接测试 */
  HAL_Delay(100);  /* 等待串口稳定 */
  
  /* 方法1：直接使用HAL函数测试 */
  const char *test_msg = "\r\n\r\n=== USART1 Test ===\r\n";
  HAL_UART_Transmit(&huart1, (uint8_t*)test_msg, strlen(test_msg), 1000);
  
  /* 方法2：使用BSP函数测试 */
  BSP_UART_SendString("串口初始化成功！\r\n");
  BSP_UART_SendString("========================================\r\n");
  BSP_UART_SendString("AD7606 ADC采集测试程序\r\n");
  BSP_UART_SendString("========================================\r\n");
  
  /* 初始化AD7606 */
  BSP_UART_SendString("正在初始化AD7606...\r\n");
  
  BSP_AD7606_Init();
  
  BSP_UART_SendString("AD7606初始化完成！\r\n");
  BSP_UART_SendString("配置：±5V量程，无过采样\r\n");
  BSP_UART_SendString("开始采集V1通道（1kHz, 1V正弦波）...\r\n\r\n");
  
  /* 等待AD7606稳定 */
  HAL_Delay(100);
  
  /* 启动第一次转换（填充流水线） */
  BSP_AD7606_StartConvst();
  HAL_Delay(1);  /* 等待第一次转换完成 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    
    /* 启动AD转换 */
    BSP_AD7606_StartConvst();
    
    /* 等待转换完成（无过采样时约4μs，使用短延时即可） */
    /* 注意：HAL_Delay(1)会延时约1ms，这会严重限制采样率到约500 SPS */
    /* 对于高速采样，应该使用BUSY信号或更短的延时 */
    for (volatile int i = 0; i < 100; i++);  /* 约10μs延时（在480MHz下） */
    
    /* 读取8个通道的数据 */
    BSP_AD7606_ReadChannels(ad7606_channels);
    
    /* 增加采样计数 */
    sample_count++;
    
    /* 每1000个样本打印一次，并显示采样率统计 */
    if (sample_count % 1000 == 0) {
        uint32_t current_time = HAL_GetTick();
        uint32_t elapsed_time = current_time - last_print_time;
        uint32_t sample_rate = 0;
        
        if (elapsed_time > 0 && last_print_time > 0) {
            sample_rate = (1000 * 1000) / elapsed_time;  /* 计算采样率（SPS） */
        }
        last_print_time = current_time;
        
        /* 打印V1通道的数据（16位补码格式） */
        int16_t adc_value = (int16_t)ad7606_channels[0];
        BSP_UART_Printf("Sample #%lu: ", sample_count);
        BSP_UART_Printf("V1=%d (0x%04X)", adc_value, ad7606_channels[0]);
        
        /* 转换为电压值（±10V量程） */
        float voltage = (float)adc_value * 10.0f / 32768.0f;
        
        /* 使用整数方式显示电压 */
        int32_t voltage_mv = (int32_t)(voltage * 1000.0f);
        int32_t abs_mv = (voltage_mv < 0) ? -voltage_mv : voltage_mv;
        BSP_UART_Printf(" = %s%d.%03d V", (voltage_mv < 0) ? "-" : "", abs_mv / 1000, abs_mv % 1000);
        
        /* 显示采样率 */
        if (sample_rate > 0) {
            BSP_UART_Printf(" [Rate: %lu SPS]", sample_rate);
        }
        
        BSP_UART_SendString("\r\n");
    }
    
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

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 60;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 5;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_3;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

 /* MPU Configuration */

void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct = {0};

  /* Disables the MPU */
  HAL_MPU_Disable();

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.BaseAddress = 0x0;
  MPU_InitStruct.Size = MPU_REGION_SIZE_4GB;
  MPU_InitStruct.SubRegionDisable = 0x87;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.AccessPermission = MPU_REGION_NO_ACCESS;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);
  /* Enables the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);

}

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
