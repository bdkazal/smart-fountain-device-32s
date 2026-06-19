#include "DeviceStorage.h"

#include <cstddef>
#include <cstring>

namespace
{
constexpr uint8_t FountainStateVersion = 1;
constexpr uint8_t PumpEnabledFlag = 1U << 0;
constexpr uint8_t CobEnabledFlag = 1U << 1;
constexpr uint8_t NeoPixelsEnabledFlag = 1U << 2;

struct FountainStateBlob
{
    uint8_t version = FountainStateVersion;
    uint8_t flags = 0;
    uint8_t neoPixelRed = 0;
    uint8_t neoPixelGreen = 0;
    uint8_t neoPixelBlue = 0;
    uint8_t reserved[3] = {0, 0, 0};
    uint32_t checksum = 0;
};

static_assert(sizeof(FountainStateBlob) == 12, "Unexpected fountain state blob size.");

uint32_t calculateChecksum(const FountainStateBlob &blob)
{
    const auto *bytes = reinterpret_cast<const uint8_t *>(&blob);
    constexpr size_t checksumOffset = offsetof(FountainStateBlob, checksum);
    uint32_t crc = 0xFFFFFFFFUL;

    for (size_t index = 0; index < checksumOffset; ++index)
    {
        crc ^= bytes[index];

        for (uint8_t bit = 0; bit < 8; ++bit)
        {
            const uint32_t mask = -(crc & 1UL);
            crc = (crc >> 1U) ^ (0xEDB88320UL & mask);
        }
    }

    return ~crc;
}
}

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
    Serial.println("Cached device configuration and actual fountain state remain untouched.");
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

StoredFountainState DeviceStorage::loadFountainState()
{
    StoredFountainState state;

    if (!initialized)
    {
        Serial.println("Cannot load fountain state: storage is not initialized.");
        return state;
    }

    const size_t storedLength = preferences.getBytesLength(FountainStateKey);

    if (storedLength == 0)
    {
        Serial.println("No stored fountain state found. Safe OFF defaults remain active.");
        return state;
    }

    if (storedLength != sizeof(FountainStateBlob))
    {
        Serial.println("Stored fountain state has an unexpected size. Safe OFF defaults remain active.");
        return state;
    }

    FountainStateBlob blob;
    const size_t read = preferences.getBytes(FountainStateKey, &blob, sizeof(blob));

    if (read != sizeof(blob))
    {
        Serial.println("Stored fountain state could not be read completely.");
        return state;
    }

    if (blob.version != FountainStateVersion)
    {
        Serial.println("Stored fountain state version is unsupported. Safe OFF defaults remain active.");
        return state;
    }

    if (blob.checksum != calculateChecksum(blob))
    {
        Serial.println("Stored fountain state checksum failed. Safe OFF defaults remain active.");
        return state;
    }

    state.valid = true;
    state.pumpEnabled = (blob.flags & PumpEnabledFlag) != 0;
    state.cobEnabled = (blob.flags & CobEnabledFlag) != 0;
    state.neoPixelsEnabled = (blob.flags & NeoPixelsEnabledFlag) != 0;
    state.neoPixelRed = blob.neoPixelRed;
    state.neoPixelGreen = blob.neoPixelGreen;
    state.neoPixelBlue = blob.neoPixelBlue;

    Serial.println("Stored actual fountain state loaded and verified.");
    return state;
}

bool DeviceStorage::saveFountainState(const StoredFountainState &state)
{
    if (!initialized)
    {
        Serial.println("Cannot save fountain state: storage is not initialized.");
        return false;
    }

    FountainStateBlob blob;

    if (state.pumpEnabled)
    {
        blob.flags |= PumpEnabledFlag;
    }

    if (state.cobEnabled)
    {
        blob.flags |= CobEnabledFlag;
    }

    if (state.neoPixelsEnabled)
    {
        blob.flags |= NeoPixelsEnabledFlag;
    }

    blob.neoPixelRed = state.neoPixelRed;
    blob.neoPixelGreen = state.neoPixelGreen;
    blob.neoPixelBlue = state.neoPixelBlue;
    blob.checksum = calculateChecksum(blob);

    if (preferences.getBytesLength(FountainStateKey) == sizeof(blob))
    {
        FountainStateBlob existing;
        const size_t existingRead = preferences.getBytes(FountainStateKey, &existing, sizeof(existing));

        if (existingRead == sizeof(existing) && std::memcmp(&existing, &blob, sizeof(blob)) == 0)
        {
            Serial.println("Actual fountain state unchanged. Flash write skipped.");
            return true;
        }
    }

    const size_t written = preferences.putBytes(FountainStateKey, &blob, sizeof(blob));

    if (written != sizeof(blob))
    {
        Serial.println("Actual fountain state write failed.");
        return false;
    }

    FountainStateBlob verified;
    const size_t verifiedRead = preferences.getBytes(FountainStateKey, &verified, sizeof(verified));

    if (verifiedRead != sizeof(verified) || std::memcmp(&verified, &blob, sizeof(blob)) != 0)
    {
        Serial.println("Actual fountain state verification failed after writing.");
        return false;
    }

    Serial.println("Actual fountain state saved and verified.");
    return true;
}

void DeviceStorage::removeKeyIfPresent(const char *key)
{
    if (preferences.isKey(key))
    {
        preferences.remove(key);
    }
}