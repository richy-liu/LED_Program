#include "Pattern_Defines.h"

const LED_Colour_HSV* const rainbowColours[] = {&red_HSV, &green_HSV, &blue_HSV};
const LED_Pattern rainbowPattern = {
    .animationType = Animation_Wave,
    .numberOfColours = 3,
    .colours = &rainbowColours
};

const LED_Colour_HSV* const redGreenBlueColours[] = {&red_HSV, &green_HSV, &blue_HSV};
const LED_Pattern redGreenBluePattern = {
    .animationType = Animation_Repeating,
    .numberOfColours = 3,
    .colours = &redGreenBlueColours
};

const LED_Colour_HSV* const aquaColours[] = {&blue_HSV, &cyan_HSV, &blue_HSV, &cyan_HSV};
const LED_Pattern aquaPattern = {
    .animationType = Animation_Wave,
    .numberOfColours = 4,
    .colours = &aquaColours
};

const LED_Colour_HSV* const infernoColours[] = {&red_HSV, &orange_HSV, &red_HSV, &orange_HSV};
const LED_Pattern infernoPattern = {
    .animationType = Animation_Wave,
    .numberOfColours = 4,
    .colours = &infernoColours
};

const LED_Colour_HSV* const christmasColours[] = {&red_HSV, &green_HSV, &white_HSV};
const LED_Pattern christmasPattern = {
    .animationType = Animation_Repeating,
    .numberOfColours = 2,
    .colours = &christmasColours
};

const LED_Pattern customColourPattern = {};

const char* const Pattern_Names[NUMBER_OF_PRESETS] = {
    PRESET0_NAME, PRESET1_NAME, PRESET2_NAME, PRESET3_NAME, PRESET5_NAME
};

const LED_Pattern* const Pattern_Presets[NUMBER_OF_PRESETS] = {
    &rainbowPattern, &redGreenBluePattern, &aquaPattern, &infernoPattern, &christmasPattern
};
