#ifndef LED_H_
#define LED_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define NUMBER_OF_LEDS          30
#define LAST_LED_INDEX          (NUMBER_OF_LEDS - 1)

typedef struct LED_Data {
    uint8_t* red;
    uint8_t* green;
    uint8_t* blue;
} LED_Data;

typedef struct LED_Colour_Raw {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} LED_Colour_Raw;

typedef struct LED_Colour {
    uint8_t hue;
    uint8_t saturation;
    uint8_t value;
} LED_Colour;

enum Pattern_Type
{
    Pattern_Off,
    Pattern_Repeating,
    Pattern_Wave
};

void LED_Init(void);
void LED_Turn_Off(void);
void LED_Task(void *pvParameters);
void LED_Create_Repeating(LED_Colour* colours[], int numberOfColours, uint8_t brightness);
void LED_Create_Wave(LED_Colour* colours[], int numberOfColours, uint8_t brightness);
void LED_Shift_Forward(void);
void LED_Shift_Backward(void);

#ifdef __cplusplus
}
#endif

#endif
