#ifndef __COMM_H
#define __COMM_H

#include "cmsis_os.h"
#include "app_main.h"
#include <stdint.h>

/* =======================================================
 * 通信协议定义（与物理层无关）
 * ======================================================= */
#define COMM_FRAME_HEADER   0xAA
#define COMM_FRAME_TAIL     0x55
#define COMM_RX_FRAME_SIZE  8
#define COMM_TX_FRAME_SIZE  16

/* =======================================================
 * 接口函数（由具体物理层实现）
 * ======================================================= */
void Comm_Init(void);
void Comm_StartRx(void);
void Comm_Send(const uint8_t *data, uint16_t len);

/* 由底层中断/回调调用：解析帧并投递到消息队列 */
void Comm_RxCallback(const uint8_t *buf, uint16_t len);

/* 应用层调用：发送状态帧 */
void Comm_SendStatus(const SystemState_t *state);

#endif /* __COMM_H */