#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

#include "ApiClient.h"

class FountainController;
class HardwareOutputs;
class WaterLevelSensor;

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

    bool hasFetchedConfig() const;
    bool hasSentHeartbeat() const;
    bool hasPolledCommands() const;
    bool hasAppliedCommand() const;
    bool hasPreviewedStateReportPayload() const;
    bool hasAttemptedInitialStateReport() const;
    bool hasSyncedInitialStateReport() const;
    const char *initialStateReportStatusName() const;
    int lastAppliedCommandId() const;
    const LaravelConfigSnapshot &configSnapshot() const;

private:
    ApiClient api;
    String uuid;
    String firmware;
    LaravelConfigSnapshot config;
    String lastStateReportPayload;

    bool configured = false;
    bool configFetched = false;
    bool heartbeatSent = false;
    bool commandPollSucceeded = false;
    bool commandPollOnlineLogged = false;
    bool commandApplied = false;
    bool offlineLogged = false;
    bool stateReportPayloadPreviewPrinted = false;
    bool initialStateReportAttempted = false;
    bool initialStateReportSucceeded = false;
    int lastCommandId = 0;

    unsigned long nextConfigFetchAt = 0;
    unsigned long nextCommandPollAt = 0;
    unsigned long nextHeartbeatAt = 0;
    unsigned long lastHeartbeatAt = 0;

    static constexpr unsigned long RetryAfterFailureMs = 30000;
    static constexpr unsigned long HeartbeatIntervalMs = 60000;
    static constexpr unsigned long CommandPollIntervalMs = 2000;
    static constexpr unsigned long CommandPollFailureBackoffMs = 5000;
    static constexpr uint16_t CommandPollTimeoutMs = 800;

    bool fetchConfig();
    bool sendHeartbeat();
    bool pollCommands(FountainController &fountainController);
    bool ackCommand(int commandId, const char *status, const String &message);
    bool applyStateApplyCommand(JsonObjectConst payload, FountainController &fountainController, String &failureMessage);

    bool canBuildStateReportPayload() const;
    void previewStateReportPayload(const HardwareOutputs &hardwareOutputs, const WaterLevelSensor &waterLevelSensor);
    void postInitialStateReport(const HardwareOutputs &hardwareOutputs, const WaterLevelSensor &waterLevelSensor);
    bool postStateReportPayload(const String &payload, const char *reason);
    String buildStateReportPayload(const HardwareOutputs &hardwareOutputs, const WaterLevelSensor &waterLevelSensor, const char *source);
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
