#pragma once

#include <Arduino.h>

#include "DeviceStorage.h"

enum class WifiConnectionState : uint8_t
{
    Idle,
    Connecting,
    Connected,
    WaitingToRetry,
    NoCredentials
};

class WifiManager
{
public:
    void begin(const StoredWifiCredentials &credentials);
    void update();

    bool isConnected() const;
    WifiConnectionState state() const;
    const char *stateName() const;

private:
    StoredWifiCredentials credentials;
    WifiConnectionState currentState = WifiConnectionState::Idle;
    unsigned long attemptStartedAt = 0;
    unsigned long lastAttemptFinishedAt = 0;

    void startConnection();
    void markConnected();
    void markDisconnected();
};
