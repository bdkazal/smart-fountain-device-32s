#include <Arduino.h>

#include "FirmwareInfo.h"
#include "FountainController.h"
#include "HardwareOutputs.h"
#include "HardwarePins.h"
#include "LocalControls.h"
#include "WaterLevelSensor.h"

namespace
{
FountainController fountainController;
HardwareOutputs hardwareOutputs;
LocalControls localControls;
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
    Serial.println("Milestone: local controls and shared fountain controller");
    Serial.println("========================================");
}

void initializeWifiResetInput()
{
    pinMode(HardwarePins::WifiResetButton, INPUT_PULLUP);

    Serial.print("Wi-Fi reset button initialized on GPIO");
    Serial.println(static_cast<int>(HardwarePins::WifiResetButton));
}

void processLocalControls()
{
    if (localControls.consumePumpToggleRequest())
    {
        fountainController.togglePumpFromLocalButton();
    }

    if (localControls.consumeCobToggleRequest())
    {
        fountainController.toggleCobFromLocalButton();
    }
}

void logHardwareStatus()
{
    Serial.println();
    Serial.println("Fountain state:");

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

    Serial.print(" - last control source: ");
    Serial.println(fountainController.lastControlSourceName());

    Serial.print(" - reset button raw: ");
    Serial.println(digitalRead(HardwarePins::WifiResetButton));
}

void reportPendingStateChange()
{
    if (!fountainController.consumeStateChanged())
    {
        return;
    }

    Serial.println("State changed; ready for future Laravel state sync.");
    logHardwareStatus();
}
}

void setup()
{
    Serial.begin(115200);
    delay(1000);

    printBootInfo();
    hardwareOutputs.begin();
    waterLevelSensor.begin();
    localControls.begin();
    initializeWifiResetInput();
    fountainController.begin(hardwareOutputs, waterLevelSensor);

    Serial.println("Local fountain controls ready.");
    Serial.println("Pump button: GPIO18 to GND");
    Serial.println("COB button: GPIO19 to GND");

    reportPendingStateChange();
    lastStatusLogAt = millis();
}

void loop()
{
    waterLevelSensor.update();
    localControls.update();

    fountainController.update();
    processLocalControls();
    fountainController.update();

    hardwareOutputs.update();
    reportPendingStateChange();

    const unsigned long now = millis();

    if (now - lastStatusLogAt >= StatusLogIntervalMs)
    {
        lastStatusLogAt = now;
        logHardwareStatus();
    }

    delay(5);
}
