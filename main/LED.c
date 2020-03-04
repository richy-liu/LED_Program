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

    uint8_t* txBuffer = LED_Comms_Get_Tx_Buffer();
    int offset;
    for (int i = 0; i < NUMBER_OF_LEDS; i++)
    {
        offset = (i * 3);
        LEDData[i]->red = (txBuffer + offset);
        LEDData[i]->green = (txBuffer + offset + 1);
        LEDData[i]->blue = (txBuffer + offset + 2);
    }
}

void LED_Turn_Off()
{
    uint8_t* txBuffer = LED_Comms_Get_Tx_Buffer();
    memset(txBuffer, 0, 3 * NUMBER_OF_LEDS);
}

void LED_Task(void)
{
    while (1)
    {
        gpio_set_level(5, 0);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        LED_Comms_Send();

        gpio_set_level(5, 1);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        printf("Hello world!\n");
        LED_Comms_Send();
    }
}
