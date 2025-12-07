/*
*********************************************************************************************************
*	                                  
*	模块名称 : AD7606驱动模块
*	文件名称 : bsp_ad7606.c
*	版    本 : V1.0
*	说    明 : AD7606 SPI接口驱动实现（适配STM32H743 HAL库）
*	修改记录 :
*		版本号  日期       作者    说明
*		v1.0    2025-01-XX  Auto    创建，适配STM32H7 HAL库
*
*********************************************************************************************************
*/

#include "bsp/bsp_ad7606.h"
#include "gpio.h"

/* 外部SPI句柄 */
extern SPI_HandleTypeDef hspi1;

/*
*********************************************************************************************************
*	函 数 名: BSP_AD7606_Init
*	功能说明: 初始化AD7606 SPI接口和控制GPIO
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void BSP_AD7606_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    /* 使能GPIO时钟 */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();
    
    /* 配置CS引脚（PA4已在CubeMX中配置，这里确保状态正确） */
    /* CS初始状态为高电平 */
    AD7606_CS_HIGH();
    
    /* 配置CONVST引脚（PG13） */
    GPIO_InitStruct.Pin = AD7606_CONVST_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(AD7606_CONVST_PORT, &GPIO_InitStruct);
    AD7606_CONVST_HIGH();  /* CONVST平时保持高电平 */
    
    /* 配置RESET引脚（PG14） */
    GPIO_InitStruct.Pin = AD7606_RESET_PIN;
    HAL_GPIO_Init(AD7606_RESET_PORT, &GPIO_InitStruct);
    AD7606_RESET_LOW();    /* RESET初始为低电平 */
    
    /* 配置RANGE引脚（PC4，需要从输入改为输出） */
    GPIO_InitStruct.Pin = AD7606_RANGE_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(AD7606_RANGE_PORT, &GPIO_InitStruct);
    AD7606_RANGE_5V();     /* 默认±5V量程 */
    
    /* 配置OS0-OS2引脚（如果硬件连接了） */
    /* 注意：如果硬件未连接这些引脚，可以注释掉以下代码，并在SetOS函数中不操作GPIO */
    /*
    GPIO_InitStruct.Pin = AD7606_OS0_PIN | AD7606_OS1_PIN | AD7606_OS2_PIN;
    HAL_GPIO_Init(AD7606_OS0_PORT, &GPIO_InitStruct);
    */
    
    /* 硬件复位AD7606 */
    BSP_AD7606_Reset();
    
    /* 设置过采样模式（无过采样） */
    BSP_AD7606_SetOS(AD7606_OS_NO);
    
    /* 设置量程（根据硬件连接选择） */
    /* 注意：如果RANGE引脚接高电平，使用±10V量程；接低电平，使用±5V量程 */
    /* 根据采集数据分析，建议使用±10V量程（数据最大值12537按±10V计算=3.824V，接近示波器3.59V） */
    BSP_AD7606_SetRange(AD7606_RANGE_10V);  /* 改为±10V量程 */
    
    /* 等待复位完成 */
    HAL_Delay(10);
}

/*
*********************************************************************************************************
*	函 数 名: BSP_AD7606_Reset
*	功能说明: 硬件复位AD7606
*	形    参：无
*	返 回 值: 无
*	说    明: RESET高电平脉冲宽度至少50ns
*********************************************************************************************************
*/
void BSP_AD7606_Reset(void)
{
    /* AD7606是高电平复位，要求最小脉宽50ns */
    AD7606_RESET_LOW();
    
    /* 短暂延时确保低电平 */
    for (volatile int i = 0; i < 10; i++);
    
    /* 产生高电平复位脉冲 */
    AD7606_RESET_HIGH();
    for (volatile int i = 0; i < 50; i++);  /* 延时约50ns（在480MHz下） */
    
    AD7606_RESET_LOW();
}

/*
*********************************************************************************************************
*	函 数 名: BSP_AD7606_SetOS
*	功能说明: 设置过采样模式
*	形    参：mode - 过采样模式（0=无, 1=2倍, 2=4倍, 3=8倍, 4=16倍, 5=32倍, 6=64倍）
*	返 回 值: 无
*	说    明: OS引脚在BUSY下降沿时锁存，设置过采样会影响转换时间
*********************************************************************************************************
*/
void BSP_AD7606_SetOS(AD7606_OS_Mode_t mode)
{
    /* 注意：如果OS0-OS2引脚未连接，此函数可以留空或只设置内部变量 */
    /* 过采样模式由OS[2:0]引脚组合决定 */
    /* OS2是MSB，OS0是LSB */
    
    /* 如果硬件连接了OS引脚，取消以下 #if 0 并配置GPIO */
    /* 注意：由于OS引脚可能未连接，当前实现为空函数 */
    (void)mode;  /* 避免未使用参数警告 */
    
#if 0
    /* 以下是OS引脚连接的实现示例 */
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    __HAL_RCC_GPIOA_CLK_ENABLE();
    
    GPIO_InitStruct.Pin = AD7606_OS0_PIN | AD7606_OS1_PIN | AD7606_OS2_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(AD7606_OS0_PORT, &GPIO_InitStruct);
    
    switch (mode) {
        case AD7606_OS_NO:   // 000
            HAL_GPIO_WritePin(AD7606_OS0_PORT, AD7606_OS0_PIN, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(AD7606_OS1_PORT, AD7606_OS1_PIN, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(AD7606_OS2_PORT, AD7606_OS2_PIN, GPIO_PIN_RESET);
            break;
        case AD7606_OS_2:    // 001
            HAL_GPIO_WritePin(AD7606_OS0_PORT, AD7606_OS0_PIN, GPIO_PIN_SET);
            HAL_GPIO_WritePin(AD7606_OS1_PORT, AD7606_OS1_PIN, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(AD7606_OS2_PORT, AD7606_OS2_PIN, GPIO_PIN_RESET);
            break;
        case AD7606_OS_4:    // 010
            HAL_GPIO_WritePin(AD7606_OS0_PORT, AD7606_OS0_PIN, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(AD7606_OS1_PORT, AD7606_OS1_PIN, GPIO_PIN_SET);
            HAL_GPIO_WritePin(AD7606_OS2_PORT, AD7606_OS2_PIN, GPIO_PIN_RESET);
            break;
        case AD7606_OS_8:    // 011
            HAL_GPIO_WritePin(AD7606_OS0_PORT, AD7606_OS0_PIN, GPIO_PIN_SET);
            HAL_GPIO_WritePin(AD7606_OS1_PORT, AD7606_OS1_PIN, GPIO_PIN_SET);
            HAL_GPIO_WritePin(AD7606_OS2_PORT, AD7606_OS2_PIN, GPIO_PIN_RESET);
            break;
        case AD7606_OS_16:   // 100
            HAL_GPIO_WritePin(AD7606_OS0_PORT, AD7606_OS0_PIN, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(AD7606_OS1_PORT, AD7606_OS1_PIN, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(AD7606_OS2_PORT, AD7606_OS2_PIN, GPIO_PIN_SET);
            break;
        case AD7606_OS_32:   // 101
            HAL_GPIO_WritePin(AD7606_OS0_PORT, AD7606_OS0_PIN, GPIO_PIN_SET);
            HAL_GPIO_WritePin(AD7606_OS1_PORT, AD7606_OS1_PIN, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(AD7606_OS2_PORT, AD7606_OS2_PIN, GPIO_PIN_SET);
            break;
        case AD7606_OS_64:   // 110
            HAL_GPIO_WritePin(AD7606_OS0_PORT, AD7606_OS0_PIN, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(AD7606_OS1_PORT, AD7606_OS1_PIN, GPIO_PIN_SET);
            HAL_GPIO_WritePin(AD7606_OS2_PORT, AD7606_OS2_PIN, GPIO_PIN_SET);
            break;
        default:
            break;
    }
#endif
}

/*
*********************************************************************************************************
*	函 数 名: BSP_AD7606_SetRange
*	功能说明: 设置输入量程
*	形    参：range - 量程选择（0=±5V, 1=±10V）
*	返 回 值: 无
*********************************************************************************************************
*/
void BSP_AD7606_SetRange(AD7606_Range_t range)
{
    if (range == AD7606_RANGE_5V) {
        AD7606_RANGE_5V();
    } else {
        AD7606_RANGE_10V();
    }
}

/*
*********************************************************************************************************
*	函 数 名: BSP_AD7606_StartConvst
*	功能说明: 启动AD转换
*	形    参：无
*	返 回 值: 无
*	说    明: CONVST低电平至少25ns，高电平至少25ns
*********************************************************************************************************
*/
void BSP_AD7606_StartConvst(void)
{
    /* CONVST上升沿启动转换，低电平持续时间至少25ns */
    AD7606_CONVST_LOW();
    
    /* 短暂延时确保低电平（约25ns） */
    for (volatile int i = 0; i < 10; i++);
    
    AD7606_CONVST_HIGH();
}

/*
*********************************************************************************************************
*	函 数 名: BSP_AD7606_ReadChannel
*	功能说明: 读取一个通道的数据（16位）
*	形    参：无
*	返 回 值: 读取的16位数据
*********************************************************************************************************
*/
uint16_t BSP_AD7606_ReadChannel(void)
{
    uint16_t data = 0;
    
    /* 片选有效 */
    AD7606_CS_LOW();
    
    /* 通过SPI读取16位数据 */
    HAL_SPI_Receive(&hspi1, (uint8_t*)&data, 1, HAL_MAX_DELAY);
    
    /* 片选无效 */
    AD7606_CS_HIGH();
    
    return data;
}

/*
*********************************************************************************************************
*	函 数 名: BSP_AD7606_ReadChannels
*	功能说明: 读取8个通道的数据
*	形    参：channels - 存储8个通道数据的数组指针
*	返 回 值: 无
*	说    明: 通道顺序为V1, V2, V3, V4, V5, V6, V7, V8
*********************************************************************************************************
*/
void BSP_AD7606_ReadChannels(uint16_t *channels)
{
    int i;
    
    /* 片选有效 */
    AD7606_CS_LOW();
    
    /* 读取8个通道的数据 */
    for (i = 0; i < AD7606_CH_NUM; i++) {
        HAL_SPI_Receive(&hspi1, (uint8_t*)&channels[i], 1, HAL_MAX_DELAY);
    }
    
    /* 片选无效 */
    AD7606_CS_HIGH();
}

