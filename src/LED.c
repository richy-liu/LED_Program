#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sdkconfig.h"

#include <math.h>

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#include "Colour_Defines.h"
#include "LED.h"
#include "LED_Comms.h"

typedef struct LED_Data {
    size_t ledCount;
    size_t arraySize;

    LED_Colour_HSV* coloursHSV;
    LED_Colour_RGB* coloursRGB;
    LED_Colour_RGB* coloursRGBCopy;

    // bool solidColour;
    // LED_Colour_HSV customColour;
    LED_Pattern* ledPattern;

    uint8_t brightness;
    int16_t currentOffset;
    uint16_t period;
    uint8_t cycles;
    enum Direction direction;
    bool synchronise;
} LED_Data;

static LED_Colour_HSV customColour = {0};
static const LED_Colour_HSV* customColours[] = {&customColour};
LED_Pattern customPattern = {
    .numberOfColours = 1,
    .colours = &customColours
};

static int difference(int num1, int num2);
static void print_values(void);
// static uint8_t fast_square_root(int number);

// static LED_Data* LEDData.coloursRGB[LEDData.ledCount];
// The copy is for changing the colour if its synced.
// static LED_Colour_RGB LEDData.coloursRGB[LEDData.ledCount];
// static LED_Colour_RGB LEDData.coloursRGBCopy[LEDData.ledCount];

// char LEDDataRaw[500] = {};
LED_Data LEDData = {
    .brightness = 40 // Just one default value.
};

// LED_Colour_RGB currentColours[MAXIMUM_COLOURS];
// LED_Colour_RGB customColours[MAXIMUM_COLOURS];

QueueHandle_t LEDConfig_Queue = NULL;
QueueHandle_t LEDPeriod_Queue = NULL;
QueueHandle_t LEDBrightness_Queue = NULL;
QueueHandle_t LEDReverse_Queue = NULL;
QueueHandle_t LEDSynchronise_Queue = NULL;

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

static void HSVtoRGB(LED_Colour_RGB* LEDDataLocal, LED_Colour_HSV* LEDColour) {
    if (LEDColour->saturation == 0) {
        LEDDataLocal->red = LEDColour->value;
        LEDDataLocal->green = LEDColour->value;
        LEDDataLocal->blue = LEDColour->value;
        return;
    }

  // region = LEDColour->hue / 43;
  // remainder = (LEDColour->hue % 43) * 6;

  // p = (LEDColour->value * (255 - LEDColour->saturation)) >> 8;
  // q = (LEDColour->value * (255 - ((LEDColour->saturation * remainder) >> 8))) >> 8;
  // t = (LEDColour->value * (255 - ((LEDColour->saturation * (255 - remainder)) >> 8))) >> 8;

    switch (LEDColour->hue / 43) {
      case 0:
          LEDDataLocal->red = LEDColour->value;
          LEDDataLocal->green = (LEDColour->value * (255 - ((LEDColour->saturation * (255 - (LEDColour->hue % 43) * 6)) >> 8))) >> 8;
          LEDDataLocal->blue = (LEDColour->value * (255 - LEDColour->saturation)) >> 8;
          break;
      case 1:
          LEDDataLocal->red = (LEDColour->value * (255 - ((LEDColour->saturation * (LEDColour->hue % 43) * 6) >> 8))) >> 8;
          LEDDataLocal->green = LEDColour->value;
          LEDDataLocal->blue = (LEDColour->value * (255 - LEDColour->saturation)) >> 8;
          break;
      case 2:
          LEDDataLocal->red = (LEDColour->value * (255 - LEDColour->saturation)) >> 8;
          LEDDataLocal->green = LEDColour->value;
          LEDDataLocal->blue = (LEDColour->value * (255 - ((LEDColour->saturation * (255 - (LEDColour->hue % 43) * 6)) >> 8))) >> 8;
          break;
      case 3:
          LEDDataLocal->red = (LEDColour->value * (255 - LEDColour->saturation)) >> 8;
          LEDDataLocal->green = (LEDColour->value * (255 - ((LEDColour->saturation * (LEDColour->hue % 43) * 6) >> 8))) >> 8;
          LEDDataLocal->blue = LEDColour->value;
          break;
      case 4:
          LEDDataLocal->red = (LEDColour->value * (255 - ((LEDColour->saturation * (255 - (LEDColour->hue % 43) * 6)) >> 8))) >> 8;
          LEDDataLocal->green = (LEDColour->value * (255 - LEDColour->saturation)) >> 8;
          LEDDataLocal->blue = LEDColour->value;
          break;
      default:
          LEDDataLocal->red = LEDColour->value;
          LEDDataLocal->green = (LEDColour->value * (255 - LEDColour->saturation)) >> 8;
          LEDDataLocal->blue = (LEDColour->value * (255 - ((LEDColour->saturation * (LEDColour->hue % 43) * 6) >> 8))) >> 8;
          break;
    }
}

void LED_Turn_Off(void) {
    // uint8_t* txBuffer = LED_Comms_Get_Tx_Buffer();
    // memset(txBuffer, 0, 3 * LEDData.ledCount);
    //
    // LED_Comms_Send();

    memset(LEDData.coloursRGB, 0, LEDData.arraySize);
}

static void LED_Shift_Forward(void) {
    // printf("for\n");
    LEDData.currentOffset++;
    if (LEDData.currentOffset >= LEDData.ledCount) {
        LEDData.currentOffset = 0;
    }

    // uint8_t* txBuffer = LED_Comms_Get_Tx_Buffer();

    if (LEDData.synchronise) {
        if (LEDData.ledPattern->animationType == Animation_Repeating) {
            if (LEDData.currentOffset >= LEDData.ledPattern->numberOfColours) {
                LEDData.currentOffset = 0;
            }
        }

        LED_Colour_RGB nextColor = LEDData.coloursRGBCopy[LEDData.currentOffset];
        for (uint32_t i = 0; i < LEDData.ledCount; i++) {
            memcpy(LEDData.coloursRGB + i, &nextColor, sizeof(*LEDData.coloursRGB));
        }
    } else {
        LED_Colour_RGB colourTemp = {0};

        switch (LEDData.ledPattern->animationType) {
            case Animation_Repeating: {
                colourTemp = LEDData.coloursRGB[LEDData.ledPattern->numberOfColours - 1];
                // HSVtoRGB(&colourTemp, LEDData.coloursRGB[LEDData.ledPattern->numberOfColours - 1]);
                break;
            }
            case Animation_Wave: {
                colourTemp = LEDData.coloursRGB[LEDData.ledCount - 1];
                // memcpy(&colourTemp, &LEDData.coloursRGB[LEDData.ledCount - 1], sizeof(colourTemp));
                break;
            }
            default: break;
        }

        memmove(LEDData.coloursRGB + 1, LEDData.coloursRGB, LEDData.arraySize - sizeof(*LEDData.coloursRGB));
        memcpy(LEDData.coloursRGB, &colourTemp, sizeof(colourTemp));
    }
}

static void LED_Shift_Backward(void) {
    // printf("oiwruh\n");
    LEDData.currentOffset--;
    if (LEDData.currentOffset < 0) {
        LEDData.currentOffset = LEDData.ledCount - 1;
    }

    if (LEDData.synchronise) {
        if (LEDData.ledPattern->animationType == Animation_Repeating) {
            if (LEDData.currentOffset < LEDData.ledCount - LEDData.ledPattern->numberOfColours) {
                LEDData.currentOffset = LEDData.ledCount - 1;
            }
        }

        LED_Colour_RGB nextColor = LEDData.coloursRGBCopy[LEDData.currentOffset];
        for (uint32_t i = 0; i < LEDData.ledCount; i++) {
            memcpy(LEDData.coloursRGB + i, &nextColor, sizeof(*LEDData.coloursRGB));
        }
    } else {
        LED_Colour_RGB colourTemp = {0};

        switch (LEDData.ledPattern->animationType) {
            case Animation_Repeating: {
                colourTemp = LEDData.coloursRGB[LEDData.ledCount - LEDData.ledPattern->numberOfColours];
                // HSVtoRGB(&colourTemp, LEDData.ledPattern->colours[0]);
                break;
            }
            case Animation_Wave: {
                colourTemp = LEDData.coloursRGB[0];
                // memcpy(&colourTemp, &LEDData.coloursRGB[0], sizeof(colourTemp));
                break;
            }
            default: break;
        }

        memmove(LEDData.coloursRGB, LEDData.coloursRGB + 1, LEDData.arraySize - sizeof(*LEDData.coloursRGB));
        memcpy(LEDData.coloursRGB + LEDData.ledCount - 1, &colourTemp, sizeof(colourTemp));
    }
}

static void LED_Create_Repeating(const LED_Colour_HSV* const colours[], const int numberOfColours) {
    LED_Colour_HSV colour;
    int offset = LEDData.currentOffset;

    if (LEDData.synchronise) offset = 0;

    for (uint32_t i = 0, j = numberOfColours - (offset % numberOfColours); i < LEDData.ledCount; i++, j++) {
        j %= numberOfColours;
        colour.hue = colours[j]->hue;
        colour.saturation = colours[j]->saturation;
        colour.value = colours[j]->value * LEDData.brightness / 255;

        HSVtoRGB(&LEDData.coloursRGB[i], &colour);
        // printf("%d, h:%d, s: %d, v: %d, r: %d, g: %d, b: %d\n", i, colour.hue, colour.saturation, colour.value, LEDData.coloursRGB[i].red, LEDData.coloursRGB[i].green, LEDData.coloursRGB[i].blue);
    }
}

static void LED_Create_Wave(const LED_Colour_HSV* const colours[], const int numberOfColours) {
    int colourIndex0 = -1, colourIndex1 = 0;
    for (uint32_t i = 0, j = 0, start = 0, end = 0; i < LEDData.ledCount; i++) {
        if (i == end) {
            j++;
            colourIndex0++;
            colourIndex1++;
            start = end;

            // find the next end, rounded divison
            if (numberOfColours & 1) {
                end = (j * LAST_LED_INDEX + 1 + (numberOfColours / 2)) / numberOfColours;
            } else {
                end = (j * LAST_LED_INDEX + (numberOfColours / 2)) / numberOfColours;
            }

            // if it's the last one then set the value of end to the next
            if (end == LAST_LED_INDEX) {
                end = LEDData.ledCount;
                colourIndex1 = 0;
            }
        }

        LED_Colour_HSV colour;

        if (!(colours[colourIndex0]->value && colours[colourIndex1]->value)) {
            if (!colours[colourIndex0]->value) {
                colour.hue = colours[colourIndex1]->hue;
            } else {
                colour.hue = colours[colourIndex0]->hue;
            }
        } else if (colours[colourIndex0]->saturation && colours[colourIndex1]->saturation)  {
            if ((colours[colourIndex0]->hue - colours[colourIndex1]->hue <= 128
                    && colours[colourIndex0]->hue - colours[colourIndex1]->hue >= 0)
                    || (colours[colourIndex1]->hue - colours[colourIndex0]->hue <= 128
                    && colours[colourIndex1]->hue - colours[colourIndex0]->hue >= 0)) {
                    colour.hue = colours[colourIndex0]->hue + difference(i, start) * (colours[colourIndex1]->hue - colours[colourIndex0]->hue) / difference(start, end);
            } else {
                if (colours[colourIndex1]->hue - colours[colourIndex0]->hue >= 0) {
                        colour.hue = colours[colourIndex0]->hue + difference(i, start) * ~((uint8_t) (colours[colourIndex0]->hue - colours[colourIndex1]->hue)) / difference(start, end);
                } else {
                    colour.hue = colours[colourIndex0]->hue + difference(i, start) * ((uint8_t) (colours[colourIndex1]->hue - colours[colourIndex0]->hue)) / difference(start, end);
                }
            }
        } else {
            if (colours[colourIndex0]->saturation) {
                colour.hue = colours[colourIndex0]->hue;
            } else {
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
        //     if (colours[colourIndex1]->hue - colours[colourIndex0]->hue >= 0) {
        //         hue = colours[colourIndex0]->hue + difference(i, start) * -(colours[colourIndex0]->hue - colours[colourIndex1]->hue + 240) / difference(start, end);
        //     } else {
        //         hue = colours[colourIndex0]->hue + difference(i, start) * -(colours[colourIndex0]->hue - colours[colourIndex1]->hue - 240) / difference(start, end);
        //     }
        // }

        // if (hue >= 0) {
        //     colour.hue = hue;
        // } else {
        //     colour.hue = 240 + hue;
        // }

        colour.saturation = colours[colourIndex0]->saturation + difference(i, start) * (colours[colourIndex1]->saturation - colours[colourIndex0]->saturation) / difference(start, end);
        // colour.value = colours[colourIndex0]->value + difference(i, start) * (colours[colourIndex1]->value - colours[colourIndex0]->value) / difference(start, end);
        // colour.value = (colours[colourIndex0]->value + difference(i, start) * (colours[colourIndex1]->value - colours[colourIndex0]->value) / difference(start, end)) * brightness / 255;
        colour.value = (colours[colourIndex0]->value + difference(i, start) * (colours[colourIndex1]->value - colours[colourIndex0]->value) / difference(start, end)) * LEDData.brightness / 255;
        // uint32_t contribution = (difference(i, start) * colours[colourIndex0]->value / difference(start, end) * difference(i, start) * colours[colourIndex0]->value / difference(start, end))
        //                     + (difference(i, end) * colours[colourIndex1]->value / difference(start, end) * difference(i, end) * colours[colourIndex1]->value / difference(start, end));
        // colour.value = fast_square_root((uint16_t) contribution);
        // printf("%d, %d, from %d to %d, ", (uint8_t) colour.hue, difference(i, start), colours[colourIndex0]->hue, colours[colourIndex1]->hue);
        // printf("dif: %d, dif: %d, d: %d, c: %d\n\r", difference(i, start) * colours[colourIndex0]->value / difference(start, end), difference(i, end) * colours[colourIndex1]->value / difference(start, end), contribution, colour.value);

        uint32_t offset = LEDData.currentOffset;
        if (LEDData.synchronise) offset = 0;

        uint32_t index = i + offset;
        if (index >= LEDData.ledCount) {
            index -= LEDData.ledCount;
        }
        HSVtoRGB(&LEDData.coloursRGB[index], &colour);
        //
        // printf("h: %d, s: %d, v: %d, ", (uint8_t) colour.hue, colour.saturation, colour.value);
        // printf("2: %d,%d,%lu,%lu, h:%d, s: %d, v: %d, r: %d, g: %d, b: %d\n", LEDData.brightness, LEDData.currentOffset, i, index, colour.hue, colour.saturation, colour.value, LEDData.coloursRGB[i].red, LEDData.coloursRGB[i].green, LEDData.coloursRGB[i].blue);

        //
        // printf("r: %d, g: %d, b: %d\n", *LEDData.coloursRGB[i]->red, *LEDData.coloursRGB[i]->green, *LEDData.coloursRGB[i]->blue);
    }
}

static void LED_Create_Pattern(void) {
// printf("0: %d\n", LEDData.currentOffset);

    switch (LEDData.ledPattern->animationType) {
        case Animation_Repeating: {
            LED_Create_Repeating(LEDData.ledPattern->colours, LEDData.ledPattern->numberOfColours);
            break;
        }
        case Animation_Wave: {
            LED_Create_Wave(LEDData.ledPattern->colours, LEDData.ledPattern->numberOfColours);
            break;
        }
        default: break;
    }

    for (uint32_t i = 0; i < LEDData.ledCount; i++) {
        memcpy(LEDData.coloursRGBCopy, LEDData.coloursRGB, LEDData.arraySize);
    }

    // LEDData.currentOffset--;
    // LED_Shift_Forward();

    // LED_Comms_Refresh_Data();
}

static void print_values(void) {
    for (uint32_t i = 0; i < LEDData.ledCount; i++) {
        if (!(i % 4)) {
            printf("\n");
        }
        printf("{%3u, %3u, %3u}, ", LEDData.coloursRGB[i].red, LEDData.coloursRGB[i].green, LEDData.coloursRGB[i].blue);
    }
    printf("\n");
}

static int difference(int num1, int num2) {
    return num1 > num2 ? num1 - num2 : num2 - num1;
}

uint16_t LED_Get_Period(void) {
    return LEDData.period;
}

uint8_t LED_Get_Brightness(void) {
    return LEDData.brightness;
}

enum Direction LED_Get_Direction(void) {
    return LEDData.direction;
}

bool LED_Get_Synchronise(void) {
    return LEDData.synchronise;
}

void LED_Init(void) {
    LEDData.ledCount = NUMBER_OF_LEDS;
    LEDData.arraySize = LEDData.ledCount * 3;

    LEDData.coloursHSV = malloc(LEDData.arraySize);
    LEDData.coloursRGB = malloc(LEDData.arraySize);
    LEDData.coloursRGBCopy = malloc(LEDData.arraySize);

    LEDData.arraySize = LEDData.ledCount * 3;
    LEDData.ledPattern = &customPattern;

    LEDConfig_Queue = xQueueCreate(1, sizeof(LED_Pattern*));
    LEDPeriod_Queue = xQueueCreate(1, sizeof(uint16_t));
    LEDBrightness_Queue = xQueueCreate(1, sizeof(uint8_t));
    LEDReverse_Queue = xQueueCreate(1, sizeof(bool));
    LEDSynchronise_Queue = xQueueCreate(1, sizeof(bool));
}

void LED_Task(void *pvParameters) {

    LED_Init();
    LED_Comms_Init();

    LED_Create_Pattern();

    // print_values();

    uint16_t newPeriod;
    uint16_t timer = 0;
    uint8_t newBrightness;
    bool patternChanged = true;

    while (1) {

        if (xQueueReceive(LEDPeriod_Queue, &newPeriod, 0)) {
            LEDData.period = newPeriod;
            // printf("New period: %d\n", LEDData.period);
        }

        static bool direction;
        if (xQueueReceive(LEDReverse_Queue, &direction, 0)) {
            LEDData.direction = direction;
            // printf("New direction: %d\n", LEDData.direction);
        }

        static bool synchronise;
        if (xQueueReceive(LEDSynchronise_Queue, &synchronise, 0)) {
            if (LEDData.synchronise != synchronise) {
                LEDData.synchronise = synchronise;
                patternChanged = true;
                LED_Create_Pattern();
            }
            // printf("New sync: %d\n", LEDData.synchronise);
        }

        if (LEDData.period && timer >= LEDData.period && LEDData.ledPattern->numberOfColours != 1) {
            timer = 0;
            patternChanged = true;

            if (LEDData.ledPattern->numberOfColours > 1) {
                if (LEDData.direction == Forwards) {
                    LED_Shift_Forward();
                } else {
                    LED_Shift_Backward();
                }
            }

        } else if (xQueueReceive(LEDConfig_Queue, &LEDData.ledPattern, 0)) {
            // printf("h: %d, s: %d, v: %d, \n", LEDData.ledPattern->colours[0]->hue, LEDData.ledPattern->colours[0]->saturation, LEDData.ledPattern->colours[0]->value);
            // printf("h: %d, s: %d, v: %d, \n", customColours[0]->hue, customColours[0]->saturation, customColours[0]->value);
            patternChanged = true;
            LED_Create_Pattern();
        } else if (xQueueReceive(LEDBrightness_Queue, &newBrightness, 0)) {
            if (LEDData.brightness != newBrightness) {
                LEDData.brightness = newBrightness;
                patternChanged = true;
                LED_Create_Pattern();
            }
        }

        if (patternChanged) LED_Comms_Send(LEDData.coloursRGB, LEDData.arraySize);

        vTaskDelay(1);

        timer++;
        patternChanged = false;
    }
}

// Very inacurate only up to 255
// static uint8_t fast_square_root(int number) {
//     for (uint32_t i = 17; i < 256; i += 17) {
//         if (number <= square_lookup[i]) {
//             while (number <= square_lookup[i]) {
//                 if (number >= square_lookup[i]) return (uint8_t) i;
//                 i--;
//             }
//         }
//     }
//
//     return 0;
// }
