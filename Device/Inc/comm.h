#ifndef __COMM_H
#define __COMM_H

#include "cmsis_os.h"
#include "app_main.h"
#include <stdint.h>

#define COMM_FRAME_HEADER   0xAA
#define COMM_FRAME_TAIL     0x55
#define COMM_RX_FRAME_SIZE  8
#define COMM_TX_FRAME_SIZE  16

void Comm_Init(void);
void Comm_StartRx(void);
void Comm_Send(const uint8_t *data, uint16_t len);

void Comm_RxCallback(const uint8_t *buf, uint16_t len);
void Comm_SendStatus(const SystemState_t *state);

#endif /* __COMM_H */