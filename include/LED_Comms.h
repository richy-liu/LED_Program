#ifndef LED_COMMS_H_
#define LED_COMMS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "LED.h"

#define LED_COMMS_TX             GPIO_NUM_2
#define LED_COMMS_CHANNEL        RMT_CHANNEL_0

void LED_Comms_Init(void);
void LED_Comms_Send(const uint8_t buffer[], const size_t length);

#ifdef __cplusplus
}
#endif

#endif
