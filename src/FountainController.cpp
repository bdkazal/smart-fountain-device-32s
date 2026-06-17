#include "FountainController.h"

#include "HardwareOutputs.h"
#include "WaterLevelSensor.h"

void FountainController::begin(HardwareOutputs &hardwareOutputs, WaterLevelSensor &sensor)
{
    outputs = &hardwareOutputs;
    waterLevelSensor = &sensor;
    source = ControlSource::Boot;
    stateChanged = true;
}

void FountainController::update()
{
    if (outputs == nullptr || waterLevelSensor == nullptr)
    {
        return;
    }

    if (waterLevelSensor->isWaterLow() && outputs->isPumpEnabled())
    {
        outputs->setPumpEnabled(false);
        markStateChanged(ControlSource::Safety);
        Serial.println("Water safety forced pump OFF.");
    }
}

void FountainController::togglePumpFromLocalButton()
{
    if (outputs != nullptr)
    {
        requestPumpState(!outputs->isPumpEnabled(), ControlSource::LocalButton);
    }
}

void FountainController::toggleCobFromLocalButton()
{
    if (outputs != nullptr)
    {
        requestCobState(!outputs->isCobEnabled(), ControlSource::LocalButton);
    }
}

bool FountainController::requestPumpState(bool enabled, ControlSource requestedSource)
{
    if (outputs == nullptr || waterLevelSensor == nullptr)
    {
        return false;
    }

    if (enabled && waterLevelSensor->isWaterLow())
    {
        if (outputs->isPumpEnabled())
        {
            outputs->setPumpEnabled(false);
        }

        markStateChanged(ControlSource::Safety);
        Serial.print("Pump request rejected by low-water safety. source=");
        Serial.println(sourceName(requestedSource));
        return false;
    }

    if (outputs->isPumpEnabled() != enabled)
    {
        outputs->setPumpEnabled(enabled);
        markStateChanged(requestedSource);
    }

    Serial.print("Pump state: ");
    Serial.print(outputs->isPumpEnabled() ? "ON" : "OFF");
    Serial.print(" source=");
    Serial.println(sourceName(requestedSource));
    return true;
}

void FountainController::requestCobState(bool enabled, ControlSource requestedSource)
{
    if (outputs == nullptr)
    {
        return;
    }

    if (outputs->isCobEnabled() != enabled)
    {
        outputs->setCobEnabled(enabled);
        markStateChanged(requestedSource);
    }

    Serial.print("COB state: ");
    Serial.print(outputs->isCobEnabled() ? "ON" : "OFF");
    Serial.print(" source=");
    Serial.println(sourceName(requestedSource));
}

void FountainController::requestNeoPixelState(
    bool enabled,
    uint8_t red,
    uint8_t green,
    uint8_t blue,
    ControlSource requestedSource)
{
    if (outputs == nullptr)
    {
        return;
    }

    const bool enabledChanged = outputs->areNeoPixelsEnabled() != enabled;
    const bool colorChanged =
        outputs->neoPixelRed() != red ||
        outputs->neoPixelGreen() != green ||
        outputs->neoPixelBlue() != blue;

    if (!enabled && enabledChanged)
    {
        outputs->setNeoPixelsEnabled(false);
    }

    if (colorChanged)
    {
        outputs->setNeoPixelColor(red, green, blue);
    }

    if (enabled && enabledChanged)
    {
        outputs->setNeoPixelsEnabled(true);
    }

    if (enabledChanged || colorChanged)
    {
        markStateChanged(requestedSource);
    }

    Serial.print("NeoPixel state: ");
    Serial.print(outputs->areNeoPixelsEnabled() ? "ON" : "OFF");
    Serial.print(" color=");
    Serial.print(outputs->neoPixelRed());
    Serial.print(",");
    Serial.print(outputs->neoPixelGreen());
    Serial.print(",");
    Serial.print(outputs->neoPixelBlue());
    Serial.print(" source=");
    Serial.println(sourceName(requestedSource));
}

bool FountainController::consumeStateChanged()
{
    const bool changed = stateChanged;
    stateChanged = false;
    return changed;
}

ControlSource FountainController::lastControlSource() const
{
    return source;
}

const char *FountainController::lastControlSourceName() const
{
    return sourceName(source);
}

void FountainController::markStateChanged(ControlSource newSource)
{
    source = newSource;
    stateChanged = true;
}

const char *FountainController::sourceName(ControlSource value) const
{
    switch (value)
    {
    case ControlSource::Boot:
        return "boot";
    case ControlSource::Restore:
        return "restore";
    case ControlSource::LocalButton:
        return "local_button";
    case ControlSource::Laravel:
        return "laravel";
    case ControlSource::Schedule:
        return "schedule";
    case ControlSource::Safety:
        return "safety";
    }

    return "unknown";
}