#include "bsp_i2c.h"
#include "i2c.h"
#include "main.h"
#include "cmsis_os2.h"
//IIC总线恢复函数，用于在 I2C 总线卡死时释放 SDA
// 声明外部的 I2C 初始化函数 
extern void MX_I2C1_Init(void);

// 声明一个 I2C 互斥锁 (需要在 App_Init 或 main 中创建)
osMutexId_t I2C1_MutexHandle; 

static void I2C1_BusRecovery(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* 1. 彻底复位 I2C 外设 */
    __HAL_RCC_I2C1_FORCE_RESET();
    HAL_Delay(1); 
    __HAL_RCC_I2C1_RELEASE_RESET();
    CLEAR_BIT(hi2c1.Instance->CR1, I2C_CR1_PE);

    /* 2. 将 SCL(PB6) 和 SDA(PB7) 配置为普通开漏输出 */
    GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* 3. 手动发送 9 个 SCL 时钟脉冲，释放 SDA */
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET);
    for (int i = 0; i < 9; i++) {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);
        for (volatile int d = 0; d < 50; d++); // 稍微增加一点延时
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET);
        for (volatile int d = 0; d < 50; d++);
    }

    /* 4. 发送 STOP 条件 */
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);
    for (volatile int d = 0; d < 50; d++);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET);
    for (volatile int d = 0; d < 50; d++);

    /* 5. 关键修复：不要自己配 GPIO，直接调用 CubeMX 生成的完整初始化 */
    // MX_I2C1_Init 内部包含了正确的 GPIO 复用配置和 I2C 寄存器配置
    MX_I2C1_Init(); 
}

int8_t BSP_I2C1_WriteMem(uint16_t dev_addr, uint16_t mem_addr, uint8_t *data, uint16_t len) {
    HAL_StatusTypeDef status;
    uint8_t retry = 0;

    // 【重要】获取 I2C 互斥锁，防止多任务争抢总线
    if (I2C1_MutexHandle != NULL) {
        osMutexAcquire(I2C1_MutexHandle, osWaitForever);
    }

    while (retry < 3) {
        // 容错处理：等待一小段时间看 BUSY 是否解除，而不是一上来就重置
        uint32_t timeout = 1000;
        while (__HAL_I2C_GET_FLAG(&hi2c1, I2C_FLAG_BUSY) == SET && timeout > 0) {
            timeout--;
            for(volatile int d = 0; d < 10; d++); // 微小延时
        }

        // 如果真的死锁了，才执行恢复
        if (timeout == 0) {
            I2C1_BusRecovery();
        }

        status = HAL_I2C_Mem_Write(&hi2c1, dev_addr, mem_addr,
                                   I2C_MEMADD_SIZE_8BIT,
                                   data, len, 100);
                                   
        if (status == HAL_OK) {
            break; // 成功则跳出重试循环
        }
        
        retry++;
        I2C1_BusRecovery(); // 失败则重置总线重试
    }

    // 释放互斥锁
    if (I2C1_MutexHandle != NULL) {
        osMutexRelease(I2C1_MutexHandle);
    }

    if (status == HAL_OK) return DEV_OK;
    else if (status == HAL_TIMEOUT) return DEV_TIMEOUT;
    return DEV_ERROR;
}