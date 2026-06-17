#pragma once

#include <Arduino.h>

class HardwareOutputs;

class HardwareValidator
{
public:
    void begin(HardwareOutputs &outputs);
    void update(HardwareOutputs &outputs);

private:
    static constexpr uint8_t ColorTestLevel = 32;
    static constexpr uint8_t WhiteTestLevel = 10;

    void handleSerialCommands(HardwareOutputs &outputs);
    void printHelp() const;
    void printOutputState(const HardwareOutputs &outputs) const;
    void setNeoPixelColor(HardwareOutputs &outputs, uint8_t red, uint8_t green, uint8_t blue, const char *name);
};
