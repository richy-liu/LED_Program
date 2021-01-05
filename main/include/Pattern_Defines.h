#ifndef PATTERN_DEFINES_H_
#define PATTERN_DEFINES_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "LED.h"

#define NUMBER_OF_PRESETS                       6
#define PRESET0_NAME                            "Rainbow"
#define PRESET1_NAME                            "Red-Green-Blue"
#define PRESET2_NAME                            "Aqua Wave"
#define PRESET3_NAME                            "Inferno Wave"
#define PRESET4_NAME                            "Off"
#define PRESET5_NAME                            "Holidays"

extern char* Pattern_Pattern_Names[];
extern LED_Pattern* Pattern_Presets[];
extern LED_Pattern* Pattern_Off;

void Pattern_Initialise(void);

#ifdef __cplusplus
}
#endif

#endif
