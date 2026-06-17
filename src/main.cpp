#include <Arduino.h>
#include <WiFi.h>

#include "DeviceStorage.h"
#include "FirmwareInfo.h"
#include "FountainController.h"
#include "HardwareOutputs.h"
#include "LocalControls.h"
#include "SetupPortal.h"
#include "WaterLevelSensor.h"
#include "WifiManager.h"
#include "WifiReset.h"

namespace
{
DeviceStorage deviceStorage;
FountainController fountainController;
HardwareOutputs hardwareOutputs;
LocalControls localControls;
SetupPortal setupPortal;
WaterLevelSensor waterLevelSensor;
WifiManager wifiManager;
WifiReset wifiReset;

unsigned long lastStatusLogAt = 0;
constexpr unsigned long StatusLogIntervalMs = 5000;

void printBootInfo()
{
    Serial.println();
    Serial.println("========================================");
    Serial.println(FirmwareInfo::ProductName);
    Serial.print("Board: ");
    Serial.println(FirmwareInfo::BoardName);
    Serial.print("Firmware: ");
    Serial.println(FirmwareInfo::Version);
    Serial.println("Milestone: Wi-Fi onboarding and local-first runtime");
    Serial.println("========================================");
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

void updateLocalRuntime()
{
    waterLevelSensor.update();
    localControls.update();

    fountainController.update();
    processLocalControls();
    fountainController.update();

    hardwareOutputs.update();
}

void reportPendingStateChange()
{
    if (!fountainController.consumeStateChanged())
    {
        return;
    }

    Serial.println("State changed; ready for future Laravel state sync.");
}

void startNetworkRuntime()
{
    const bool resetRequested = wifiReset.checkOnBoot(deviceStorage);
    const StoredWifiCredentials credentials = deviceStorage.loadWifiCredentials();

    bool provisioningRequired = resetRequested || deviceStorage.isProvisioningRequired();

    if (!credentials.valid)
    {
        provisioningRequired = true;
        deviceStorage.setProvisioningRequired(true);
    }

    if (provisioningRequired)
    {
        Serial.println("Provisioning required. Starting setup hotspot directly.");
        setupPortal.begin(deviceStorage);
        return;
    }

    wifiManager.begin(credentials);
}

void updateNetworkRuntime()
{
    if (setupPortal.isActive())
    {
        setupPortal.update();
        return;
    }

    wifiManager.update();
}

void logRuntimeStatus()
{
    Serial.println();
    Serial.println("Fountain runtime status:");

    Serial.print(" - pump: ");
    Serial.println(hardwareOutputs.isPumpEnabled() ? "ON" : "OFF");

    Serial.print(" - COB: ");
    Serial.println(hardwareOutputs.isCobEnabled() ? "ON" : "OFF");

    Serial.print(" - NeoPixels: ");
    Serial.println(hardwareOutputs.areNeoPixelsEnabled() ? "ON" : "OFF");

    Serial.print(" - water state: ");
    Serial.println(waterLevelSensor.isWaterLow() ? "LOW WATER" : "WATER OK");

    Serial.print(" - last control source: ");
    Serial.println(fountainController.lastControlSourceName());

    if (setupPortal.isActive())
    {
        Serial.print(" - network mode: setup_portal, credential_state=");
        Serial.println(setupPortal.credentialStateName());
        return;
    }

    Serial.print(" - Wi-Fi state: ");
    Serial.println(wifiManager.stateName());

    if (wifiManager.isConnected())
    {
        Serial.print(" - IP: ");
        Serial.println(WiFi.localIP());
        Serial.print(" - RSSI dBm: ");
        Serial.println(WiFi.RSSI());
    }
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
    fountainController.begin(hardwareOutputs, waterLevelSensor);

    deviceStorage.begin();
    startNetworkRuntime();

    Serial.println("Local controls and water safety remain active in every network mode.");

    reportPendingStateChange();
    logRuntimeStatus();
    lastStatusLogAt = millis();
}

void loop()
{
    updateLocalRuntime();
    updateNetworkRuntime();
    reportPendingStateChange();

    const unsigned long now = millis();

    if (now - lastStatusLogAt >= StatusLogIntervalMs)
    {
        lastStatusLogAt = now;
        logRuntimeStatus();
    }

    delay(5);
}
