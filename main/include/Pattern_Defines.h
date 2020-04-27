#ifndef PATTERN_DEFINES_H_
#define PATTERN_DEFINES_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "LED.h"

extern LED_Pattern *Pattern_Rainbow;
extern LED_Pattern *Pattern_Red_Green_Blue;
extern LED_Pattern *Pattern_Aqua_Wave;
// extern LED_Pattern *Pattern_White;

void Pattern_Initialise(void);

#ifdef __cplusplus
}
#endif

#endif
