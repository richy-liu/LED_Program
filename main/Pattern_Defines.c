#include "Colour_Defines.h"
#include "Pattern_Defines.h"
#include "LED.h"

LED_Pattern *Pattern_Rainbow = NULL;
LED_Pattern *Pattern_Red_Green_Blue = NULL;
LED_Pattern *Pattern_Aqua_Wave = NULL;

LED_Colour* RainbowColours[3];
LED_Pattern Rainbow;

LED_Colour* RedGreenBlueColours[3];
LED_Pattern RedGreenBlue;

LED_Colour* AquaWaveColours[2];
LED_Pattern AquaWave;

void Pattern_Initialise(void)
{
    // static LED_Colour* RainbowColours[3] = {Colour_Red, Colour_Green, Colour_Red};
    RainbowColours[0] = Colour_Red;
    RainbowColours[1] = Colour_Green;
    RainbowColours[2] = Colour_Blue;

    Rainbow.patternType = Pattern_Wave;
    Rainbow.period = 100;
    Rainbow.colours = RainbowColours;
    Rainbow.numberOfColours = 3;
    Rainbow.direction = 1;
    Rainbow.cycles = 1;

    // static LED_Colour* RedGreenBlueColours[3] = {Colour_Red, Colour_Green, Colour_Red};
    RedGreenBlueColours[0] = Colour_Red;
    RedGreenBlueColours[1] = Colour_Green;
    RedGreenBlueColours[2] = Colour_Blue;

    RedGreenBlue.patternType = Pattern_Repeating;
    RedGreenBlue.period = 1000;
    RedGreenBlue.colours = RedGreenBlueColours;
    RedGreenBlue.numberOfColours = 3;
    RedGreenBlue.direction = 1;
    RedGreenBlue.cycles = 1;

    // static LED_Colour* AquaWaveColours[2] = {Colour_Blue, Colour_Cyan};
    AquaWaveColours[0] = Colour_Blue;
    AquaWaveColours[1] = Colour_Cyan;

    AquaWave.patternType = Pattern_Wave;
    AquaWave.period = 100;
    AquaWave.colours = AquaWaveColours;
    AquaWave.numberOfColours = 2;
    AquaWave.direction = 1;
    AquaWave.cycles = 1;

    Pattern_Rainbow = &Rainbow;
    Pattern_Red_Green_Blue = &RedGreenBlue;
    Pattern_Aqua_Wave = &AquaWave;
}
