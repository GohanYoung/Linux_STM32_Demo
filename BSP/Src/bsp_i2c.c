#include "bsp_i2c.h"
#include "i2c.h" // 包含 CubeMX 生成的底层头文件，里面有 hi2c1 的声明

int8_t BSP_I2C1_WriteMem(uint16_t dev_addr, uint16_t mem_addr, uint8_t *data, uint16_t len) {
    // 调用 ST 官方的 HAL 库函数
    // HAL_I2C_MEMADD_SIZE_8BIT 表示寄存器地址是 8 位的
    // 100 是超时时间 (100ms)
    HAL_StatusTypeDef status = HAL_I2C_Mem_Write(&hi2c1, dev_addr, mem_addr, 
                                                 I2C_MEMADD_SIZE_8BIT, 
                                                 data, len, 100);
    if (status == HAL_OK) {
        return DEV_OK;
    } else if (status == HAL_TIMEOUT) {
        return DEV_TIMEOUT;
    }
    return DEV_ERROR;
}