#pragma once

#include <Arduino.h>
#include <FastLED.h>

#include "HardwarePins.h"

class HardwareOutputs
{
public:
    void begin();
    void setPumpEnabled(bool enabled);
    void setCobEnabled(bool enabled);
    void setNeoPixelsEnabled(bool enabled);
    void setNeoPixelColor(uint8_t red, uint8_t green, uint8_t blue);
    void update();

    bool isPumpEnabled() const;
    bool isCobEnabled() const;
    bool areNeoPixelsEnabled() const;
    uint8_t neoPixelRed() const;
    uint8_t neoPixelGreen() const;
    uint8_t neoPixelBlue() const;

private:
    CRGB pixels[HardwarePins::NeoPixelCount];
    bool pumpEnabled = false;
    bool cobEnabled = false;
    bool neoPixelsEnabled = false;
    CRGB neoPixelColor = CRGB::Black;

    void applyDigitalOutput(gpio_num_t pin, bool activeHigh, bool enabled);
    void renderNeoPixels();
};