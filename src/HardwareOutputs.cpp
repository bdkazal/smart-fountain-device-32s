#include "HardwareOutputs.h"

#include "HardwarePins.h"

void HardwareOutputs::begin()
{
    pinMode(HardwarePins::PumpOutput, OUTPUT);
    pinMode(HardwarePins::CobOutput, OUTPUT);

    setPumpEnabled(false);
    setCobEnabled(false);

    FastLED.addLeds<WS2812B, HardwarePins::NeoPixelData, GRB>(pixels, HardwarePins::NeoPixelCount);
    FastLED.setBrightness(HardwarePins::NeoPixelMaxBrightness);

    neoPixelsEnabled = false;
    neoPixelColor = CRGB::Black;
    renderNeoPixels();

    Serial.println("Hardware outputs initialized safely OFF.");
}

void HardwareOutputs::setPumpEnabled(bool enabled)
{
    pumpEnabled = enabled;
    applyDigitalOutput(HardwarePins::PumpOutput, HardwarePins::PumpActiveHigh, enabled);
}

void HardwareOutputs::setCobEnabled(bool enabled)
{
    cobEnabled = enabled;
    applyDigitalOutput(HardwarePins::CobOutput, HardwarePins::CobActiveHigh, enabled);
}

void HardwareOutputs::setNeoPixelsEnabled(bool enabled)
{
    neoPixelsEnabled = enabled;
    renderNeoPixels();
}

void HardwareOutputs::setNeoPixelColor(uint8_t red, uint8_t green, uint8_t blue)
{
    neoPixelColor = CRGB(red, green, blue);
    renderNeoPixels();
}

void HardwareOutputs::update()
{
    // Reserved for future non-blocking NeoPixel effects.
}

bool HardwareOutputs::isPumpEnabled() const
{
    return pumpEnabled;
}

bool HardwareOutputs::isCobEnabled() const
{
    return cobEnabled;
}

bool HardwareOutputs::areNeoPixelsEnabled() const
{
    return neoPixelsEnabled;
}

uint8_t HardwareOutputs::neoPixelRed() const
{
    return neoPixelColor.r;
}

uint8_t HardwareOutputs::neoPixelGreen() const
{
    return neoPixelColor.g;
}

uint8_t HardwareOutputs::neoPixelBlue() const
{
    return neoPixelColor.b;
}

void HardwareOutputs::applyDigitalOutput(gpio_num_t pin, bool activeHigh, bool enabled)
{
    const bool high = activeHigh ? enabled : !enabled;
    digitalWrite(pin, high ? HIGH : LOW);
}

void HardwareOutputs::renderNeoPixels()
{
    const CRGB color = neoPixelsEnabled ? neoPixelColor : CRGB::Black;
    fill_solid(pixels, HardwarePins::NeoPixelCount, color);
    FastLED.show();
}