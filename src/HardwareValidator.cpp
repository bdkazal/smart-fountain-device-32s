#include "HardwareValidator.h"

#include "HardwareOutputs.h"

namespace
{
constexpr uint8_t WhiteTestLevel = 10;
}

void HardwareValidator::begin(HardwareOutputs &outputs)
{
    currentStep = NeoPixelStep::Off;
    lastStepAt = millis();
    started = true;

    Serial.println("NeoPixel validation sequence enabled.");
    Serial.println("Sequence: OFF -> RED -> GREEN -> BLUE -> WHITE");
    Serial.println("Each step lasts 3 seconds at low test intensity.");

    applyCurrentStep(outputs);
}

void HardwareValidator::update(HardwareOutputs &outputs)
{
    if (!started)
    {
        return;
    }

    const unsigned long now = millis();

    if (now - lastStepAt < StepIntervalMs)
    {
        return;
    }

    lastStepAt = now;
    advanceStep();
    applyCurrentStep(outputs);
}

void HardwareValidator::applyCurrentStep(HardwareOutputs &outputs)
{
    switch (currentStep)
    {
    case NeoPixelStep::Off:
        outputs.setNeoPixelsEnabled(false);
        Serial.println("NeoPixel test: OFF");
        break;

    case NeoPixelStep::Red:
        outputs.setNeoPixelColor(TestLevel, 0, 0);
        outputs.setNeoPixelsEnabled(true);
        Serial.println("NeoPixel test: RED");
        break;

    case NeoPixelStep::Green:
        outputs.setNeoPixelColor(0, TestLevel, 0);
        outputs.setNeoPixelsEnabled(true);
        Serial.println("NeoPixel test: GREEN");
        break;

    case NeoPixelStep::Blue:
        outputs.setNeoPixelColor(0, 0, TestLevel);
        outputs.setNeoPixelsEnabled(true);
        Serial.println("NeoPixel test: BLUE");
        break;

    case NeoPixelStep::White:
        outputs.setNeoPixelColor(WhiteTestLevel, WhiteTestLevel, WhiteTestLevel);
        outputs.setNeoPixelsEnabled(true);
        Serial.println("NeoPixel test: WHITE (balanced low intensity)");
        break;
    }
}

void HardwareValidator::advanceStep()
{
    switch (currentStep)
    {
    case NeoPixelStep::Off:
        currentStep = NeoPixelStep::Red;
        break;
    case NeoPixelStep::Red:
        currentStep = NeoPixelStep::Green;
        break;
    case NeoPixelStep::Green:
        currentStep = NeoPixelStep::Blue;
        break;
    case NeoPixelStep::Blue:
        currentStep = NeoPixelStep::White;
        break;
    case NeoPixelStep::White:
        currentStep = NeoPixelStep::Off;
        break;
    }
}
