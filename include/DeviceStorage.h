#pragma once

#include <Arduino.h>
#include <Preferences.h>

struct StoredWifiCredentials
{
    String ssid;
    String password;
    bool valid = false;
};

struct StoredFountainState
{
    bool valid = false;
    bool pumpEnabled = false;
    bool cobEnabled = false;
    bool neoPixelsEnabled = false;
    uint8_t neoPixelRed = 0;
    uint8_t neoPixelGreen = 0;
    uint8_t neoPixelBlue = 0;
};

class DeviceStorage
{
public:
    bool begin();

    StoredWifiCredentials loadWifiCredentials();
    bool saveWifiCredentials(const String &ssid, const String &password);
    void clearWifiCredentials();

    bool isProvisioningRequired();
    bool setProvisioningRequired(bool required);

    StoredFountainState loadFountainState();
    bool saveFountainState(const StoredFountainState &state);

private:
    Preferences preferences;
    bool initialized = false;

    static constexpr const char *StorageNamespace = "fountain";
    static constexpr const char *WifiSsidKey = "wifi_ssid";
    static constexpr const char *WifiPasswordKey = "wifi_pass";
    static constexpr const char *ProvisioningKey = "provision";
    static constexpr const char *FountainStateKey = "state_blob";

    void removeKeyIfPresent(const char *key);
};