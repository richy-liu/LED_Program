#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sdkconfig.h"

#include <math.h>

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/rmt.h"

#include "Colour_Defines.h"
#include "Pattern_Defines.h"
#include "LED.h"
#include "LED_Comms.h"

static int difference(int num1, int num2);
static void print_values(void);
// static uint8_t fast_square_root(int number);

static LED_Data* LEDData[NUMBER_OF_LEDS];
// The copy is for changing the colour if its synced.
static LED_Colour_RGB* LEDDataCopy[NUMBER_OF_LEDS];

char currentPatternRaw[500] = {};
LED_Pattern* currentPattern = NULL;
LED_Colour* currentColours[MAXIMUM_COLOURS];

LED_Colour* customColours[MAXIMUM_COLOURS];

static uint8_t brightness = 40;
static int16_t currentOffset = 0;

QueueHandle_t LEDConfig_Queue = NULL;
QueueHandle_t LEDPeriod_Queue = NULL;
QueueHandle_t LEDBrightness_Queue = NULL;
SemaphoreHandle_t LEDReverse_Semaphore = NULL;
SemaphoreHandle_t LEDSyncronise_Semaphore = NULL;

// cos values between 0 and 2pi linearly scaled between 10000 and 0
// static uint16_t cos_lookup[100] = {
// 10000, 9997, 9990, 9977, 9960, 9937, 9910, 9877, 9840, 9797, 9750, 9698, 9642,
//  9581, 9515, 9444, 9369, 9290, 9206, 9118, 9026, 8930, 8830, 8726, 8619, 8507,
//  8393, 8274, 8153, 8028, 7900, 7770, 7636, 7500, 7361, 7220, 7077, 6932, 6784,
//  6635, 6485, 6332, 6179, 6024, 5868, 5712, 5554, 5396, 5238, 5079, 4921, 4762,
//  4604, 4446, 4288, 4132, 3976, 3821, 3668, 3515, 3365, 3216, 3068, 2923, 2780,
//  2639, 2500, 2364, 2230, 2100, 1972, 1847, 1726, 1607, 1493, 1381, 1274, 1170,
//  1070,  974,  882,  794,  710,  631,  556,  485,  419,  358,  302,  250,  203,
//   160,  123,   90,   63,   40,   23,   10,    3,    0};

// static uint16_t square_lookup[256] = {0, 1, 4, 9, 16, 25, 36, 49, 64, 81, 100,
//     121, 144, 169, 196, 225, 256, 289, 324, 361, 400, 441, 484, 529, 576, 625,
//     676, 729, 784, 841, 900, 961, 1024, 1089, 1156, 1225, 1296, 1369, 1444,
//     1521, 1600, 1681, 1764, 1849, 1936, 2025, 2116, 2209, 2304, 2401, 2500,
//     2601, 2704, 2809, 2916, 3025, 3136, 3249, 3364, 3481, 3600, 3721, 3844,
//     3969, 4096, 4225, 4356, 4489, 4624, 4761, 4900, 5041, 5184, 5329, 5476,
//     5625, 5776, 5929, 6084, 6241, 6400, 6561, 6724, 6889, 7056, 7225, 7396,
//     7569, 7744, 7921, 8100, 8281, 8464, 8649, 8836, 9025, 9216, 9409, 9604,
//     9801, 10000, 10201, 10404, 10609, 10816, 11025, 11236, 11449, 11664, 11881,
//     12100, 12321, 12544, 12769, 12996, 13225, 13456, 13689, 13924, 14161, 14400,
//     14641, 14884, 15129, 15376, 15625, 15876, 16129, 16384, 16641, 16900, 17161,
//     17424, 17689, 17956, 18225, 18496, 18769, 19044, 19321, 19600, 19881, 20164,
//     20449, 20736, 21025, 21316, 21609, 21904, 22201, 22500, 22801, 23104, 23409,
//     23716, 24025, 24336, 24649, 24964, 25281, 25600, 25921, 26244, 26569, 26896,
//     27225, 27556, 27889, 28224, 28561, 28900, 29241, 29584, 29929, 30276, 30625,
//     30976, 31329, 31684, 32041, 32400, 32761, 33124, 33489, 33856, 34225, 34596,
//     34969, 35344, 35721, 36100, 36481, 36864, 37249, 37636, 38025, 38416, 38809,
//     39204, 39601, 40000, 40401, 40804, 41209, 41616, 42025, 42436, 42849, 43264,
//     43681, 44100, 44521, 44944, 45369, 45796, 46225, 46656, 47089, 47524, 47961,
//     48400, 48841, 49284, 49729, 50176, 50625, 51076, 51529, 51984, 52441, 52900,
//     53361, 53824, 54289, 54756, 55225, 55696, 56169, 56644, 57121, 57600, 58081,
//     58564, 59049, 59536, 60025, 60516, 61009, 61504, 62001, 62500, 63001, 63504,
//     64009, 64516, 65025};

static void HSVtoRGB(LED_Data* LEDDataLocal, LED_Colour LEDColour)
{
  if (!LEDColour.saturation)
  {
      *LEDDataLocal->red = LEDColour.value;
      *LEDDataLocal->green = LEDColour.value;
      *LEDDataLocal->blue = LEDColour.value;
      return;
  }

  // region = LEDColour.hue / 43;
  // remainder = (LEDColour.hue % 43) * 6;

  // p = (LEDColour.value * (255 - LEDColour.saturation)) >> 8;
  // q = (LEDColour.value * (255 - ((LEDColour.saturation * remainder) >> 8))) >> 8;
  // t = (LEDColour.value * (255 - ((LEDColour.saturation * (255 - remainder)) >> 8))) >> 8;

  switch (LEDColour.hue / 43)
  {
      case 0:
          *LEDDataLocal->red = LEDColour.value;
          *LEDDataLocal->green = (LEDColour.value * (255 - ((LEDColour.saturation * (255 - (LEDColour.hue % 43) * 6)) >> 8))) >> 8;
          *LEDDataLocal->blue = (LEDColour.value * (255 - LEDColour.saturation)) >> 8;
          break;
      case 1:
          *LEDDataLocal->red = (LEDColour.value * (255 - ((LEDColour.saturation * (LEDColour.hue % 43) * 6) >> 8))) >> 8;
          *LEDDataLocal->green = LEDColour.value;
          *LEDDataLocal->blue = (LEDColour.value * (255 - LEDColour.saturation)) >> 8;
          break;
      case 2:
          *LEDDataLocal->red = (LEDColour.value * (255 - LEDColour.saturation)) >> 8;
          *LEDDataLocal->green = LEDColour.value;
          *LEDDataLocal->blue = (LEDColour.value * (255 - ((LEDColour.saturation * (255 - (LEDColour.hue % 43) * 6)) >> 8))) >> 8;
          break;
      case 3:
          *LEDDataLocal->red = (LEDColour.value * (255 - LEDColour.saturation)) >> 8;
          *LEDDataLocal->green = (LEDColour.value * (255 - ((LEDColour.saturation * (LEDColour.hue % 43) * 6) >> 8))) >> 8;
          *LEDDataLocal->blue = LEDColour.value;
          break;
      case 4:
          *LEDDataLocal->red = (LEDColour.value * (255 - ((LEDColour.saturation * (255 - (LEDColour.hue % 43) * 6)) >> 8))) >> 8;
          *LEDDataLocal->green = (LEDColour.value * (255 - LEDColour.saturation)) >> 8;
          *LEDDataLocal->blue = LEDColour.value;
          break;
      default:
          *LEDDataLocal->red = LEDColour.value;
          *LEDDataLocal->green = (LEDColour.value * (255 - LEDColour.saturation)) >> 8;
          *LEDDataLocal->blue = (LEDColour.value * (255 - ((LEDColour.saturation * (LEDColour.hue % 43) * 6) >> 8))) >> 8;
          break;
  }
}

void LED_Turn_Off(void)
{
    // uint8_t* txBuffer = LED_Comms_Get_Tx_Buffer();
    // memset(txBuffer, 0, 3 * NUMBER_OF_LEDS);
    //
    // LED_Comms_Send();

    memcpy(currentPattern, Pattern_Off, sizeof(LED_Pattern));
}

static void LED_Shift_Forward(void)
{
    currentOffset++;
    if (currentOffset >= NUMBER_OF_LEDS)
    {
        currentOffset = 0;
    }

    uint8_t* txBuffer = LED_Comms_Get_Tx_Buffer();

    if (currentPattern->syncronised)
    {
        for (int i = 0; i < NUMBER_OF_LEDS; i++)
        {
            *LEDData[i]->red = LEDDataCopy[currentOffset]->red;
            *LEDData[i]->green = LEDDataCopy[currentOffset]->green;
            *LEDData[i]->blue = LEDDataCopy[currentOffset]->blue;
        }
    }
    else
    {
        LED_Colour_RGB colourTemp;

        if (currentPattern->patternType == Pattern_Type_Repeating)
        {
            colourTemp.red = *LEDData[currentPattern->numberOfColours - 1]->red;
            colourTemp.green = *LEDData[currentPattern->numberOfColours - 1]->green;
            colourTemp.blue = *LEDData[currentPattern->numberOfColours - 1]->blue;
        }
        else
        {
            colourTemp.red = *LEDData[LAST_LED_INDEX]->red;
            colourTemp.green = *LEDData[LAST_LED_INDEX]->green;
            colourTemp.blue = *LEDData[LAST_LED_INDEX]->blue;
        }

        memmove(txBuffer + 3, txBuffer, LAST_LED_INDEX * 3);
        *LEDData[0]->red = colourTemp.red;
        *LEDData[0]->green = colourTemp.green;
        *LEDData[0]->blue = colourTemp.blue;
    }

    // Could change this to a memmove later in LED_Comms
    LED_Comms_Refresh_Data();
}

static void LED_Shift_Backward(void)
{
    if (currentOffset)
    {
        currentOffset--;
    }
    else
    {
        currentOffset = LAST_LED_INDEX;
    }

    if (currentPattern->syncronised)
    {
        for (int i = 0; i < NUMBER_OF_LEDS; i++)
        {
            *LEDData[i]->red = LEDDataCopy[currentOffset]->red;
            *LEDData[i]->green = LEDDataCopy[currentOffset]->green;
            *LEDData[i]->blue = LEDDataCopy[currentOffset]->blue;
        }
    }
    else
    {
        LED_Colour_RGB colourTemp;
        uint8_t* txBuffer = LED_Comms_Get_Tx_Buffer();

        if (currentPattern->patternType == Pattern_Type_Repeating
                && currentPattern->numberOfColours > 1)
        {
            colourTemp.red = *LEDData[1]->red;
            colourTemp.green = *LEDData[1]->green;
            colourTemp.blue = *LEDData[1]->blue;
        }
        else
        {
            colourTemp.red = *LEDData[0]->red;
            colourTemp.green = *LEDData[0]->green;
            colourTemp.blue = *LEDData[0]->blue;
        }

        memmove(txBuffer, txBuffer + 3, LAST_LED_INDEX * 3);
        *LEDData[LAST_LED_INDEX]->red = colourTemp.red;
        *LEDData[LAST_LED_INDEX]->green = colourTemp.green;
        *LEDData[LAST_LED_INDEX]->blue = colourTemp.blue;
    }

    // Could change this to a memmove later in LED_Comms
    LED_Comms_Refresh_Data();
}

static void LED_Create_Repeating(LED_Colour* colours[], int numberOfColours)
{
    LED_Colour colour;
    int offset = currentOffset;

    if (currentPattern->syncronised) offset = 0;

    for (int i = 0, j = numberOfColours - (offset % numberOfColours); i < NUMBER_OF_LEDS; i++, j++)
    {
        j %= numberOfColours;
        colour.hue = colours[j]->hue;
        colour.saturation = colours[j]->saturation;
        colour.value = colours[j]->value * brightness / 255;

        HSVtoRGB(LEDData[i], colour);
        // printf("r: %d, g: %d, b: %d\n", *LEDData[i]->red, *LEDData[i]->green, *LEDData[i]->blue);
    }
}

static void LED_Create_Wave(LED_Colour* colours[], int numberOfColours)
{
    int colourIndex0 = -1, colourIndex1 = 0;
    for (int i = 0, j = 0, start = 0, end = 0; i < NUMBER_OF_LEDS; i++)
    {
        if (i == end)
        {
            j++;
            colourIndex0++;
            colourIndex1++;
            start = end;

            // find the next end, rounded divison
            if (numberOfColours & 1)
            {
                end = (j * LAST_LED_INDEX + 1 + (numberOfColours / 2)) / numberOfColours;
            }
            else
            {
                end = (j * LAST_LED_INDEX + (numberOfColours / 2)) / numberOfColours;
            }

            // if it's the last one then set the value of end to the next
            if (end == LAST_LED_INDEX)
            {
                end = NUMBER_OF_LEDS;
                colourIndex1 = 0;
            }
        }

        LED_Colour colour;

        if (!(colours[colourIndex0]->value && colours[colourIndex1]->value))
        {
            if (!colours[colourIndex0]->value)
            {
                colour.hue = colours[colourIndex1]->hue;
            }
            else
            {
                colour.hue = colours[colourIndex0]->hue;
            }
        }
        else if (colours[colourIndex0]->saturation && colours[colourIndex1]->saturation)
        {
            if ((colours[colourIndex0]->hue - colours[colourIndex1]->hue <= 128
                && colours[colourIndex0]->hue - colours[colourIndex1]->hue >= 0)
                || (colours[colourIndex1]->hue - colours[colourIndex0]->hue <= 128
                && colours[colourIndex1]->hue - colours[colourIndex0]->hue >= 0))
            {
                colour.hue = colours[colourIndex0]->hue + difference(i, start) * (colours[colourIndex1]->hue - colours[colourIndex0]->hue) / difference(start, end);
            }
            else
            {
                if (colours[colourIndex1]->hue - colours[colourIndex0]->hue >= 0)
                {
                    colour.hue = colours[colourIndex0]->hue + difference(i, start) * ~((uint8_t) (colours[colourIndex0]->hue - colours[colourIndex1]->hue)) / difference(start, end);
                }
                else
                {
                    colour.hue = colours[colourIndex0]->hue + difference(i, start) * ((uint8_t) (colours[colourIndex1]->hue - colours[colourIndex0]->hue)) / difference(start, end);
                }
            }
        }
        else
        {
            if(colours[colourIndex0]->saturation)
            {
                colour.hue = colours[colourIndex0]->hue;
            }
            else
            {
                colour.hue = colours[colourIndex1]->hue;
            }
        }
        // if ((colours[colourIndex0]->hue - colours[colourIndex1]->hue <= 120
        //     && colours[colourIndex0]->hue - colours[colourIndex1]->hue >= 0)
        //     || (colours[colourIndex1]->hue - colours[colourIndex0]->hue <= 120
        //     && colours[colourIndex1]->hue - colours[colourIndex0]->hue >= 0))
        // {
        //     hue = colours[colourIndex0]->hue + difference(i, start) * (colours[colourIndex1]->hue - colours[colourIndex0]->hue) / difference(start, end);
        // }
        // else
        // {
        //     if (colours[colourIndex1]->hue - colours[colourIndex0]->hue >= 0)
        //     {
        //         hue = colours[colourIndex0]->hue + difference(i, start) * -(colours[colourIndex0]->hue - colours[colourIndex1]->hue + 240) / difference(start, end);
        //     }
        //     else
        //     {
        //         hue = colours[colourIndex0]->hue + difference(i, start) * -(colours[colourIndex0]->hue - colours[colourIndex1]->hue - 240) / difference(start, end);
        //     }
        // }

        // if (hue >= 0)
        // {
        //     colour.hue = hue;
        // }
        // else
        // {
        //     colour.hue = 240 + hue;
        // }

        colour.saturation = colours[colourIndex0]->saturation + difference(i, start) * (colours[colourIndex1]->saturation - colours[colourIndex0]->saturation) / difference(start, end);
        // colour.value = colours[colourIndex0]->value + difference(i, start) * (colours[colourIndex1]->value - colours[colourIndex0]->value) / difference(start, end);
        // colour.value = (colours[colourIndex0]->value + difference(i, start) * (colours[colourIndex1]->value - colours[colourIndex0]->value) / difference(start, end)) * brightness / 255;
        colour.value = (colours[colourIndex0]->value + difference(i, start) * (colours[colourIndex1]->value - colours[colourIndex0]->value) / difference(start, end)) * brightness / 255;
        // uint32_t contribution = (difference(i, start) * colours[colourIndex0]->value / difference(start, end) * difference(i, start) * colours[colourIndex0]->value / difference(start, end))
        //                     + (difference(i, end) * colours[colourIndex1]->value / difference(start, end) * difference(i, end) * colours[colourIndex1]->value / difference(start, end));
        // colour.value = fast_square_root((uint16_t) contribution);
        // printf("%d, %d, from %d to %d, ", (uint8_t) colour.hue, difference(i, start), colours[colourIndex0]->hue, colours[colourIndex1]->hue);
        // printf("dif: %d, dif: %d, d: %d, c: %d\n\r", difference(i, start) * colours[colourIndex0]->value / difference(start, end), difference(i, end) * colours[colourIndex1]->value / difference(start, end), contribution, colour.value);

        int offset = currentOffset;
        if (currentPattern->syncronised) offset = 0;

        int index = i + offset;
        if (index >= NUMBER_OF_LEDS)
        {
            index -= NUMBER_OF_LEDS;
        }
        HSVtoRGB(LEDData[index], colour);
        //
        // printf("h: %d, s: %d, v: %d, ", (uint8_t) colour.hue, colour.saturation, colour.value);
        //
        // printf("r: %d, g: %d, b: %d\n", *LEDData[i]->red, *LEDData[i]->green, *LEDData[i]->blue);
    }
}

static void LED_Create_Pattern(void)
{
    switch (currentPattern->patternType)
    {
        case Pattern_Type_Repeating:
            LED_Create_Repeating(currentPattern->colours, currentPattern->numberOfColours);
            break;
        case Pattern_Type_Wave:
            LED_Create_Wave(currentPattern->colours, currentPattern->numberOfColours);
            break;
        default: break;
    }

    for (int i = 0; i < NUMBER_OF_LEDS; i++)
    {
        LEDDataCopy[i]->red = *LEDData[i]->red;
        LEDDataCopy[i]->green = *LEDData[i]->green;
        LEDDataCopy[i]->blue = *LEDData[i]->blue;
    }

    currentOffset--;
    LED_Shift_Forward();

    LED_Comms_Refresh_Data();
}

static void print_values(void)
{
    for (int i = 0; i < NUMBER_OF_LEDS; i++)
    {
        if (!(i % 4))
        {
            printf("\n");
        }
        printf("{%3u, %3u, %3u}, ", *LEDData[i]->red, *LEDData[i]->green, *LEDData[i]->blue);
    }
    printf("\n");
}

static int difference(int num1, int num2)
{
    return num1 > num2 ? num1 - num2 : num2 - num1;
}

uint16_t LED_Get_Period(void)
{
    return currentPattern->period;
}

uint8_t LED_Get_Brightness(void)
{
    return brightness;
}

void LED_Init(void)
{
    LED_Comms_Init();

    currentPattern = malloc(sizeof(LED_Pattern));

    LEDConfig_Queue = xQueueCreate(1, sizeof(LED_Pattern));
    LEDPeriod_Queue = xQueueCreate(1, sizeof(uint16_t));
    LEDBrightness_Queue = xQueueCreate(1, sizeof(uint8_t));
    LEDReverse_Semaphore = xSemaphoreCreateBinary();
    LEDSyncronise_Semaphore = xSemaphoreCreateBinary();

    uint8_t* txBuffer = LED_Comms_Get_Tx_Buffer();
    int offset;
    for (int i = 0; i < NUMBER_OF_LEDS; i++)
    {
        LEDDataCopy[i] = malloc(sizeof(LED_Colour_RGB));

        LEDData[i] = malloc(sizeof(LED_Data));
        offset = (i * 3);
        LEDData[i]->red = (txBuffer + offset + 1);
        LEDData[i]->green = (txBuffer + offset);
        LEDData[i]->blue = (txBuffer + offset + 2);
    }
}

void LED_Task(void *pvParameters)
{

    LED_Init();

    LED_Turn_Off();
    // memcpy(currentPattern, Pattern_Black, sizeof(LED_Pattern));

    if (!currentPattern)
    {
        while (1) vTaskDelay(portMAX_DELAY);
    }

    LED_Create_Pattern();
    // LED_Comms_Refresh_Data();

    // print_values();

    uint16_t newPeriod;
    uint16_t timer = 0;

    LED_Pattern* newPattern;
    uint8_t newBrightness;

    uint8_t patternChanged = 1;

    while (1)
    {

        if (xQueueReceive(LEDPeriod_Queue, &newPeriod, 0))
        {
            currentPattern->period = newPeriod;
        }

        if (xSemaphoreTake(LEDReverse_Semaphore, 0))
        {
            currentPattern->direction = !currentPattern->direction;
        }

        if (xSemaphoreTake(LEDSyncronise_Semaphore, 0))
        {
            currentPattern->syncronised = !currentPattern->syncronised;
            patternChanged = 1;
            LED_Create_Pattern();
        }

        if (currentPattern->period && timer >= currentPattern->period)
        {
            timer = 0;
            patternChanged = 1;

            if (currentPattern->direction)
            {
                LED_Shift_Forward();
            }
            else
            {
                LED_Shift_Backward();
            }
        }
        else if (xQueueReceive(LEDConfig_Queue, &newPattern, 0))
        {
            memcpy(currentPattern, newPattern, sizeof(LED_Pattern));
            currentOffset = 0;
            patternChanged = 1;
            LED_Create_Pattern();
        }
        else if (xQueueReceive(LEDBrightness_Queue, &newBrightness, 0))
        {
            if (brightness != newBrightness)
            {
                patternChanged = 1;
                brightness = newBrightness;
                LED_Create_Pattern();
            }
        }

        // portTICK_PERIOD_MS == 1
        if (patternChanged) LED_Comms_Send();

        vTaskDelay(1);
        // taskYIELD();
        timer++;
        patternChanged = 0;
    }
}

// Very inacurate only up to 255
// static uint8_t fast_square_root(int number)
// {
//     for (int i = 17; i < 256; i += 17)
//     {
//         if (number <= square_lookup[i])
//         {
//             while (number <= square_lookup[i])
//             {
//                 if (number >= square_lookup[i]) return (uint8_t) i;
//                 i--;
//             }
//         }
//     }
//
//     return 0;
// }
