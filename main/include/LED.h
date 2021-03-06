#ifndef LED_H_
#define LED_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

// #define NUMBER_OF_LEDS              30
#define NUMBER_OF_LEDS              180
#define MAXIMUM_COLOURS             10
#define LAST_LED_INDEX              (NUMBER_OF_LEDS - 1)

#define MAXIMUM_PERIOD              5000 // milliseconds
#define MINIMUM_PERIOD              2 // milliseconds
// #define SPEED_DIVISON_INTERVAL      (MAXIMUM_PERIOD / 100)

typedef struct LED_Data
{
    uint8_t* red;
    uint8_t* green;
    uint8_t* blue;
} LED_Data;

typedef struct LED_Colour_RGB
{
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} LED_Colour_RGB;

typedef struct LED_Colour
{
    uint8_t hue;
    uint8_t saturation;
    uint8_t value;
} LED_Colour;

enum LED_Pattern_Type
{
    Pattern_Type_Repeating,
    Pattern_Type_Wave,
};

typedef struct LED_Pattern
{
    enum LED_Pattern_Type patternType;
    uint16_t period;
    LED_Colour** colours;
    uint8_t numberOfColours;

    uint8_t direction;
    uint8_t cycles;
    bool syncronised;
} LED_Pattern;

extern QueueHandle_t LEDConfig_Queue, LEDPeriod_Queue, LEDBrightness_Queue;
extern SemaphoreHandle_t LEDReverse_Semaphore, LEDSyncronise_Semaphore;

void LED_Init(void);
void LED_Turn_Off(void);
void LED_Task(void *pvParameters);
uint16_t LED_Get_Period(void);
uint8_t LED_Get_Brightness(void);

#ifdef __cplusplus
}
#endif

#endif
