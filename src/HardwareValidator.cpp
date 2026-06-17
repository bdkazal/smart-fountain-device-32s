#include "HardwareValidator.h"

#include "HardwareOutputs.h"

namespace
{
constexpr uint8_t WhiteTestLevel = 10;
}

void HardwareValidator::begin(HardwareOutputs &outputs)
{
    currentStep = NeoPixelStep::Off;
    activePulse = OutputPulse::None;
    lastStepAt = millis();
    started = true;

    Serial.println("NeoPixel validation sequence enabled.");
    Serial.println("Sequence: OFF -> RED -> GREEN -> BLUE -> WHITE");
    Serial.println("Each step lasts 3 seconds at low test intensity.");
    Serial.println();
    Serial.println("Manual GPIO signal-test commands:");
    Serial.println("  p = GPIO25 HIGH for 2 seconds");
    Serial.println("  c = GPIO26 HIGH for 2 seconds");
    Serial.println("  x = return GPIO25 and GPIO26 LOW");
    Serial.println("Run this only with external loads disconnected.");

    applyCurrentStep(outputs);
}

void HardwareValidator::update(HardwareOutputs &outputs)
{
    if (!started)
    {
        return;
    }

    handleSerialCommands(outputs);

    const unsigned long now = millis();

    if (activePulse != OutputPulse::None &&
        now - pulseStartedAt >= OutputPulseDurationMs)
    {
        stopOutputPulse(outputs);
    }

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

void HardwareValidator::handleSerialCommands(HardwareOutputs &outputs)
{
    while (Serial.available() > 0)
    {
        const char command = static_cast<char>(Serial.read());

        switch (command)
        {
        case 'p':
        case 'P':
            startOutputPulse(OutputPulse::Pump, outputs);
            break;

        case 'c':
        case 'C':
            startOutputPulse(OutputPulse::Cob, outputs);
            break;

        case 'x':
        case 'X':
            stopOutputPulse(outputs);
            Serial.println("Manual GPIO test: outputs set LOW.");
            break;

        default:
            break;
        }
    }
}

void HardwareValidator::startOutputPulse(OutputPulse pulse, HardwareOutputs &outputs)
{
    stopOutputPulse(outputs);

    activePulse = pulse;
    pulseStartedAt = millis();

    if (pulse == OutputPulse::Pump)
    {
        outputs.setPumpEnabled(true);
        Serial.println("Manual GPIO test: GPIO25 HIGH for 2 seconds.");
    }
    else if (pulse == OutputPulse::Cob)
    {
        outputs.setCobEnabled(true);
        Serial.println("Manual GPIO test: GPIO26 HIGH for 2 seconds.");
    }
}

void HardwareValidator::stopOutputPulse(HardwareOutputs &outputs)
{
    outputs.setPumpEnabled(false);
    outputs.setCobEnabled(false);

    if (activePulse != OutputPulse::None)
    {
        Serial.println("Manual GPIO test: outputs returned LOW.");
    }

    activePulse = OutputPulse::None;
}
