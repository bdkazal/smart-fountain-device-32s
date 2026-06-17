#include "DeviceStorage.h"

bool DeviceStorage::begin()
{
    initialized = preferences.begin(StorageNamespace, false);

    Serial.println();
    Serial.println(initialized
        ? "Device storage initialized."
        : "Device storage initialization failed.");

    return initialized;
}

StoredWifiCredentials DeviceStorage::loadWifiCredentials()
{
    StoredWifiCredentials credentials;

    if (!initialized)
    {
        Serial.println("Cannot load Wi-Fi credentials: storage is not initialized.");
        return credentials;
    }

    credentials.ssid = preferences.getString(WifiSsidKey, "");
    credentials.password = preferences.getString(WifiPasswordKey, "");
    credentials.valid = credentials.ssid.length() > 0;

    Serial.println();
    Serial.println("Loading stored Wi-Fi credentials...");

    if (credentials.valid)
    {
        Serial.print("Stored SSID: ");
        Serial.println(credentials.ssid);
    }
    else
    {
        Serial.println("No stored Wi-Fi credentials found.");
    }

    return credentials;
}

bool DeviceStorage::saveWifiCredentials(const String &ssid, const String &password)
{
    if (!initialized)
    {
        Serial.println("Cannot save Wi-Fi credentials: storage is not initialized.");
        return false;
    }

    String normalizedSsid = ssid;
    normalizedSsid.trim();

    if (normalizedSsid.length() == 0)
    {
        Serial.println("Cannot save Wi-Fi credentials: SSID is empty.");
        return false;
    }

    const size_t ssidBytes = preferences.putString(WifiSsidKey, normalizedSsid);
    const size_t passwordBytes = preferences.putString(WifiPasswordKey, password);

    if (ssidBytes == 0 || (password.length() > 0 && passwordBytes == 0))
    {
        Serial.println("Wi-Fi credential write failed.");
        return false;
    }

    const String savedSsid = preferences.getString(WifiSsidKey, "");
    const String savedPassword = preferences.getString(WifiPasswordKey, "");

    if (savedSsid != normalizedSsid || savedPassword != password)
    {
        Serial.println("Wi-Fi credential verification failed after writing.");
        return false;
    }

    if (!setProvisioningRequired(false))
    {
        Serial.println("Wi-Fi saved, but provisioning flag could not be cleared.");
        return false;
    }

    Serial.println("Wi-Fi credentials saved and verified.");
    return true;
}

void DeviceStorage::clearWifiCredentials()
{
    if (!initialized)
    {
        return;
    }

    removeKeyIfPresent(WifiSsidKey);
    removeKeyIfPresent(WifiPasswordKey);

    Serial.println("Stored Wi-Fi credentials cleared.");
    Serial.println("Other future cached device configuration remains untouched.");
}

bool DeviceStorage::isProvisioningRequired()
{
    if (!initialized)
    {
        return true;
    }

    return preferences.getBool(ProvisioningKey, false);
}

bool DeviceStorage::setProvisioningRequired(bool required)
{
    if (!initialized)
    {
        return false;
    }

    const size_t written = preferences.putBool(ProvisioningKey, required);

    if (written == 0)
    {
        return false;
    }

    return preferences.getBool(ProvisioningKey, !required) == required;
}

void DeviceStorage::removeKeyIfPresent(const char *key)
{
    if (preferences.isKey(key))
    {
        preferences.remove(key);
    }
}
