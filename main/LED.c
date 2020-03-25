#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sdkconfig.h"

#include <math.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/rmt.h"

#include "LED.h"
#include "LED_Comms.h"

static int difference(int num1, int num2);
static int difference_cyclic(int num1, int num2, int halfperiod);
static int cyclic_increment(int num1, int num2, int step, int total, int cycle);
static void print_values(void);

static LED_Data* LEDData[NUMBER_OF_LEDS];

// cos values between 0 and 2pi linearly scaled between 10000 and 0
static uint16_t cos_lookup[100] = {
10000, 9997, 9990, 9977, 9960, 9937, 9910, 9877, 9840, 9797, 9750, 9698, 9642,
 9581, 9515, 9444, 9369, 9290, 9206, 9118, 9026, 8930, 8830, 8726, 8619, 8507,
 8393, 8274, 8153, 8028, 7900, 7770, 7636, 7500, 7361, 7220, 7077, 6932, 6784,
 6635, 6485, 6332, 6179, 6024, 5868, 5712, 5554, 5396, 5238, 5079, 4921, 4762,
 4604, 4446, 4288, 4132, 3976, 3821, 3668, 3515, 3365, 3216, 3068, 2923, 2780,
 2639, 2500, 2364, 2230, 2100, 1972, 1847, 1726, 1607, 1493, 1381, 1274, 1170,
 1070,  974,  882,  794,  710,  631,  556,  485,  419,  358,  302,  250,  203,
  160,  123,   90,   63,   40,   23,   10,    3,    0};

void HSVtoRGB(uint8_t *r, uint8_t *g, uint8_t *b, uint8_t h, uint8_t s, uint8_t v )
{
  unsigned char region, remainder, p, q, t;

  if (s == 0)
  {
      *r = v;
      *g = v;
      *b = v;
  }

  region = h / 40;
  remainder = (h - (region * 40)) * 6;

  p = (v * (255 - s)) >> 8;
  q = (v * (255 - ((s * remainder) >> 8))) >> 8;
  t = (v * (255 - ((s * (255 - remainder)) >> 8))) >> 8;

  switch (region)
  {
      case 0:
          *r = v; *g = t; *b = p;
          break;
      case 1:
          *r = q; *g = v; *b = p;
          break;
      case 2:
          *r = p; *g = v; *b = t;
          break;
      case 3:
          *r = p; *g = q; *b = v;
          break;
      case 4:
          *r = t; *g = p; *b = v;
          break;
      default:
          *r = v; *g = p; *b = q;
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

void LED_Turn_Off()
{
    uint8_t* txBuffer = LED_Comms_Get_Tx_Buffer();
    memset(txBuffer, 0, 3 * NUMBER_OF_LEDS);
}

void LED_Task(void) {
    LED_Colour* colours[6];
    LED_Colour colour1 = {
        .red = 100,
        .green = 0,
        .blue = 0
    };
    LED_Colour colour2 = {
        .red = 0,
        .green = 100,
        .blue = 0
    };
    LED_Colour colour3 = {
        .red = 0,
        .green = 0,
        .blue = 100
    };
    LED_Colour colour4 = {
        .red = 0,
        .green = 0,
        .blue = 0
    };
    LED_Colour colour5 = {
        .red = 0,
        .green = 0,
        .blue = 100
    };
    LED_Colour colour6 = {
        .red = 100,
        .green = 0,
        .blue = 100
    };

    colours[0] = &colour1;
    colours[1] = &colour2;
    colours[2] = &colour3;
    colours[3] = &colour4;
    colours[4] = &colour5;
    colours[5] = &colour6;

    LED_Colour_HSV* coloursHSV[6];

    LED_Colour_HSV HSVcolour1 = {
        .hue = 0,
        .saturation = 255,
        .value = 100
    };
    LED_Colour_HSV HSVcolour2 = {
        .hue = 80,
        .saturation = 255,
        .value = 50
    };
    LED_Colour_HSV HSVcolour3 = {
        .hue = 160,
        .saturation = 255,
        .value = 50
    };
    LED_Colour_HSV HSVcolour4 = {
        .hue = 160,
        .saturation = 255,
        .value = 50
    };

    coloursHSV[0] = &HSVcolour1;
    coloursHSV[1] = &HSVcolour2;
    coloursHSV[2] = &HSVcolour3;
    coloursHSV[3] = &HSVcolour4;
    LED_Create_Wave_HSV(coloursHSV, 2);

    // LED_Create_Wave_Smooth(colours, 3);
    // LED_Create_Wave_Linear(colours, 3);
    // LED_Create_Static(colours, 4);
    print_values();

    while (1)
    {
        LED_Shift_Forward();

        gpio_set_level(5, 0);

        vTaskDelay(80 / portTICK_PERIOD_MS);

        LED_Comms_Send();
    }
}

void LED_Shift_Forward(void)
{
    LED_Colour colourTemp;
    uint8_t* txBuffer = LED_Comms_Get_Tx_Buffer();

    colourTemp.red = *LEDData[LAST_LED_INDEX]->red;
    colourTemp.green = *LEDData[LAST_LED_INDEX]->green;
    colourTemp.blue = *LEDData[LAST_LED_INDEX]->blue;

    memmove(txBuffer + 3, txBuffer, LAST_LED_INDEX * 3);
    *LEDData[0]->red = colourTemp.red;
    *LEDData[0]->green = colourTemp.green;
    *LEDData[0]->blue = colourTemp.blue;
}

void LED_Shift_Backward(void)
{
    LED_Colour colourTemp;
    uint8_t* txBuffer = LED_Comms_Get_Tx_Buffer();

    colourTemp.red = *LEDData[0]->red;
    colourTemp.green = *LEDData[0]->green;
    colourTemp.blue = *LEDData[0]->blue;

    memmove(txBuffer, txBuffer + 3, LAST_LED_INDEX * 3);
    *LEDData[LAST_LED_INDEX]->red = colourTemp.red;
    *LEDData[LAST_LED_INDEX]->green = colourTemp.green;
    *LEDData[LAST_LED_INDEX]->blue = colourTemp.blue;
}

void LED_Create_Static(LED_Colour* colours[], int numberOfColours)
{
    for (int i = 0, j = 0; i < NUMBER_OF_LEDS; i++)
    {
        j = i % numberOfColours;

        *LEDData[i]->red = colours[j]->red;
        *LEDData[i]->green = colours[j]->green;
        *LEDData[i]->blue = colours[j]->blue;
    }
}

void LED_Create_Wave_HSV(LED_Colour_HSV* colours[], int numberOfColours)
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
            // if (!(numberOfColours % 2))
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

        LED_Colour_HSV colourHSV;

        int hue;

        if ((colours[colourIndex0]->hue - colours[colourIndex1]->hue <= 120
            && colours[colourIndex0]->hue - colours[colourIndex1]->hue >= 0)
            || (colours[colourIndex1]->hue - colours[colourIndex0]->hue <= 120
            && colours[colourIndex1]->hue - colours[colourIndex0]->hue >= 0))
        {
            hue = colours[colourIndex0]->hue + difference(i, start) * (colours[colourIndex1]->hue - colours[colourIndex0]->hue) / difference(start, end);
        }
        else
        {
            if (colours[colourIndex1]->hue - colours[colourIndex0]->hue >= 0)
            {
                hue = colours[colourIndex0]->hue + difference(i, start) * -(colours[colourIndex0]->hue - colours[colourIndex1]->hue + 240) / difference(start, end);
            }
            else
            {
                hue = colours[colourIndex0]->hue + difference(i, start) * -(colours[colourIndex0]->hue - colours[colourIndex1]->hue - 240) / difference(start, end);
            }
        }

        if (hue >= 0)
        {
            colourHSV.hue = hue;
        }
        else
        {
            colourHSV.hue = 240 + hue;
        }

        colourHSV.saturation = colours[colourIndex0]->saturation + difference(i, start) * difference(colours[colourIndex0]->saturation, colours[colourIndex1]->saturation) / difference(start, end);
        colourHSV.value = colours[colourIndex0]->value + difference(i, start) * (colours[colourIndex1]->value - colours[colourIndex0]->value) / difference(start, end);

        // printf("%d, %d, from %d to %d\n", hue, difference(i, start), colours[colourIndex0]->hue, colours[colourIndex1]->hue);

        HSVtoRGB(LEDData[i]->red, LEDData[i]->green, LEDData[i]->blue, (float) colourHSV.hue, (float) colourHSV.saturation, (float) colourHSV.value);
    }
}

void LED_Create_Wave_Smooth(LED_Colour* colours[], int numberOfColours)
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
          // if (!(numberOfColours % 2))
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

      int startScalex1000 = cos_lookup[difference(i, start) * 99 / difference(start, end)];
      int endScalex1000 = cos_lookup[difference(i, end) * 99 / difference(start, end)];

      *LEDData[i]->red = ((startScalex1000 * colours[colourIndex0]->red) + (endScalex1000 * colours[colourIndex1]->red)) / 10000;
      *LEDData[i]->green = ((startScalex1000 * colours[colourIndex0]->green) + (endScalex1000 * colours[colourIndex1]->green)) / 10000;
      *LEDData[i]->blue = ((startScalex1000 * colours[colourIndex0]->blue) + (endScalex1000 * colours[colourIndex1]->blue)) / 10000;
  }
}

void LED_Create_Wave_Linear(LED_Colour* colours[], int numberOfColours)
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
          // end = (j * LAST_LED_INDEX + (numberOfColours / 2)) / numberOfColours;
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

      *LEDData[i]->red = ((difference(i, end) * colours[colourIndex0]->red) + (difference(i, start) * colours[colourIndex1]->red)) / difference(start, end);
      *LEDData[i]->green = ((difference(i, end) * colours[colourIndex0]->green) + (difference(i, start) * colours[colourIndex1]->green)) / difference(start, end);
      *LEDData[i]->blue = ((difference(i, end) * colours[colourIndex0]->blue) + (difference(i, start) * colours[colourIndex1]->blue)) / difference(start, end);
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

static int cyclic_increment(int num1, int num2, int step, int total, int cycle)
{
    int val = difference_cyclic(num1, num2, cycle/2);

    if (!(num2 > num1 && num2 - num1 <= cycle/2)
        || (num1 > num2 && num1 - num2 >= cycle/2))
    {
        val *= -1;
    }

    return step * val / total;
}

static int difference_cyclic(int num1, int num2, int halfperiod)
{
    // smallest greater than 0
    int value = 1, result;

    result = num2 - num1;
    if (result > 0 && value > result)
    {
        value = result;
    }

    result = num1 - num2;
    if (result > 0 && value > result)
    {
        value = result;
    }

    result = num1 - halfperiod - num2;
    if (result > 0 && value > result)
    {
        value = result;
    }

    result = num2 - halfperiod - num1;
    if (result > 0 && value > result)
    {
        value = result;
    }

    printf("%d", value);

    return value;
}
