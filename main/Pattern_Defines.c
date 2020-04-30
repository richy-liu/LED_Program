#include "Colour_Defines.h"
#include "Pattern_Defines.h"
#include "LED.h"

static LED_Colour* RainbowColours[3];
static LED_Pattern Rainbow;

static LED_Colour* RedGreenBlueColours[3];
static LED_Pattern RedGreenBlue;

static LED_Colour* AquaWaveColours[2];
static LED_Pattern AquaWave;

static LED_Colour* BlackColours[1];
static LED_Pattern Black;

static LED_Colour* WhiteColours[1];
static LED_Pattern White;

LED_Pattern* Pattern_Off = &Black;

char* Pattern_Pattern_Names[NUMBER_OF_PRESETS] = {PRESET0_NAME, PRESET1_NAME, PRESET2_NAME, PRESET3_NAME};
LED_Pattern* Pattern_Presets[NUMBER_OF_PRESETS] = {&Rainbow,
    &RedGreenBlue, &AquaWave, &Black};

void Pattern_Initialise(void)
{
    RainbowColours[0] = Colour_Red;
    RainbowColours[1] = Colour_Green;
    RainbowColours[2] = Colour_Blue;
    Rainbow.patternType = Pattern_Type_Wave;
    Rainbow.period = 100;
    Rainbow.colours = RainbowColours;
    Rainbow.numberOfColours = 3;
    Rainbow.direction = 1;
    Rainbow.cycles = 1;

    RedGreenBlueColours[0] = Colour_Red;
    RedGreenBlueColours[1] = Colour_Green;
    RedGreenBlueColours[2] = Colour_Blue;
    RedGreenBlue.patternType = Pattern_Type_Repeating;
    RedGreenBlue.period = 1000;
    RedGreenBlue.colours = RedGreenBlueColours;
    RedGreenBlue.numberOfColours = 3;
    RedGreenBlue.direction = 1;
    RedGreenBlue.cycles = 1;

    AquaWaveColours[0] = Colour_Blue;
    AquaWaveColours[1] = Colour_Cyan;
    AquaWave.patternType = Pattern_Type_Wave;
    AquaWave.period = 100;
    AquaWave.colours = AquaWaveColours;
    AquaWave.numberOfColours = 2;
    AquaWave.direction = 1;
    AquaWave.cycles = 1;

    BlackColours[0] = Colour_Black;
    Black.patternType = Pattern_Type_Repeating;
    Black.period = 0;
    Black.colours = BlackColours;
    Black.numberOfColours = 1;
    Black.direction = 1;
    Black.cycles = 1;

    WhiteColours[0] = Colour_White;
    White.patternType = Pattern_Type_Repeating;
    White.period = 0;
    White.colours = WhiteColours;
    White.numberOfColours = 1;
    White.direction = 1;
    White.cycles = 1;

    Pattern_Off = &Black;

    // extern char* Pattern_Pattern_Names[];
}
