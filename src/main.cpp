#include <Arduino.h>
#include <WiFi.h>

#if __has_include("DeviceSecrets.h")
#include "DeviceSecrets.h"
#else
#include "DeviceSecrets.example.h"
#endif

#include "DeviceStorage.h"
#include "FirmwareInfo.h"
#include "FountainController.h"
#include "HardwareOutputs.h"
#include "LaravelApiClient.h"
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
LaravelApiClient laravelApiClient;
LocalControls localControls;
SetupPortal setupPortal;
WaterLevelSensor waterLevelSensor;
WifiManager wifiManager;
WifiReset wifiReset;

unsigned long lastStatusLogAt = 0;
unsigned long fountainStateSaveNotBefore = 0;
bool fountainStateSavePending = false;

constexpr unsigned long StatusLogIntervalMs = 5000;
constexpr unsigned long FountainStateSaveDelayMs = 300;
constexpr unsigned long FountainStateSaveRetryMs = 5000;

bool looksLikePlaceholder(const String &value)
{
    return value.length() == 0 ||
           value.indexOf("YOUR_") >= 0 ||
           value.indexOf("xxx") >= 0;
}

StoredWifiCredentials loadRuntimeWifiCredentials()
{
    StoredWifiCredentials credentials = deviceStorage.loadWifiCredentials();

    if (credentials.valid)
    {
        return credentials;
    }

    String secretsSsid = WIFI_SSID;
    String secretsPassword = WIFI_PASSWORD;
    secretsSsid.trim();

    if (looksLikePlaceholder(secretsSsid))
    {
        return credentials;
    }

    credentials.ssid = secretsSsid;
    credentials.password = secretsPassword;
    credentials.valid = true;

    Serial.println("Using development Wi-Fi credentials from include/DeviceSecrets.h.");
    Serial.println("These credentials are not saved to NVS; setup portal remains the production path.");

    return credentials;
}

void printBootInfo()
{
    Serial.println();
    Serial.println("========================================");
    Serial.println(FirmwareInfo::ProductName);
    Serial.print("Board: ");
    Serial.println(FirmwareInfo::BoardName);
    Serial.print("Firmware: ");
    Serial.println(FirmwareInfo::Version);
    Serial.println("Milestone: Laravel API handshake baseline");
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

StoredFountainState captureActualFountainState()
{
    StoredFountainState state;
    state.valid = true;
    state.pumpEnabled = hardwareOutputs.isPumpEnabled();
    state.cobEnabled = hardwareOutputs.isCobEnabled();
    state.neoPixelsEnabled = hardwareOutputs.areNeoPixelsEnabled();
    state.neoPixelRed = hardwareOutputs.neoPixelRed();
    state.neoPixelGreen = hardwareOutputs.neoPixelGreen();
    state.neoPixelBlue = hardwareOutputs.neoPixelBlue();
    return state;
}

void restoreStoredFountainState()
{
    const StoredFountainState state = deviceStorage.loadFountainState();

    if (!state.valid)
    {
        return;
    }

    Serial.println("Restoring stored actual state through FountainController...");

    fountainController.requestCobState(state.cobEnabled, ControlSource::Restore);
    fountainController.requestNeoPixelState(
        state.neoPixelsEnabled,
        state.neoPixelRed,
        state.neoPixelGreen,
        state.neoPixelBlue,
        ControlSource::Restore);

    // Pump restore is intentionally last. GPIO32 has already been initialized,
    // and FountainController may reject this request when water is low.
    fountainController.requestPumpState(state.pumpEnabled, ControlSource::Restore);
}

void scheduleFountainStateSave()
{
    fountainStateSavePending = true;
    fountainStateSaveNotBefore = millis() + FountainStateSaveDelayMs;
}

void persistFountainStateIfDue()
{
    if (!fountainStateSavePending)
    {
        return;
    }

    const unsigned long now = millis();

    if (static_cast<long>(now - fountainStateSaveNotBefore) < 0)
    {
        return;
    }

    if (deviceStorage.saveFountainState(captureActualFountainState()))
    {
        fountainStateSavePending = false;
        return;
    }

    // Keep the final state pending while backing off repeated storage failures.
    fountainStateSaveNotBefore = now + FountainStateSaveRetryMs;
}

void reportPendingStateChange()
{
    if (!fountainController.consumeStateChanged())
    {
        return;
    }

    Serial.println("State changed; queued for local persistence and future Laravel state sync.");
    scheduleFountainStateSave();
}

void startNetworkRuntime()
{
    const bool resetRequested = wifiReset.checkOnBoot(deviceStorage);
    const StoredWifiCredentials credentials = loadRuntimeWifiCredentials();

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

void updateLaravelRuntime()
{
    if (setupPortal.isActive())
    {
        return;
    }

    laravelApiClient.update(wifiManager.isConnected());
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
    Serial.print(hardwareOutputs.areNeoPixelsEnabled() ? "ON" : "OFF");
    Serial.print(" color=");
    Serial.print(hardwareOutputs.neoPixelRed());
    Serial.print(",");
    Serial.print(hardwareOutputs.neoPixelGreen());
    Serial.print(",");
    Serial.println(hardwareOutputs.neoPixelBlue());

    Serial.print(" - water state: ");
    Serial.println(waterLevelSensor.isWaterLow() ? "LOW WATER" : "WATER OK");

    Serial.print(" - last control source: ");
    Serial.println(fountainController.lastControlSourceName());

    Serial.print(" - state persistence: ");
    Serial.println(fountainStateSavePending ? "pending" : "saved");

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

    Serial.print(" - Laravel config fetched: ");
    Serial.println(laravelApiClient.hasFetchedConfig() ? "yes" : "no");
    Serial.print(" - Laravel heartbeat sent: ");
    Serial.println(laravelApiClient.hasSentHeartbeat() ? "yes" : "no");
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
    restoreStoredFountainState();
    laravelApiClient.begin(API_BASE_URL, DEVICE_UUID, DEVICE_API_KEY, FIRMWARE_VERSION);
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
    updateLaravelRuntime();
    reportPendingStateChange();
    persistFountainStateIfDue();

    const unsigned long now = millis();

    if (now - lastStatusLogAt >= StatusLogIntervalMs)
    {
        lastStatusLogAt = now;
        logRuntimeStatus();
    }

    delay(5);
}
