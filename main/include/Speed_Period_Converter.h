#ifndef SPEED_PERIOD_CONVERTER_H_
#define SPEED_PERIOD_CONVERTER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "LED.h"

void Speed_Period_Converter_Initialise(void);
uint8_t Get_Speed_From_Period(uint16_t period);
uint16_t Get_Period_From_Speed(uint8_t speed);

#ifdef __cplusplus
}
#endif

#endif
