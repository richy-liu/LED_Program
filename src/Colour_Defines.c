#include "Colour_Defines.h"

// The hue is normalised to 255. The calculations are done based on a 360 degree
// scaled to 255 and rounded to the nearest integer.

const LED_Colour_HSV red_HSV = {
    .hue = (0 * 255 + 180) / 360,
    .saturation = 255,
    .value = 255
};

const LED_Colour_HSV green_HSV = {
    .hue = (120 * 255 + 180) / 360,
    .saturation = 255,
    .value = 255
};

const LED_Colour_HSV blue_HSV = {
    .hue = (240 * 255 + 180) / 360,
    .saturation = 255,
    .value = 255
};

const LED_Colour_HSV bloe_HSV = {
    .hue = (200 * 255 + 180) / 360,
    .saturation = 255,
    .value = 255
};

const LED_Colour_HSV cyan_HSV = {
    .hue = (180 * 255 + 180) / 360,
    .saturation = 255,
    .value = 255
};

const LED_Colour_HSV magenta_HSV = {
    .hue = (300 * 255 + 180) / 360,
    .saturation = 255,
    .value = 255
};

const LED_Colour_HSV yellow_HSV = {
    .hue = (60 * 255 + 180) / 360,
    .saturation = 255,
    .value = 255
};

const LED_Colour_HSV black_HSV = {
    .hue = 0,
    .saturation = 255,
    .value = 0
};

const LED_Colour_HSV white_HSV = {
    .hue = 0,
    .saturation = 0,
    .value = 255
};

const LED_Colour_HSV orange_HSV = {
    .hue = (15 * 255 + 180) / 360,
    .saturation = 255,
    .value = 255
};

const LED_Colour_RGB black_RGB = {
    .red = 0,
    .green = 0,
    .blue = 0
};