#include "Colour_Defines.h"
#include "LED.h"

// The hue is normalised to 255. The calculations are done based on a 360 degree
// scaled to 255 and rounded to the nearest integer.

static LED_Colour Red = {
    .hue = (0 * 255 + 180) / 360,
    .saturation = 255,
    .value = 255
};

static LED_Colour Green = {
    .hue = (120 * 255 + 180) / 360,
    .saturation = 255,
    .value = 255
};

static LED_Colour Blue = {
    .hue = (240 * 255 + 180) / 360,
    .saturation = 255,
    .value = 255
};

static LED_Colour Bloe = {
    .hue = (200 * 255 + 180) / 360,
    .saturation = 255,
    .value = 255
};

static LED_Colour Cyan = {
    .hue = (180 * 255 + 180) / 360,
    .saturation = 255,
    .value = 255
};

static LED_Colour Magenta = {
    .hue = (300 * 255 + 180) / 360,
    .saturation = 255,
    .value = 255
};

static LED_Colour Yellow = {
    .hue = (60 * 255 + 180) / 360,
    .saturation = 255,
    .value = 255
};

static LED_Colour Black = {
    .hue = 0,
    .saturation = 255,
    .value = 0
};

static LED_Colour White = {
    .hue = 0,
    .saturation = 0,
    .value = 255
};

LED_Colour *Colour_Red = &Red;
LED_Colour *Colour_Green = &Green;
LED_Colour *Colour_Blue = &Blue;
LED_Colour *Colour_Bloe = &Bloe;
LED_Colour *Colour_Cyan = &Cyan;
LED_Colour *Colour_Magenta = &Magenta;
LED_Colour *Colour_Yellow = &Yellow;
LED_Colour *Colour_White = &White;
LED_Colour *Colour_Black = &Black;
