#ifndef LED_H_
#define LED_H_

#ifdef __cplusplus
extern "C" {
#endif

#define NUMBER_OF_LEDS          10

typedef struct LED_Data {
    uint8_t* red;
    uint8_t* green;
    uint8_t* blue;
} LED_Data;

void LED_Init(void);
void LED_Turn_Off(void);
void LED_Task(void);

#ifdef __cplusplus
}
#endif

#endif
