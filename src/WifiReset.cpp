#include "WifiReset.h"

#include <Arduino.h>

#include "DeviceStorage.h"
#include "HardwarePins.h"
#include "WifiConfig.h"

bool WifiReset::checkOnBoot(DeviceStorage &storage)
{
    pinMode(HardwarePins::WifiResetButton, INPUT_PULLUP);

    Serial.println();
    Serial.println("Checking Wi-Fi reset button...");
    Serial.print("Wi-Fi reset GPIO: ");
    Serial.println(static_cast<int>(HardwarePins::WifiResetButton));

    if (digitalRead(HardwarePins::WifiResetButton) == HIGH)
    {
        Serial.println("Wi-Fi reset button not pressed.");
        return false;
    }

    Serial.println("Wi-Fi reset button pressed. Keep holding for 3 seconds...");
    const unsigned long startedAt = millis();

    while (millis() - startedAt < WifiConfig::ResetHoldMs)
    {
        if (digitalRead(HardwarePins::WifiResetButton) == HIGH)
        {
            Serial.println("Wi-Fi reset cancelled: button released too early.");
            return false;
        }

        delay(10);
    }

    storage.clearWifiCredentials();

    if (!storage.setProvisioningRequired(true))
    {
        Serial.println("Wi-Fi reset failed: provisioning flag could not be stored.");
        return false;
    }

    Serial.println("Wi-Fi reset confirmed.");
    Serial.println("Provisioning will remain required until valid Wi-Fi is saved.");
    return true;
}
