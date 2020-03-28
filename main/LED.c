#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sdkconfig.h"

#include <math.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/rmt.h"

#include "Colour_Defines.h"
#include "LED.h"
#include "LED_Comms.h"

static int difference(int num1, int num2);
static void print_values(void);

static LED_Data* LEDData[NUMBER_OF_LEDS];
LED_Colour* colours[6];

int repeatingPatternIndex = 0;
enum Pattern_Type patternType = Pattern_Off;

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

void HSVtoRGB(LED_Data* LEDDateLocal, LED_Colour LEDColour)
{
  if (!LEDColour.saturation)
  {
      *LEDDateLocal->red = LEDColour.value;
      *LEDDateLocal->green = LEDColour.value;
      *LEDDateLocal->blue = LEDColour.value;
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
          *LEDDateLocal->red = LEDColour.value;
          *LEDDateLocal->green = (LEDColour.value * (255 - ((LEDColour.saturation * (255 - (LEDColour.hue % 43) * 6)) >> 8))) >> 8;
          *LEDDateLocal->blue = (LEDColour.value * (255 - LEDColour.saturation)) >> 8;
          break;
      case 1:
          *LEDDateLocal->red = (LEDColour.value * (255 - ((LEDColour.saturation * (LEDColour.hue % 43) * 6) >> 8))) >> 8;
          *LEDDateLocal->green = LEDColour.value;
          *LEDDateLocal->blue = (LEDColour.value * (255 - LEDColour.saturation)) >> 8;
          break;
      case 2:
          *LEDDateLocal->red = (LEDColour.value * (255 - LEDColour.saturation)) >> 8;
          *LEDDateLocal->green = LEDColour.value;
          *LEDDateLocal->blue = (LEDColour.value * (255 - ((LEDColour.saturation * (255 - (LEDColour.hue % 43) * 6)) >> 8))) >> 8;
          break;
      case 3:
          *LEDDateLocal->red = (LEDColour.value * (255 - LEDColour.saturation)) >> 8;
          *LEDDateLocal->green = (LEDColour.value * (255 - ((LEDColour.saturation * (LEDColour.hue % 43) * 6) >> 8))) >> 8;
          *LEDDateLocal->blue = LEDColour.value;
          break;
      case 4:
          *LEDDateLocal->red = (LEDColour.value * (255 - ((LEDColour.saturation * (255 - (LEDColour.hue % 43) * 6)) >> 8))) >> 8;
          *LEDDateLocal->green = (LEDColour.value * (255 - LEDColour.saturation)) >> 8;
          *LEDDateLocal->blue = LEDColour.value;
          break;
      default:
          *LEDDateLocal->red = LEDColour.value;
          *LEDDateLocal->green = (LEDColour.value * (255 - LEDColour.saturation)) >> 8;
          *LEDDateLocal->blue = (LEDColour.value * (255 - ((LEDColour.saturation * (LEDColour.hue % 43) * 6) >> 8))) >> 8;
          break;
  }
}

void LED_Init(void)
{
    LED_Comms_Init();

    uint8_t* txBuffer = LED_Comms_Get_Tx_Buffer();
    int offset;
    for (int i = 0; i < NUMBER_OF_LEDS; i++)
    {
        LEDData[i] = malloc(sizeof(LED_Data));
        offset = (i * 3);
        LEDData[i]->red = (txBuffer + offset + 1);
        LEDData[i]->green = (txBuffer + offset);
        LEDData[i]->blue = (txBuffer + offset + 2);
    }
}

void LED_Task(void *pvParameters) {

    LED_Init();

    for (int i = 0; i < 10; i++)
    {
        colours[i] = malloc(sizeof(LED_Colour));
    }

    memcpy(colours[0], Colour_Red, sizeof(LED_Colour));
    memcpy(colours[1], Colour_Green, sizeof(LED_Colour));
    memcpy(colours[2], Colour_Blue, sizeof(LED_Colour));
    memcpy(colours[3], Colour_Yellow, sizeof(LED_Colour));
    // colours[1]->value = 0;
    // colours[2]->value = 200;
    //
    LED_Create_Wave(colours, 3, 50);
    // LED_Create_Repeating(colours, 4, 255);

    print_values();

    while (1)
    {
        LED_Shift_Forward();

        vTaskDelay(80 / portTICK_PERIOD_MS);
    }
}

void LED_Turn_Off()
{
    uint8_t* txBuffer = LED_Comms_Get_Tx_Buffer();
    memset(txBuffer, 0, 3 * NUMBER_OF_LEDS);

    LED_Comms_Send();
}

void LED_Shift_Forward(void)
{
    LED_Colour_Raw colourTemp;
    uint8_t* txBuffer = LED_Comms_Get_Tx_Buffer();

    if (patternType == Pattern_Repeating)
    {
        colourTemp.red = *LEDData[repeatingPatternIndex]->red;
        colourTemp.green = *LEDData[repeatingPatternIndex]->green;
        colourTemp.blue = *LEDData[repeatingPatternIndex]->blue;
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

    LED_Comms_Send();
}

void LED_Shift_Backward(void)
{
    LED_Colour_Raw colourTemp;
    uint8_t* txBuffer = LED_Comms_Get_Tx_Buffer();

    if (patternType == Pattern_Repeating)
    {
        colourTemp.red = *LEDData[repeatingPatternIndex]->red;
        colourTemp.green = *LEDData[repeatingPatternIndex]->green;
        colourTemp.blue = *LEDData[repeatingPatternIndex]->blue;
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

    LED_Comms_Send();
}

void LED_Create_Repeating(LED_Colour* colours[], int numberOfColours, uint8_t brightness)
{
    LED_Colour colour;

    for (int i = 0, j = 0; i < NUMBER_OF_LEDS; i++)
    {
        j = i % numberOfColours;

        colour.hue = colours[j]->hue;
        colour.saturation = colours[j]->saturation;
        colour.value = colours[j]->value * brightness / 255;

        HSVtoRGB(LEDData[i], colour);
    }

    repeatingPatternIndex = numberOfColours - 1;
    patternType = Pattern_Repeating;
}

void LED_Create_Wave(LED_Colour* colours[], int numberOfColours, uint8_t brightness)
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

        int hue;

        if (colours[colourIndex0]->saturation && colours[colourIndex1]->saturation)
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
        colour.value = (colours[colourIndex0]->value + difference(i, start) * (colours[colourIndex1]->value - colours[colourIndex0]->value) / difference(start, end)) * brightness / 255;
        // printf("%d, %d, from %d to %d, ", (uint8_t) colour.hue, difference(i, start), colours[colourIndex0]->hue, colours[colourIndex1]->hue);

        HSVtoRGB(LEDData[i], colour);
        //
        // printf("h: %d, s: %d, v: %d, ", (uint8_t) colour.hue, colour.saturation, colour.value);
        //
        // printf("r: %d, g: %d, b: %d\n", *LEDData[i]->red, *LEDData[i]->green, *LEDData[i]->blue);
        patternType = Pattern_Wave;
    }
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
  return num1 >= num2 ? num1 - num2 : num2 - num1;
}
