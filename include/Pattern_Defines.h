#ifndef PATTERN_DEFINES_H_
#define PATTERN_DEFINES_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "Colour_Defines.h"

#define NUMBER_OF_PRESETS                       5
#define PRESET0_NAME                            "Rainbow"
#define PRESET1_NAME                            "Red-Green-Blue"
#define PRESET2_NAME                            "Aqua Wave"
#define PRESET3_NAME                            "Inferno Wave"
#define PRESET4_NAME                            "Off"
#define PRESET5_NAME                            "Holidays"

enum LED_Animation {
    Animation_Repeating,
    Animation_Wave
};

typedef struct LED_Pattern {
    enum LED_Animation animationType;
    uint8_t numberOfColours;

	LED_Colour_HSV** colours;
} LED_Pattern;

extern const char* const Pattern_Names[];
extern const LED_Pattern* const Pattern_Presets[];

#ifdef __cplusplus
}
#endif

#endif
