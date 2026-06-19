#pragma once

#include <Arduino.h>

class WaterLevelSensor
{
public:
    void begin();
    void update();

    bool isWaterLow() const;
    int rawPinState() const;

private:
    bool lastRawWaterLow = false;
    bool debouncedWaterLow = false;
    unsigned long lastRawChangedAt = 0;
    bool initialized = false;

    bool readRawWaterLow() const;
};
