#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sdkconfig.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/rmt.h"

#include "LED.h"
#include "LED_Comms.h"

static LED_Data* LEDData[NUMBER_OF_LEDS];

void LED_Init(void)
{
    LED_Comms_Init();

    for (int i = 0; i < NUMBER_OF_LEDS; i++)
    {
        LEDData[i] = calloc(sizeof(LED_Data), 0);
    }
}

void LED_Turn_Off()
{
    for (int i = 0; i < NUMBER_OF_LEDS; i++)
    {
        LEDData[i]->red = 0;
        LEDData[i]->green = 0;
        LEDData[i]->blue = 0;
        // memset(LEDData, 0, sizeof(LED_Data));
    }
}

void LED_Task(void)
{
    while (1)
    {
        gpio_set_level(5, 0);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        LED_Comms_Send(LEDData);

        gpio_set_level(5, 1);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        printf("Hello world!\n");
        LED_Comms_Send(LEDData);
    }
}
