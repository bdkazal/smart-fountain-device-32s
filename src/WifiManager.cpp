#include "WifiManager.h"

#include <WiFi.h>

#include "WifiConfig.h"

void WifiManager::begin(const StoredWifiCredentials &storedCredentials)
{
    credentials = storedCredentials;

    WiFi.persistent(false);
    WiFi.setAutoReconnect(false);
    WiFi.mode(WIFI_STA);

    if (!credentials.valid)
    {
        currentState = WifiConnectionState::NoCredentials;
        Serial.println("Wi-Fi manager started without stored credentials.");
        return;
    }

    startConnection();
}

void WifiManager::update()
{
    const wl_status_t status = WiFi.status();
    const unsigned long now = millis();

    if (status == WL_CONNECTED)
    {
        if (currentState != WifiConnectionState::Connected)
        {
            markConnected();
        }

        return;
    }

    if (currentState == WifiConnectionState::Connected)
    {
        markDisconnected();
        return;
    }

    if (currentState == WifiConnectionState::Connecting)
    {
        if (now - attemptStartedAt >= WifiConfig::StationConnectTimeoutMs)
        {
            WiFi.disconnect(false, false);
            currentState = WifiConnectionState::WaitingToRetry;
            lastAttemptFinishedAt = now;
            Serial.println("Wi-Fi connection attempt timed out. Local controls remain active.");
        }

        return;
    }

    if (currentState == WifiConnectionState::WaitingToRetry &&
        now - lastAttemptFinishedAt >= WifiConfig::StationReconnectIntervalMs)
    {
        startConnection();
    }
}

bool WifiManager::isConnected() const
{
    return WiFi.status() == WL_CONNECTED;
}

WifiConnectionState WifiManager::state() const
{
    return currentState;
}

const char *WifiManager::stateName() const
{
    switch (currentState)
    {
    case WifiConnectionState::Idle:
        return "idle";
    case WifiConnectionState::Connecting:
        return "connecting";
    case WifiConnectionState::Connected:
        return "connected";
    case WifiConnectionState::WaitingToRetry:
        return "waiting_to_retry";
    case WifiConnectionState::NoCredentials:
        return "no_credentials";
    }

    return "unknown";
}

void WifiManager::startConnection()
{
    if (!credentials.valid)
    {
        currentState = WifiConnectionState::NoCredentials;
        return;
    }

    WiFi.mode(WIFI_STA);
    WiFi.disconnect(false, false);
    WiFi.begin(credentials.ssid.c_str(), credentials.password.c_str());

    attemptStartedAt = millis();
    currentState = WifiConnectionState::Connecting;

    Serial.println();
    Serial.print("Starting non-blocking Wi-Fi connection to SSID: ");
    Serial.println(credentials.ssid);
}

void WifiManager::markConnected()
{
    currentState = WifiConnectionState::Connected;

    Serial.println();
    Serial.println("Wi-Fi connected.");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.print("RSSI dBm: ");
    Serial.println(WiFi.RSSI());
}

void WifiManager::markDisconnected()
{
    currentState = WifiConnectionState::WaitingToRetry;
    lastAttemptFinishedAt = millis();

    Serial.println("Wi-Fi disconnected. Local controls remain active; reconnect will be retried.");
}
