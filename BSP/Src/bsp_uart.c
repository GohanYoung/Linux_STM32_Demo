// BSP/Src/bsp_uart.c
#include "bsp_uart.h"
#include "usart.h"     // huart1
#include <string.h>

static UART_RingBuf_t rx_rb;
static uint8_t uart_rx_byte;

// ---------- 环形缓冲区 ----------
static bool rb_empty(const UART_RingBuf_t *rb) {
    return (rb->head == rb->tail);
}

static bool rb_full(const UART_RingBuf_t *rb) {
    return (((rb->head + 1) % UART_RX_BUF_SIZE) == rb->tail);
}

static void rb_put(UART_RingBuf_t *rb, uint8_t byte) {
    if (!rb_full(rb)) {
        rb->buffer[rb->head] = byte;
        rb->head = (rb->head + 1) % UART_RX_BUF_SIZE;
    }
}

static bool rb_get(UART_RingBuf_t *rb, uint8_t *byte) {
    if (rb_empty(rb)) return false;
    *byte = rb->buffer[rb->tail];
    rb->tail = (rb->tail + 1) % UART_RX_BUF_SIZE;
    return true;
}

// ---------- 对外接口 ----------
void BSP_UART_Init(void) {
    memset((void *)&rx_rb, 0, sizeof(rx_rb));
    HAL_UART_Receive_IT(&huart1, &uart_rx_byte, 1);
}

bool BSP_UART_ReadByte(uint8_t *byte) {
    return rb_get(&rx_rb, byte);
}

void BSP_UART_SendBytes(const uint8_t *data, uint16_t len) {
    HAL_UART_Transmit(&huart1, (uint8_t *)data, len, 100);
}

void BSP_UART_SendString(const char *str) {
    BSP_UART_SendBytes((const uint8_t *)str, strlen(str));
}

void BSP_UART_FeedRxISR(uint8_t byte) {
    rb_put(&rx_rb, byte);
    HAL_UART_Receive_IT(&huart1, &uart_rx_byte, 1);
}

// HAL 回调：USART1 每收到一个字节就进这里(因为在main.c中没有设置这个HAL底层，所以现在可以在这里设置，而不用采用钩子)
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART1) {
        BSP_UART_FeedRxISR(uart_rx_byte);
    }
}