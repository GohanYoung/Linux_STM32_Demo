#include "dht11.h"
#include "bsp_dht11.h"
#include "bsp_sys.h"

void Device_DHT11_Init(void) {
    BSP_DHT11_SetPin(1);    // 默认释放总线，保持高电平
    BSP_Delay_us(1000);     // 上电稳定延时
}

// 内部静态函数：等待引脚变为期望的电平target_state，等待timeout_us微秒超时，返回DEV_OK或DEV_TIMEOUT
static int8_t Wait_Pin_State(uint8_t target_state, uint16_t timeout_us) {
    uint16_t retry = 0;
    while (BSP_DHT11_ReadPin() != target_state) {
        BSP_Delay_us(1);
        if (++retry > timeout_us) return DEV_TIMEOUT; // 超时报错
    }
    return DEV_OK;
}

// 内部静态函数：从DHT11读取 1 个字节 (8 bits)
static int8_t Read_Byte(uint8_t *data) {
    uint8_t i, temp = 0;
    for (i = 0; i < 8; i++) {
        temp <<= 1; // 先左移一位
        
        // 1. 等待变低电平 (每个比特传输前都有 50us 的低电平)
        if (Wait_Pin_State(0, 100) != DEV_OK) return DEV_TIMEOUT;
        
        // 2. 等待变高电平 (高电平持续时间决定是 0 还是 1)
        if (Wait_Pin_State(1, 100) != DEV_OK) return DEV_TIMEOUT;
        
        // 3. 延时 40us。因为数据 0 的高电平只有 28us，数据 1 的高电平有 70us。
        // 如果 40us 后引脚还是高电平，说明发的是 1；否则就是 0。
        BSP_Delay_us(40);
        if (BSP_DHT11_ReadPin() == 1) {
            temp |= 0x01;
            // 等待这个高电平走完
            if (Wait_Pin_State(0, 100) != DEV_OK) return DEV_TIMEOUT; 
        }
    }
    *data = temp;
    return DEV_OK;
}
// 读取一次温湿度，返回 DEV_OK 表示成功，DEV_ERROR 表示失败或校验错
int8_t Device_DHT11_Read(SensorDHT11_t *sensor_data) {
    uint8_t buf[5]; // 接收 40bit = 5字节
    uint8_t i;

    // 1. 单片机发送起始信号：拉低至少 18ms
    BSP_DHT11_SetPin(0);
    // 调用 BSP 封装的延时，底层自动触发 OS 任务调度，完美避开代码耦合
    BSP_Sys_Delay(20);
    
    // 单片机释放总线，等待传感器响应 (延时 20~40us)
    BSP_DHT11_SetPin(1);
    BSP_Delay_us(30);                             

    // 2. 调用 BSP 封装的临界区，保护微秒级时序不被打断
    BSP_Sys_EnterCritical();

    // 3. 检查传感器的应答信号 (传感器拉低 80us，再拉高 80us，如果没有先拉底再拉高，说明没应答)
    if (Wait_Pin_State(0, 100) != DEV_OK) { BSP_Sys_ExitCritical(); return DEV_ERROR; }
    if (Wait_Pin_State(1, 100) != DEV_OK) { BSP_Sys_ExitCritical(); return DEV_ERROR; } 

    // 4. 开始接收 40 位数据
    for (i = 0; i < 5; i++) {
        if (Read_Byte(&buf[i]) != DEV_OK) { BSP_Sys_ExitCritical(); return DEV_ERROR; }
    }

    BSP_Sys_ExitCritical();

    // 5. 释放总线，结束通信
    BSP_DHT11_SetPin(1);

    // 6. 数据校验 (最后 1 字节 = 前 4 字节之和)
    if (buf[0] + buf[1] + buf[2] + buf[3] == buf[4]) {
        // DHT11 数据格式：湿度整数(0), 湿度小数(1), 温度整数(2), 温度小数(3)
        sensor_data->humidity    = (int16_t)(buf[0] * 10 + buf[1]);
        sensor_data->temperature = (int16_t)(buf[2] * 10 + buf[3]);
        sensor_data->is_online = 1;
        return DEV_OK;
    } else {
        return DEV_ERROR; // 校验错
    }
}