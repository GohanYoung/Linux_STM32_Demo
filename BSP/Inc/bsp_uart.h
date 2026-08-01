#ifndef __BSP_UART_H
#define __BSP_UART_H

#include <stdint.h>

void BSP_UART1_Init(void);
void BSP_UART1_StartRx(void);
void BSP_UART1_Send(const uint8_t *data, uint16_t len);

#endif