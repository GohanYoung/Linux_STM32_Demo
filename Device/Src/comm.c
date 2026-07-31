#include "comm.h"
#include "motor.h"
#include <string.h>

/* =======================================================
 * 初始化（物理层由 CubeMX 生成的 USB CDC 初始化完成）
 * ======================================================= */
void Comm_Init(void)
{
    /* USB CDC 初始化由 MX_USB_DEVICE_Init() 完成 */
}

void Comm_StartRx(void)
{
    /* USB CDC 接收由 USBD_CDC_SetRxBuffer 启动，在 usb_device.c 中配置 */
}

/* =======================================================
 * 发送数据（由具体物理层实现）
 * ======================================================= */
extern void CDC_Transmit_FS(const uint8_t *data, uint16_t len);

void Comm_Send(const uint8_t *data, uint16_t len)
{
    CDC_Transmit_FS(data, len);
}

/* =======================================================
 * 中断回调处理：解析帧并投递到消息队列
 * ======================================================= */
void Comm_RxCallback(const uint8_t *buf, uint16_t len)
{
    if (len != COMM_RX_FRAME_SIZE) {
        return;
    }

    if (buf[0] == COMM_FRAME_HEADER && buf[7] == COMM_FRAME_TAIL) {
        ControlMsg_t msg;
        msg.cmd   = (ControlCmd_t)buf[1];
        msg.value = (int16_t)((buf[2] << 8) | buf[3]);

        osMessageQueuePut(Queue_ControlCmdHandle, &msg, 0, 0);
    }
}

/* =======================================================
 * 状态上报：将系统状态打包为 16 字节帧发送
 * ======================================================= */
void Comm_SendStatus(const SystemState_t *state)
{
    uint8_t tx_buf[COMM_TX_FRAME_SIZE] = {0};

    tx_buf[0]  = COMM_FRAME_HEADER;
    tx_buf[1]  = state->system_fault ? 0x01 : 0x00;

    int16_t temp = state->sensor.temperature;
    tx_buf[2]  = (uint8_t)(temp >> 8);
    tx_buf[3]  = (uint8_t)(temp & 0xFF);

    int16_t humi = state->sensor.humidity;
    tx_buf[4]  = (uint8_t)(humi >> 8);
    tx_buf[5]  = (uint8_t)(humi & 0xFF);

    tx_buf[6]  = (uint8_t)(state->motor_rpm >> 8);
    tx_buf[7]  = (uint8_t)(state->motor_rpm & 0xFF);

    tx_buf[8]  = (uint8_t)(state->motor_target_rpm >> 8);
    tx_buf[9]  = (uint8_t)(state->motor_target_rpm & 0xFF);

    tx_buf[10] = state->sensor.is_online ? 0x01 : 0x00;
    tx_buf[11] = Motor_IsRunning() ? 0x01 : 0x00;

    tx_buf[15] = COMM_FRAME_TAIL;

    Comm_Send(tx_buf, COMM_TX_FRAME_SIZE);
}