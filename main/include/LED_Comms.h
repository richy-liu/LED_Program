#ifndef LED_COMMS_H_
#define LED_COMMS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "LED.h"

#define LED_COMMS_TX             GPIO_NUM_17
#define LED_COMMS_CHANNEL        RMT_CHANNEL_0

void LED_Comms_Init(void);
uint8_t* LED_Comms_Get_Tx_Buffer(void);
void LED_Comms_Refresh_Data(void);
void LED_Comms_Send(void);

#ifdef __cplusplus
}
#endif

#endif
