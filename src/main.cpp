#include <Arduino.h>

#include "FirmwareInfo.h"
#include "HardwareOutputs.h"
#include "HardwarePins.h"
#include "HardwareValidator.h"
#include "WaterLevelSensor.h"

namespace
{
HardwareOutputs hardwareOutputs;
HardwareValidator hardwareValidator;
WaterLevelSensor waterLevelSensor;
unsigned long lastStatusLogAt = 0;
constexpr unsigned long StatusLogIntervalMs = 2000;

void printBootInfo()
{
    Serial.println();
    Serial.println("========================================");
    Serial.println(FirmwareInfo::ProductName);
    Serial.print("Board: ");
    Serial.println(FirmwareInfo::BoardName);
    Serial.print("Firmware: ");
    Serial.println(FirmwareInfo::Version);
    Serial.println("Milestone: project foundation and hardware validation");
    Serial.println("========================================");
}

void initializeInputs()
{
    pinMode(HardwarePins::WifiResetButton, INPUT_PULLUP);

    Serial.print("Wi-Fi reset button initialized on GPIO");
    Serial.println(static_cast<int>(HardwarePins::WifiResetButton));
}

void enforceLocalWaterSafety()
{
    if (waterLevelSensor.isWaterLow() && hardwareOutputs.isPumpEnabled())
    {
        hardwareOutputs.setPumpEnabled(false);
        Serial.println("Water safety forced pump OFF.");
    }
}

void logHardwareStatus()
{
    Serial.println();
    Serial.println("Hardware validation status:");

    Serial.print(" - pump: ");
    Serial.println(hardwareOutputs.isPumpEnabled() ? "ON" : "OFF");

    Serial.print(" - COB: ");
    Serial.println(hardwareOutputs.isCobEnabled() ? "ON" : "OFF");

    Serial.print(" - NeoPixels: ");
    Serial.println(hardwareOutputs.areNeoPixelsEnabled() ? "ON" : "OFF");

    Serial.print(" - water GPIO raw: ");
    Serial.println(waterLevelSensor.rawPinState());

    Serial.print(" - water state: ");
    Serial.println(waterLevelSensor.isWaterLow() ? "LOW WATER" : "WATER OK");

    Serial.print(" - reset button raw: ");
    Serial.println(digitalRead(HardwarePins::WifiResetButton));
}
}

void setup()
{
    Serial.begin(115200);
    delay(1000);

    printBootInfo();
    hardwareOutputs.begin();
    waterLevelSensor.begin();
    initializeInputs();
    hardwareValidator.begin(hardwareOutputs);

    Serial.println("Foundation hardware validation firmware ready.");
    logHardwareStatus();
    lastStatusLogAt = millis();
}

void loop()
{
    waterLevelSensor.update();
    enforceLocalWaterSafety();
    hardwareValidator.update(hardwareOutputs);
    hardwareOutputs.update();

    const unsigned long now = millis();

    if (now - lastStatusLogAt >= StatusLogIntervalMs)
    {
        lastStatusLogAt = now;
        logHardwareStatus();
    }

    delay(5);
}
