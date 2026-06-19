#pragma once

#include <Arduino.h>

#include "ApiClient.h"

struct LaravelConfigSnapshot
{
    bool valid = false;
    String serverTimeUtc;
    String deviceType;
    String configRevision;
};

class LaravelApiClient
{
public:
    void begin(
        const char *baseUrl,
        const char *deviceUuid,
        const char *deviceApiKey,
        const char *firmwareVersion);

    void update(bool wifiConnected);

    bool hasFetchedConfig() const;
    bool hasSentHeartbeat() const;
    const LaravelConfigSnapshot &configSnapshot() const;

private:
    enum class RuntimeStep : uint8_t
    {
        FetchConfig,
        SendHeartbeat,
        Waiting
    };

    ApiClient api;
    String uuid;
    String firmware;
    LaravelConfigSnapshot config;

    bool configured = false;
    bool configFetched = false;
    bool heartbeatSent = false;
    bool offlineLogged = false;
    RuntimeStep step = RuntimeStep::FetchConfig;
    unsigned long nextActionAt = 0;
    unsigned long lastHeartbeatAt = 0;

    static constexpr unsigned long RetryAfterFailureMs = 30000;
    static constexpr unsigned long HeartbeatIntervalMs = 60000;

    bool fetchConfig();
    bool sendHeartbeat();
    bool secretsLookReal() const;
    bool isPlaceholder(const String &value) const;
    void scheduleRetry();
    void scheduleSoon();
    String queryEncode(const String &value) const;
    String jsonEscape(const String &value) const;
    String variantToString(JsonVariantConst value) const;
};
