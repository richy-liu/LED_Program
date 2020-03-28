#include "Colour_Defines.h"
#include "LED.h"

LED_Colour Red = {
    .hue = (0 *255+180)/360,
    .saturation = 255,
    .value = 255
};

LED_Colour Green = {
    .hue = (120 *255+180)/360,
    .saturation = 255,
    .value = 255
};

LED_Colour Blue = {
    .hue = (240 *255+180)/360,
    .saturation = 255,
    .value = 255
};

LED_Colour Cyan = {
    .hue = (180 *255+180)/360,
    .saturation = 255,
    .value = 255
};

LED_Colour Magenta = {
    .hue = (300 *255+180)/360,
    .saturation = 255,
    .value = 255
};

LED_Colour Yellow = {
    .hue = (60 *255+180)/360,
    .saturation = 255,
    .value = 255
};

LED_Colour Black = {
    .hue = 0,
    .saturation = 255,
    .value = 0
};

LED_Colour *Colour_Red = &Red;
LED_Colour *Colour_Green = &Green;
LED_Colour *Colour_Blue = &Blue;
LED_Colour *Colour_Cyan = &Cyan;
LED_Colour *Colour_Magenta = &Magenta;
LED_Colour *Colour_Yellow = &Yellow;
LED_Colour *Colour_Black = &Black;
