#include "WaterLevelSensor.h"

#include "HardwarePins.h"

void WaterLevelSensor::begin()
{
    pinMode(HardwarePins::WaterLevelSwitch, INPUT_PULLUP);

    lastRawWaterLow = readRawWaterLow();
    debouncedWaterLow = lastRawWaterLow;
    lastRawChangedAt = millis();
    initialized = true;

    Serial.print("Water-level sensor initialized on GPIO");
    Serial.print(static_cast<int>(HardwarePins::WaterLevelSwitch));
    Serial.print(" state=");
    Serial.println(debouncedWaterLow ? "LOW WATER" : "WATER OK");
}

void WaterLevelSensor::update()
{
    if (!initialized)
    {
        return;
    }

    const bool rawWaterLow = readRawWaterLow();
    const unsigned long now = millis();

    if (rawWaterLow != lastRawWaterLow)
    {
        lastRawWaterLow = rawWaterLow;
        lastRawChangedAt = now;
    }

    if (debouncedWaterLow != rawWaterLow &&
        now - lastRawChangedAt >= HardwarePins::WaterLevelDebounceMs)
    {
        debouncedWaterLow = rawWaterLow;

        Serial.print("Water-level state changed: ");
        Serial.println(debouncedWaterLow ? "LOW WATER" : "WATER OK");
    }
}

bool WaterLevelSensor::isWaterLow() const
{
    return debouncedWaterLow;
}

int WaterLevelSensor::rawPinState() const
{
    return digitalRead(HardwarePins::WaterLevelSwitch);
}

bool WaterLevelSensor::readRawWaterLow() const
{
    const int state = digitalRead(HardwarePins::WaterLevelSwitch);
    return HardwarePins::WaterLevelActiveLow ? state == LOW : state == HIGH;
}
