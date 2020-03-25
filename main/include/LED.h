#ifndef LED_H_
#define LED_H_

#ifdef __cplusplus
extern "C" {
#endif

#define NUMBER_OF_LEDS          40
#define LAST_LED_INDEX          (NUMBER_OF_LEDS - 1)

typedef struct LED_Data {
    uint8_t* red;
    uint8_t* green;
    uint8_t* blue;
} LED_Data;

typedef struct LED_Colour {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} LED_Colour;

typedef struct LED_Colour_HSV {
    uint8_t hue;
    uint8_t saturation;
    uint8_t value;
} LED_Colour_HSV;

void LED_Init(void);
void LED_Turn_Off(void);
void LED_Task(void);
void LED_Create_Static(LED_Colour* colours[], int numberOfColours);
void LED_Create_Wave_HSV(LED_Colour_HSV* colours[], int numberOfColours);
void LED_Create_Wave_Smooth(LED_Colour* colours[], int numberOfColours);
void LED_Create_Wave_Linear(LED_Colour* colours[], int numberOfColours);
void LED_Shift_Forward(void);
void LED_Shift_Backward(void);

#ifdef __cplusplus
}
#endif

#endif
