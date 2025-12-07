/*
*********************************************************************************************************
*	                                  
*	模块名称 : AD7606驱动模块 
*	文件名称 : bsp_ad7606.h
*	版    本 : V1.0
*	说    明 : AD7606 SPI接口驱动头文件（适配STM32H743）
*	修改记录 :
*		版本号  日期       作者    说明
*		v1.0    2025-01-XX  Auto    创建，适配STM32H7 HAL库
*
*********************************************************************************************************
*/

#ifndef __BSP_AD7606_H
#define __BSP_AD7606_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "spi.h"

/* AD7606通道数 */
#define AD7606_CH_NUM           8               /* AD7606有8个通道 */

/* 定义AD7606的SPI接口（使用SPI1） */
extern SPI_HandleTypeDef hspi1;

/* 定义AD7606的GPIO引脚（根据实际硬件连接修改） */
/* 注意：以下GPIO定义需要根据实际硬件连接进行修改 */

/* CS片选信号 - 使用PA4（已在CubeMX中配置为GPIO输出） */
#define AD7606_CS_PORT          GPIOA
#define AD7606_CS_PIN           GPIO_PIN_4

/* CONVST转换启动信号 - 使用PG13 */
#define AD7606_CONVST_PORT      GPIOG
#define AD7606_CONVST_PIN       GPIO_PIN_13

/* RESET复位信号 - 使用PG14 */
#define AD7606_RESET_PORT       GPIOG
#define AD7606_RESET_PIN        GPIO_PIN_14

/* RANGE量程选择 - 使用PC4（已在CubeMX中配置为输入，改为输出） */
#define AD7606_RANGE_PORT       GPIOC
#define AD7606_RANGE_PIN        GPIO_PIN_4

/* OS0-OS2过采样选择 - 可以暂时不连接，使用固定电平或添加GPIO */
/* 如果硬件未连接，可以在初始化时设置为固定值 */
#define AD7606_OS0_PORT         GPIOA
#define AD7606_OS0_PIN          GPIO_PIN_0      /* 需要根据实际连接修改 */
#define AD7606_OS1_PORT         GPIOA
#define AD7606_OS1_PIN          GPIO_PIN_1      /* 需要根据实际连接修改 */
#define AD7606_OS2_PORT         GPIOA
#define AD7606_OS2_PIN          GPIO_PIN_2      /* 需要根据实际连接修改 */

/* BUSY信号（可选）- 如果使用，需要配置为输入 */
/* #define AD7606_BUSY_PORT      GPIOx */
/* #define AD7606_BUSY_PIN       GPIO_PIN_x */

/* GPIO控制宏定义 */
#define AD7606_CS_LOW()         HAL_GPIO_WritePin(AD7606_CS_PORT, AD7606_CS_PIN, GPIO_PIN_RESET)
#define AD7606_CS_HIGH()        HAL_GPIO_WritePin(AD7606_CS_PORT, AD7606_CS_PIN, GPIO_PIN_SET)

#define AD7606_CONVST_LOW()     HAL_GPIO_WritePin(AD7606_CONVST_PORT, AD7606_CONVST_PIN, GPIO_PIN_RESET)
#define AD7606_CONVST_HIGH()     HAL_GPIO_WritePin(AD7606_CONVST_PORT, AD7606_CONVST_PIN, GPIO_PIN_SET)

#define AD7606_RESET_LOW()      HAL_GPIO_WritePin(AD7606_RESET_PORT, AD7606_RESET_PIN, GPIO_PIN_RESET)
#define AD7606_RESET_HIGH()      HAL_GPIO_WritePin(AD7606_RESET_PORT, AD7606_RESET_PIN, GPIO_PIN_SET)

#define AD7606_RANGE_5V()       HAL_GPIO_WritePin(AD7606_RANGE_PORT, AD7606_RANGE_PIN, GPIO_PIN_RESET)
#define AD7606_RANGE_10V()      HAL_GPIO_WritePin(AD7606_RANGE_PORT, AD7606_RANGE_PIN, GPIO_PIN_SET)

/* 过采样模式枚举 */
typedef enum {
    AD7606_OS_NO = 0,      /* 无过采样 */
    AD7606_OS_2 = 1,       /* 2倍过采样 */
    AD7606_OS_4 = 2,       /* 4倍过采样 */
    AD7606_OS_8 = 3,       /* 8倍过采样 */
    AD7606_OS_16 = 4,      /* 16倍过采样 */
    AD7606_OS_32 = 5,      /* 32倍过采样 */
    AD7606_OS_64 = 6       /* 64倍过采样 */
} AD7606_OS_Mode_t;

/* 量程选择枚举 */
typedef enum {
    AD7606_RANGE_5V = 0,   /* ±5V */
    AD7606_RANGE_10V = 1    /* ±10V */
} AD7606_Range_t;

/* 函数声明 */
void BSP_AD7606_Init(void);
void BSP_AD7606_Reset(void);
void BSP_AD7606_SetOS(AD7606_OS_Mode_t mode);
void BSP_AD7606_SetRange(AD7606_Range_t range);
void BSP_AD7606_StartConvst(void);
void BSP_AD7606_ReadChannels(uint16_t *channels);
uint16_t BSP_AD7606_ReadChannel(void);

#ifdef __cplusplus
}
#endif

#endif /* __BSP_AD7606_H */

