#ifndef LED_Comms_H_
#define LED_Comms_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "LED.h"

#define LED_COMMS_TX             GPIO_NUM_17
#define LED_COMMS_CHANNEL        RMT_CHANNEL_0

void LED_Comms_Init(void);
void LED_Comms_Send(LED_Data** LEDData);

#ifdef __cplusplus
}
#endif

#endif
