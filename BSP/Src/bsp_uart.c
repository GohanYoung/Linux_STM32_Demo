#include "bsp_uart.h"
#include "usart.h"
#include "comm.h"
#include "cmsis_os.h"

#define RX_FRAME_SIZE 8

static uint8_t  rx_buf[RX_FRAME_SIZE];
static uint8_t  rx_index;
static uint8_t  rx_byte;

void BSP_UART1_Init(void)
{
    MX_USART1_UART_Init();
}

void BSP_UART1_StartRx(void)
{
    rx_index = 0;
    HAL_UART_Receive_IT(&huart1, &rx_byte, 1);
}

void BSP_UART1_Send(const uint8_t *data, uint16_t len)
{
    HAL_UART_Transmit(&huart1, (uint8_t *)data, len, 100);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance != USART1) {
        return;
    }

    if (rx_index == 0) {
        if (rx_byte == 0xAA) {
            rx_buf[rx_index++] = rx_byte;
        }
    } else {
        rx_buf[rx_index++] = rx_byte;
        if (rx_index >= RX_FRAME_SIZE) {
            Comm_RxCallback(rx_buf, RX_FRAME_SIZE);
            rx_index = 0;
        }
    }

    HAL_UART_Receive_IT(&huart1, &rx_byte, 1);
}