#include "Colour_Defines.h"
#include "Pattern_Defines.h"
#include "LED.h"

static LED_Colour* RainbowColours[3];
static LED_Pattern Rainbow;

static LED_Colour* RedGreenBlueColours[3];
static LED_Pattern RedGreenBlue;

static LED_Colour* AquaWaveColours[4];
static LED_Pattern AquaWave;

static LED_Colour* InfernoWaveColours[4];
static LED_Pattern InfernoWave;

static LED_Colour* BlackColours[1];
static LED_Pattern Black;

static LED_Colour* LuxembourgColours[3];
static LED_Pattern Luxembourg;

static LED_Colour* ChristmasColours[3];
static LED_Pattern Christmas;

static LED_Colour* WhiteColours[1];
static LED_Pattern White;

LED_Pattern* Pattern_Off = &Black;

char* Pattern_Pattern_Names[NUMBER_OF_PRESETS] = {PRESET0_NAME, PRESET1_NAME, PRESET2_NAME, PRESET3_NAME, PRESET4_NAME, PRESET5_NAME};
LED_Pattern* Pattern_Presets[NUMBER_OF_PRESETS] = {&Rainbow,
    &RedGreenBlue, &AquaWave, &InfernoWave, &Black, &Christmas};

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
    Rainbow.syncronised = false;

    RedGreenBlueColours[0] = Colour_Red;
    RedGreenBlueColours[1] = Colour_Green;
    RedGreenBlueColours[2] = Colour_Blue;
    RedGreenBlue.patternType = Pattern_Type_Repeating;
    RedGreenBlue.period = 1000;
    RedGreenBlue.colours = RedGreenBlueColours;
    RedGreenBlue.numberOfColours = 3;
    RedGreenBlue.direction = 1;
    RedGreenBlue.cycles = 1;
    RedGreenBlue.syncronised = false;

    AquaWaveColours[0] = Colour_Blue;
    AquaWaveColours[1] = Colour_Cyan;
    AquaWaveColours[2] = Colour_Blue;
    AquaWaveColours[3] = Colour_Cyan;
    AquaWave.patternType = Pattern_Type_Wave;
    AquaWave.period = 100;
    AquaWave.colours = AquaWaveColours;
    AquaWave.numberOfColours = 4;
    AquaWave.direction = 1;
    AquaWave.cycles = 1;
    AquaWave.syncronised = false;

    InfernoWaveColours[0] = Colour_Red;
    InfernoWaveColours[1] = Colour_Orange;
    InfernoWaveColours[2] = Colour_Red;
    InfernoWaveColours[3] = Colour_Orange;
    InfernoWave.patternType = Pattern_Type_Wave;
    InfernoWave.period = 100;
    InfernoWave.colours = InfernoWaveColours;
    InfernoWave.numberOfColours = 4;
    InfernoWave.direction = 1;
    InfernoWave.cycles = 1;
    InfernoWave.syncronised = false;

    BlackColours[0] = Colour_Black;
    Black.patternType = Pattern_Type_Repeating;
    Black.period = 0;
    Black.colours = BlackColours;
    Black.numberOfColours = 1;
    Black.direction = 1;
    Black.cycles = 1;
    Black.syncronised = false;

    LuxembourgColours[0] = Colour_Red;
    LuxembourgColours[1] = Colour_White;
    LuxembourgColours[2] = Colour_Bloe;
    Luxembourg.patternType = Pattern_Type_Repeating;
    Luxembourg.period = 1000;
    Luxembourg.colours = LuxembourgColours;
    Luxembourg.numberOfColours = 3;
    Luxembourg.direction = 1;
    Luxembourg.cycles = 1;
    Luxembourg.syncronised = false;

    WhiteColours[0] = Colour_White;
    White.patternType = Pattern_Type_Repeating;
    White.period = 0;
    White.colours = WhiteColours;
    White.numberOfColours = 1;
    White.direction = 1;
    White.cycles = 1;
    White.syncronised = false;

    ChristmasColours[0] = Colour_Red;
    ChristmasColours[1] = Colour_Green;
    // ChristmasColours[2] = Colour_White;
    Christmas.patternType = Pattern_Type_Repeating;
    Christmas.period = 1000;
    Christmas.colours = ChristmasColours;
    Christmas.numberOfColours = 2;
    Christmas.direction = 1;
    Christmas.cycles = 1;
    Christmas.syncronised = false;
}
