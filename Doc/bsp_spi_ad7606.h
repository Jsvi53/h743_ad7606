/*
*********************************************************************************************************
*	                                  
*	模块名称 : AD7606驱动模块 
*	文件名称 : bsp_ad7606.h
*	版    本 : V1.0
*	说    明 : 头文件
*	修改记录 :
*		版本号  日期       作者    说明
*		v0.1    2009-12-27 armfly  创建该文件，ST固件库版本为V3.1.2
*		v1.0    2011-01-11 armfly  ST固件库升级到V3.4.0版本。
*
*	Copyright (C), 2010-2011, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

#ifndef __BSP_AD7606_H
#define __BSP_AD7606_H

/* 每个样本2字节，采集通道 */
#define CH_NUM			2				/* 采集2通道 */
#define FIFO_SIZE		8*1024*2		/* 大小不要超过48K (CPU内部RAM 只有64K) */

/* 定义AD7606的SPI GPIO */
#define AD_SPI                           SPI2
#define AD_SPI_CLK                       RCC_APB1Periph_SPI2

#define AD_SPI_SCK_PIN                   GPIO_Pin_13				
#define AD_SPI_SCK_GPIO_PORT             GPIOB						
#define AD_SPI_SCK_GPIO_CLK              RCC_APB2Periph_GPIOB

#define AD_SPI_MISO_PIN                  GPIO_Pin_14				
#define AD_SPI_MISO_GPIO_PORT            GPIOB						
#define AD_SPI_MISO_GPIO_CLK             RCC_APB2Periph_GPIOB

#define AD_CS_PIN                        GPIO_Pin_13
#define AD_CS_GPIO_PORT                  GPIOE				
#define AD_CS_GPIO_CLK                   RCC_APB2Periph_GPIOE

/*　定义AD7606其它的GPIO */
#define AD_RESET_PIN                     GPIO_Pin_12
#define AD_RESET_GPIO_PORT               GPIOE				
#define AD_RESET_GPIO_CLK                RCC_APB2Periph_GPIOE

#define AD_CONVST_PIN                    GPIO_Pin_11
#define AD_CONVST_GPIO_PORT              GPIOE		
#define AD_CONVST_GPIO_CLK               RCC_APB2Periph_GPIOE

#define AD_RANGE_PIN                     GPIO_Pin_10
#define AD_RAGEE_GPIO_PORT               GPIOE		
#define AD_RAGEE_GPIO_CLK                RCC_APB2Periph_GPIOE

#define AD_OS0_PIN                     GPIO_Pin_7
#define AD_OS0_GPIO_PORT               GPIOE		
#define AD_OS0_GPIO_CLK                RCC_APB2Periph_GPIOE

#define AD_OS1_PIN                     GPIO_Pin_8
#define AD_OS1_GPIO_PORT               GPIOE		
#define AD_OS1_GPIO_CLK                RCC_APB2Periph_GPIOE

#define AD_OS2_PIN                     GPIO_Pin_9
#define AD_OS2_GPIO_PORT               GPIOE		
#define AD_OS2_GPIO_CLK                RCC_APB2Periph_GPIOE

#define AD_CS_LOW()     				AD_CS_GPIO_PORT->BRR = AD_CS_PIN
#define AD_CS_HIGH()     				AD_CS_GPIO_PORT->BSRR = AD_CS_PIN

#define AD_RESET_LOW()					AD_RESET_GPIO_PORT->BRR = AD_RESET_PIN
#define AD_RESET_HIGH()					AD_RESET_GPIO_PORT->BSRR = AD_RESET_PIN
	
#define AD_CONVST_LOW()					AD_CONVST_GPIO_PORT->BRR = AD_CONVST_PIN
#define AD_CONVST_HIGH()				AD_CONVST_GPIO_PORT->BSRR = AD_CONVST_PIN

#define AD_RANGE_5V()					AD_RANGE_GPIO_PORT->BRR = AD_RANGE_PIN
#define AD_RANGE_10V()					AD_RANGE_GPIO_PORT->BSRR = AD_RANGE_PIN

#define AD_OS0_0()						AD_OS0_GPIO_PORT->BRR = AD_OS0_PIN
#define AD_OS0_1()						AD_OS0_GPIO_PORT->BSRR = AD_OS0_PIN

#define AD_OS1_0()						AD_OS1_GPIO_PORT->BRR = AD_OS1_PIN
#define AD_OS1_1()						AD_OS1_GPIO_PORT->BSRR = AD_OS1_PIN

#define AD_OS2_0()						AD_OS2_GPIO_PORT->BRR = AD_OS2_PIN
#define AD_OS2_1()						AD_OS2_GPIO_PORT->BSRR = AD_OS2_PIN

/* AD数据采集缓冲区 */
typedef struct
{
	uint16_t usRead;
	uint16_t usWrite;
	uint16_t usCount;
	uint16_t usBuf[FIFO_SIZE];
}FIFO_T;

/* 供外部调用的函数声明 */
void ad7606_Reset(void);
void ad7606_SetOS(uint8_t _ucMode);
void bsp_SET_TIM2_FREQ(uint32_t _ulFreq);
void bsp_InitAD7606(void);
void ad7606_StartRecord(uint32_t _ulFreq);
void ad7606_StopRecord(void);
uint8_t GetAdcFormFifo(uint16_t *_usReadAdc);


extern FIFO_T  g_tAD;

#endif


