#pragma once

#include <Arduino.h>

class HardwareOutputs;
class WaterLevelSensor;

enum class ControlSource : uint8_t
{
    Boot,
    Restore,
    LocalButton,
    Laravel,
    Schedule,
    Safety
};

class FountainController
{
public:
    void begin(HardwareOutputs &outputs, WaterLevelSensor &waterLevelSensor);
    void update();

    void togglePumpFromLocalButton();
    void toggleCobFromLocalButton();

    bool requestPumpState(bool enabled, ControlSource source);
    void requestCobState(bool enabled, ControlSource source);
    void requestNeoPixelState(
        bool enabled,
        uint8_t red,
        uint8_t green,
        uint8_t blue,
        ControlSource source);

    bool consumeStateChanged();
    ControlSource lastControlSource() const;
    const char *lastControlSourceName() const;

private:
    HardwareOutputs *outputs = nullptr;
    WaterLevelSensor *waterLevelSensor = nullptr;
    bool stateChanged = false;
    ControlSource source = ControlSource::Boot;

    void markStateChanged(ControlSource newSource);
    const char *sourceName(ControlSource value) const;
};