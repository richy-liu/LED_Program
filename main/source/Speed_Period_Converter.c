#include <Math.h>
#include "LED.h"

// uint16_t periodArray[100] = {};
uint16_t periodArray[100] = {5000, 4800, 4600, 4400, 4200, 4000, 3900, 3800, 3700, 3600,
3500, 3400, 3300, 3200, 3100, 3000, 2900, 2800, 2700, 2600,
2500, 2400, 2300, 2200, 2100, 2000, 1950, 1900, 1850, 1800,
1750, 1700, 1650, 1600, 1550, 1500, 1450, 1400, 1350, 1300,
1250, 1200, 1150, 1100, 1050, 1000, 975, 950, 925, 900,
875, 850, 825, 800, 775, 750, 725, 700, 675, 650,
625, 600, 575, 550, 525, 500, 475, 450, 425, 400,
375, 350, 325, 300, 280, 260, 240, 220, 210, 200,
180, 170, 160, 150, 140, 130, 120, 110, 100, 90,
80, 70, 60, 50, 40, 30, 20, 10, 5, 1};

// This isn't done very well, only excecuted once so i'll just leave it
void Speed_Period_Converter_Initialise(void)
{
    // for (int i = 1; i < 51; i++)
    // {
        // printf("this: %d\n", i);

        // int blocks = 3;
        // int numberPerBlock = 100 / blocks;
        //
        // double factor = pow((MAXIMUM_PERIOD / MINIMUM_PERIOD), (1.0 / blocks));
        //
        // for (int i = 0; i < blocks; i++)
        // {
        //     periodArray[99 - (i * numberPerBlock)] = (uint16_t) (pow(factor, (double) i) * MINIMUM_PERIOD);
        // }
        // periodArray[0] = MAXIMUM_PERIOD;
        //
        // for (int i = 0; i < blocks; i++)
        // {
        //     int anchor1 = i ? (99 - ((blocks - i) * numberPerBlock)) : i;
        //     int anchor2 = (99 - ((blocks - i - 1) * numberPerBlock));
        //
        //     for (int newValue = anchor1 + 1, j = 1; newValue != anchor2; newValue++, j++)
        //     {
        //         // periodArray[newValue] = periodArray[anchor1] - (periodArray[anchor1] - periodArray[anchor2] + numberPerBlock / 2) * j / numberPerBlock;
        //         periodArray[newValue] = periodArray[anchor1] - (periodArray[anchor1] - periodArray[anchor2]) * j / numberPerBlock;
        //     }
        // }

        for (int i = 0; i < 100; i++)
        {
            printf("%d, ", periodArray[i]);
        }
        printf("\n\n");
    // }

    // for (int i = 0; i < 100; i++)
    // {
    //     periodArray[99 - i] = (uint16_t) (pow(factor, (double) i) * MINIMUM_PERIOD);
    //     // Don't allow any of them to be 0
    //     if (!periodArray[99 - i]) periodArray[99 - i] = 1;
    // }
}

uint8_t Get_Speed_From_Period(uint16_t period)
{
    if (!period)
    {
        return 0;
    }

    for (int i = 1; i < 100; i++)
    {
        if (periodArray[i + 1] < period)
        return i;
    }

    return 100;
}

uint16_t Get_Period_From_Speed(uint8_t speed)
{
    if (!speed)
    {
        return 0;
    }

    return periodArray[speed - 1];
}
