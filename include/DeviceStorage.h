#pragma once

#include <Arduino.h>
#include <Preferences.h>

struct StoredWifiCredentials
{
    String ssid;
    String password;
    bool valid = false;
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

private:
    Preferences preferences;
    bool initialized = false;

    static constexpr const char *StorageNamespace = "fountain";
    static constexpr const char *WifiSsidKey = "wifi_ssid";
    static constexpr const char *WifiPasswordKey = "wifi_pass";
    static constexpr const char *ProvisioningKey = "provision";

    void removeKeyIfPresent(const char *key);
};
