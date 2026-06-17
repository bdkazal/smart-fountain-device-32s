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

    enum class OutputPulse : uint8_t
    {
        None,
        Pump,
        Cob
    };

    NeoPixelStep currentStep = NeoPixelStep::Off;
    OutputPulse activePulse = OutputPulse::None;
    unsigned long lastStepAt = 0;
    unsigned long pulseStartedAt = 0;
    bool started = false;

    static constexpr unsigned long StepIntervalMs = 3000;
    static constexpr unsigned long OutputPulseDurationMs = 2000;
    static constexpr uint8_t TestLevel = 32;

    void applyCurrentStep(HardwareOutputs &outputs);
    void advanceStep();
    void handleSerialCommands(HardwareOutputs &outputs);
    void startOutputPulse(OutputPulse pulse, HardwareOutputs &outputs);
    void stopOutputPulse(HardwareOutputs &outputs);
};
