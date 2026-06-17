#pragma once

#include <Arduino.h>

class HardwareOutputs;

class HardwareValidator
{
public:
    void begin(HardwareOutputs &outputs);
    void update(HardwareOutputs &outputs);

private:
    enum class NeoPixelStep : uint8_t
    {
        Off,
        Red,
        Green,
        Blue,
        White
    };

    NeoPixelStep currentStep = NeoPixelStep::Off;
    unsigned long lastStepAt = 0;
    bool started = false;

    static constexpr unsigned long StepIntervalMs = 3000;
    static constexpr uint8_t TestLevel = 32;

    void applyCurrentStep(HardwareOutputs &outputs);
    void advanceStep();
};
