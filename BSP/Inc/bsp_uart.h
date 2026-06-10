// BSP/Inc/bsp_uart.h
#ifndef __BSP_UART_H
#define __BSP_UART_H

#include <stdint.h>
#include <stdbool.h>

#define UART_RX_BUF_SIZE  64

// ==========================================
// 环形缓冲区 (单生产者 ISR + 单消费者 Task，无锁)
// ==========================================
typedef struct {
    uint8_t  buffer[UART_RX_BUF_SIZE];
    volatile uint16_t head;   // ISR 写入
    volatile uint16_t tail;   // Task 读取
} UART_RingBuf_t;

void BSP_UART_Init(void);
bool BSP_UART_ReadByte(uint8_t *byte);                 // Task 取一个字节
void BSP_UART_SendBytes(const uint8_t *data, uint16_t len);
void BSP_UART_SendString(const char *str);
void BSP_UART_FeedRxISR(uint8_t byte);                 // ISR 喂入字节

#endif