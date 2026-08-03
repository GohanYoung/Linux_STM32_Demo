#include "bsp_uart.h"
#include "usart.h"
#include "comm.h"
#include "cmsis_os.h"

// 接收缓冲区可以设大一点，防止上位机连发
#define RX_BUF_SIZE 32
static uint8_t rx_buf[RX_BUF_SIZE];

void BSP_UART1_Init(void)
{
    MX_USART1_UART_Init();
}

void BSP_UART1_StartRx(void)
{
    // 1. 开启串口空闲中断 (IDLE)
    __HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE);
    
    // 2. 启动 DMA 接收
    HAL_UART_Receive_DMA(&huart1, rx_buf, RX_BUF_SIZE);
}

/* 非阻塞 DMA 发送 */
void BSP_UART1_Send(const uint8_t *data, uint16_t len)
{
    // 瞬间丢给 DMA 搬运，CPU 零阻塞立刻返回！
    HAL_UART_Transmit_DMA(&huart1, (uint8_t *)data, len);
}

//空闲中断处理 它负责在收到一帧完整数据后，截断 DMA 并上报。
void BSP_UART1_IDLE_IRQHandler(void)
{
    // 检测是否是串口空闲中断触发
    if ((__HAL_UART_GET_FLAG(&huart1, UART_FLAG_IDLE) != RESET) && 
        (__HAL_UART_GET_IT_SOURCE(&huart1, UART_IT_IDLE) != RESET)) {
        
        // 1. 必须清除空闲中断标志位，否则会陷入死循环中断风暴
        __HAL_UART_CLEAR_IDLEFLAG(&huart1);
        
        // 2. 暂停 DMA，防止处理期间有新数据覆盖
        HAL_UART_DMAStop(&huart1);
        
        // 3. 计算这一帧实际收到了多少个字节
        // 实际长度 = 缓冲区总大小 - DMA还剩多少没搬完
        uint16_t rx_len = RX_BUF_SIZE - __HAL_DMA_GET_COUNTER(huart1.hdmarx);
        
        // 4. 将收到的数据推给协议层解析 (交给 Comm_RxCallback)
        if (rx_len > 0) {
            Comm_RxCallback(rx_buf, rx_len); 
        }
        
        // 5. 重新开启 DMA，清空计数，迎接下一帧指令
        HAL_UART_Receive_DMA(&huart1, rx_buf, RX_BUF_SIZE);
    }
}