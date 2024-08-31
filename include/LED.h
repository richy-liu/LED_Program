#ifndef LED_H_
#define LED_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "Pattern_Defines.h"

// #define NUMBER_OF_LEDS              10
#define NUMBER_OF_LEDS              180
// #define NUMBER_OF_LEDS              00
// #define MAXIMUM_COLOURS             10
#define LAST_LED_INDEX              (NUMBER_OF_LEDS - 1)

#define MAXIMUM_PERIOD              5000 // milliseconds
#define MINIMUM_PERIOD              2 // milliseconds
// #define SPEED_DIVISON_INTERVAL      (MAXIMUM_PERIOD / 100)

enum Direction {
    Forwards = 0,
    Backwards = 1,
};

extern QueueHandle_t LEDConfig_Queue, LEDPeriod_Queue, LEDBrightness_Queue, LEDReverse_Queue, LEDSynchronise_Queue;

extern LED_Pattern customPattern;

void LED_Init(void);
void LED_Turn_Off(void);
void LED_Task(void *pvParameters);
uint16_t LED_Get_Period(void);
uint8_t LED_Get_Brightness(void);
enum Direction LED_Get_Direction(void);
bool LED_Get_Synchronise(void);

#ifdef __cplusplus
}
#endif

#endif
