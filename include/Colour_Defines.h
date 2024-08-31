#ifndef COLOUR_DEFINES_H_
#define COLOUR_DEFINES_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef struct LED_Colour_HSV {
    uint8_t hue;
    uint8_t saturation;
    uint8_t value;
} LED_Colour_HSV;

// Important order for comms
typedef struct LED_Colour_RGB {
    uint8_t green;
    uint8_t red;
    uint8_t blue;
} LED_Colour_RGB;

extern const LED_Colour_HSV red_HSV;
extern const LED_Colour_HSV green_HSV;
extern const LED_Colour_HSV blue_HSV;
extern const LED_Colour_HSV bloe_HSV;
extern const LED_Colour_HSV cyan_HSV;
extern const LED_Colour_HSV magenta_HSV;
extern const LED_Colour_HSV yellow_HSV;
extern const LED_Colour_HSV orange_HSV;
extern const LED_Colour_HSV white_HSV;
extern const LED_Colour_HSV black_HSV;

extern const LED_Colour_RGB black_RGB;

#ifdef __cplusplus
}
#endif

#endif
