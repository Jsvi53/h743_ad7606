/*
*********************************************************************************************************
*	                                  
*	模块名称 : 串口打印辅助模块
*	文件名称 : bsp_uart.h
*	版    本 : V1.0
*	说    明 : 串口打印辅助函数头文件
*
*********************************************************************************************************
*/

#ifndef __BSP_UART_H
#define __BSP_UART_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* 函数声明 */
void BSP_UART_Printf(const char *format, ...);
void BSP_UART_SendString(const char *str);
void BSP_UART_SendData(uint8_t *data, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif /* __BSP_UART_H */

