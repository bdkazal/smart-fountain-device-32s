#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

#include "ApiClient.h"

class FountainController;
class HardwareOutputs;
class WaterLevelSensor;

enum class LaravelCloudMode : uint8_t
{
    Unknown,
    Online,
    Offline
};

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

    void update(
        bool wifiConnected,
        FountainController &fountainController,
        const HardwareOutputs &hardwareOutputs,
        const WaterLevelSensor &waterLevelSensor);

    void markStateReportPending();

    bool hasFetchedConfig() const;
    bool hasSentHeartbeat() const;
    bool hasPolledCommands() const;
    bool hasAppliedCommand() const;
    bool hasSyncedState() const;
    bool hasPendingStateReport() const;
    int lastAppliedCommandId() const;
    int lastCompletedCommandId() const;
    const char *cloudModeName() const;
    const LaravelConfigSnapshot &configSnapshot() const;

private:
    ApiClient api;
    String uuid;
    String firmware;
    LaravelConfigSnapshot config;

    bool configured = false;
    bool configFetched = false;
    bool heartbeatSent = false;
    bool commandPollSucceeded = false;
    bool commandPollOnlineLogged = false;
    bool commandApplied = false;
    bool stateSynced = false;
    bool stateReportPending = false;
    bool offlineLogged = false;
    bool cloudModeLogged = false;

    int lastCommandId = 0;
    int completedCommandId = 0;
    uint32_t stateReportSequence = 0;
    uint8_t consecutiveApiFailures = 0;
    LaravelCloudMode cloudMode = LaravelCloudMode::Unknown;

    unsigned long nextConfigFetchAt = 0;
    unsigned long nextCommandPollAt = 0;
    unsigned long nextStateReportAt = 0;
    unsigned long nextHeartbeatAt = 0;
    unsigned long lastNoCommandLogAt = 0;

    static constexpr unsigned long RetryAfterFailureMs = 30000;
    static constexpr unsigned long HeartbeatIntervalMs = 60000;
    static constexpr unsigned long ConfigRefreshIntervalMs = 120000;
    static constexpr unsigned long CommandPollIntervalMs = 2000;
    static constexpr unsigned long StateReportIntervalMs = 10000;
    static constexpr unsigned long NoCommandLogIntervalMs = 30000;
    static constexpr uint8_t MaxConsecutiveApiFailures = 3;
    static constexpr uint16_t CommandPollTimeoutMs = ApiClient::HttpTimeoutMs;

    bool fetchConfig(bool initialFetch);
    bool sendHeartbeat();
    bool pollCommands(FountainController &fountainController);
    bool ackCommand(int commandId, const char *status, const String &message);
    bool postState(
        const HardwareOutputs &hardwareOutputs,
        const WaterLevelSensor &waterLevelSensor,
        const char *reason,
        int completedCommandIdForReport = 0);
    bool applyStateApplyCommand(JsonObjectConst payload, FountainController &fountainController, String &failureMessage);

    void registerApiSuccess(const char *context);
    void registerApiFailure(const char *context);
    void setCloudMode(LaravelCloudMode mode);
    bool shouldPostState(unsigned long now) const;
    String buildReportId();
    String colorToHex(uint8_t red, uint8_t green, uint8_t blue) const;

    bool secretsLookReal() const;
    bool isPlaceholder(const String &value) const;
    bool parseHexColor(const String &color, uint8_t &red, uint8_t &green, uint8_t &blue) const;
    int hexNibble(char value) const;
    uint8_t applyBrightness(uint8_t value, int brightnessPercent) const;
    int clampPercent(int value) const;
    void appendFailure(String &message, const String &detail) const;
    void scheduleConfigRetry();
    String queryEncode(const String &value) const;
    String jsonEscape(const String &value) const;
    String variantToString(JsonVariantConst value) const;
};
