#include "LaravelApiClient.h"

#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <esp_system.h>

#include "FountainController.h"
#include "HardwareOutputs.h"
#include "WaterLevelSensor.h"

void LaravelApiClient::begin(
    const char *baseUrl,
    const char *deviceUuid,
    const char *deviceApiKey,
    const char *firmwareVersion)
{
    uuid = String(deviceUuid);
    uuid.trim();
    firmware = String(firmwareVersion);
    firmware.trim();

    api.begin(baseUrl, deviceApiKey);
    configured = api.isConfigured() && secretsLookReal();

    Serial.println();
    Serial.println("Laravel API client initialized.");

    if (!configured)
    {
        Serial.println("Laravel API disabled: copy include/DeviceSecrets.example.h to include/DeviceSecrets.h and fill real values.");
        return;
    }

    Serial.print("Laravel API base URL: ");
    Serial.println(api.baseUrl());
    Serial.println("Reminder: ESP32 must use your Mac/server LAN IP, not 127.0.0.1 or localhost.");
    Serial.println("Laravel runtime timing: command poll=2s, state sync=10s, config refresh=120s, heartbeat=60s diagnostics.");
}

void LaravelApiClient::update(
    bool wifiConnected,
    FountainController &fountainController,
    const HardwareOutputs &hardwareOutputs,
    const WaterLevelSensor &waterLevelSensor)
{
    if (!configured)
    {
        return;
    }

    if (!wifiConnected)
    {
        if (!offlineLogged)
        {
            Serial.println("Laravel API paused: Wi-Fi is not connected. Local controls and water safety continue.");
            offlineLogged = true;
        }

        setCloudMode(LaravelCloudMode::Offline);
        return;
    }

    offlineLogged = false;

    const unsigned long now = millis();

    if (!configFetched)
    {
        if (static_cast<long>(now - nextConfigFetchAt) < 0)
        {
            return;
        }

        if (fetchConfig(true))
        {
            configFetched = true;
            nextConfigFetchAt = now + ConfigRefreshIntervalMs;
            nextCommandPollAt = now + 50;
            nextStateReportAt = now + 250;
            nextHeartbeatAt = now + 500;
            stateReportPending = true;
            return;
        }

        scheduleConfigRetry();
        return;
    }

    if (config.deviceType != "smart_fountain")
    {
        return;
    }

    if (static_cast<long>(now - nextCommandPollAt) >= 0)
    {
        const bool pollOk = pollCommands(fountainController);
        nextCommandPollAt = now + (pollOk ? CommandPollIntervalMs : RetryAfterFailureMs);
        return;
    }

    if (shouldPostState(now))
    {
        const bool synced = postState(hardwareOutputs, waterLevelSensor, stateReportPending ? "state_change" : "periodic");
        nextStateReportAt = now + (synced ? StateReportIntervalMs : RetryAfterFailureMs);
        return;
    }

    if (static_cast<long>(now - nextConfigFetchAt) >= 0)
    {
        fetchConfig(false);
        nextConfigFetchAt = millis() + ConfigRefreshIntervalMs;
        return;
    }

    if (!heartbeatSent || static_cast<long>(now - nextHeartbeatAt) >= 0)
    {
        if (sendHeartbeat())
        {
            heartbeatSent = true;
            nextHeartbeatAt = now + HeartbeatIntervalMs;
            return;
        }

        nextHeartbeatAt = now + RetryAfterFailureMs;
    }
}

void LaravelApiClient::markStateReportPending()
{
    stateReportPending = true;
    nextStateReportAt = millis() + 100;
}

bool LaravelApiClient::hasFetchedConfig() const
{
    return configFetched;
}

bool LaravelApiClient::hasSentHeartbeat() const
{
    return heartbeatSent;
}

bool LaravelApiClient::hasPolledCommands() const
{
    return commandPollSucceeded;
}

bool LaravelApiClient::hasAppliedCommand() const
{
    return commandApplied;
}

bool LaravelApiClient::hasSyncedState() const
{
    return stateSynced;
}

bool LaravelApiClient::hasPendingStateReport() const
{
    return stateReportPending;
}

int LaravelApiClient::lastAppliedCommandId() const
{
    return lastCommandId;
}

int LaravelApiClient::lastCompletedCommandId() const
{
    return completedCommandId;
}

const char *LaravelApiClient::cloudModeName() const
{
    switch (cloudMode)
    {
    case LaravelCloudMode::Online:
        return "online";
    case LaravelCloudMode::Offline:
        return "offline";
    case LaravelCloudMode::Unknown:
    default:
        return "unknown";
    }
}

const LaravelConfigSnapshot &LaravelApiClient::configSnapshot() const
{
    return config;
}

bool LaravelApiClient::fetchConfig(bool initialFetch)
{
    HTTPClient http;
    const String endpoint = api.url("/api/device/config?device_uuid=" + queryEncode(uuid));

    http.setTimeout(ApiClient::HttpTimeoutMs);

    if (!http.begin(endpoint))
    {
        Serial.println("Laravel config request could not start.");
        registerApiFailure("config_begin");
        return false;
    }

    api.addDeviceHeaders(http);

    const int status = http.GET();
    const String body = http.getString();
    http.end();

    Serial.print(initialFetch ? "Laravel config HTTP status: " : "Laravel config refresh HTTP status: ");
    Serial.println(status);

    if (status != 200)
    {
        if (body.length() > 0)
        {
            Serial.print("Laravel config response: ");
            Serial.println(body.substring(0, 240));
        }
        registerApiFailure("config");
        return false;
    }

    JsonDocument doc;
    const DeserializationError error = deserializeJson(doc, body);

    if (error)
    {
        Serial.print("Laravel config JSON parse failed: ");
        Serial.println(error.c_str());
        registerApiFailure("config_json");
        return false;
    }

    JsonObject root = doc.as<JsonObject>();
    JsonObject configObject = root["config"].as<JsonObject>();

    if (configObject.isNull())
    {
        configObject = root;
    }

    config.valid = true;
    config.serverTimeUtc = root["server_time_utc"] | "";
    config.deviceType = configObject["device_type"] | "";
    config.configRevision = variantToString(configObject["config_revision"]);

    if (initialFetch)
    {
        Serial.print("server_time_utc: ");
        Serial.println(config.serverTimeUtc.length() ? config.serverTimeUtc : "missing");

        Serial.print("config.device_type: ");
        Serial.println(config.deviceType.length() ? config.deviceType : "missing");

        Serial.print("config.config_revision: ");
        Serial.println(config.configRevision.length() ? config.configRevision : "missing");
    }
    else
    {
        Serial.print("config.config_revision: ");
        Serial.println(config.configRevision.length() ? config.configRevision : "missing");
    }

    if (config.deviceType != "smart_fountain")
    {
        Serial.println("Warning: config.device_type is not smart_fountain. Command polling remains disabled.");
    }

    registerApiSuccess("config");
    return true;
}

bool LaravelApiClient::sendHeartbeat()
{
    HTTPClient http;
    const String endpoint = api.url("/api/device/heartbeat");

    http.setTimeout(ApiClient::HttpTimeoutMs);

    if (!http.begin(endpoint))
    {
        Serial.println("Laravel heartbeat request could not start.");
        registerApiFailure("heartbeat_begin");
        return false;
    }

    api.addDeviceHeaders(http);

    JsonDocument payloadDoc;
    payloadDoc["device_uuid"] = uuid;
    payloadDoc["firmware_version"] = firmware;
    payloadDoc["ip_address"] = WiFi.localIP().toString();
    payloadDoc["wifi_rssi"] = WiFi.RSSI();
    payloadDoc["cloud_mode"] = cloudModeName();
    payloadDoc["state_synced"] = stateSynced;
    payloadDoc["command_poll_active"] = commandPollSucceeded;

    String payload;
    serializeJson(payloadDoc, payload);

    const int status = http.POST(payload);
    const String body = http.getString();
    http.end();

    Serial.print("Heartbeat HTTP status: ");
    Serial.println(status);

    if (status != 200)
    {
        if (body.length() > 0)
        {
            Serial.print("Heartbeat response: ");
            Serial.println(body.substring(0, 240));
        }
        registerApiFailure("heartbeat");
        return false;
    }

    JsonDocument doc;
    const DeserializationError error = deserializeJson(doc, body);

    if (!error)
    {
        const String serverTimeUtc = doc["server_time_utc"] | "";
        const String heartbeatStatus = doc["status"] | "";
        const String lastSeenAt = doc["last_seen_at"] | "";

        if (heartbeatStatus.length() > 0)
        {
            Serial.print("heartbeat.status: ");
            Serial.println(heartbeatStatus);
        }

        if (lastSeenAt.length() > 0)
        {
            Serial.print("heartbeat.last_seen_at: ");
            Serial.println(lastSeenAt);
        }

        if (serverTimeUtc.length() > 0)
        {
            Serial.print("heartbeat.server_time_utc: ");
            Serial.println(serverTimeUtc);
        }
    }

    registerApiSuccess("heartbeat");
    return true;
}

bool LaravelApiClient::pollCommands(FountainController &fountainController)
{
    HTTPClient http;
    const String endpoint = api.url("/api/device/commands?device_uuid=" + queryEncode(uuid));

    http.setTimeout(CommandPollTimeoutMs);

    if (!http.begin(endpoint))
    {
        Serial.println("Laravel command poll request could not start.");
        registerApiFailure("command_poll_begin");
        return false;
    }

    api.addDeviceHeaders(http);

    const int status = http.GET();
    const String body = http.getString();
    http.end();

    if (status != 200)
    {
        Serial.print("Command poll HTTP status: ");
        Serial.println(status);

        if (body.length() > 0)
        {
            Serial.print("Command poll response: ");
            Serial.println(body.substring(0, 240));
        }

        commandPollSucceeded = false;
        commandPollOnlineLogged = false;
        registerApiFailure("command_poll");
        return false;
    }

    commandPollSucceeded = true;
    registerApiSuccess("command_poll");

    if (!commandPollOnlineLogged)
    {
        Serial.println("Command poll HTTP status: 200");
        Serial.println("Laravel command polling is active; dashboard presence should stay fresh.");
        commandPollOnlineLogged = true;
    }

    JsonDocument doc;
    const DeserializationError error = deserializeJson(doc, body);

    if (error)
    {
        Serial.print("Command poll JSON parse failed: ");
        Serial.println(error.c_str());
        registerApiFailure("command_poll_json");
        return false;
    }

    JsonObjectConst command = doc["command"].as<JsonObjectConst>();

    if (command.isNull())
    {
        const unsigned long now = millis();
        if (lastNoCommandLogAt == 0 || now - lastNoCommandLogAt >= NoCommandLogIntervalMs)
        {
            Serial.println("No pending Laravel command.");
            lastNoCommandLogAt = now;
        }
        return true;
    }

    const int commandId = command["id"] | 0;
    const String commandType = command["command_type"] | "";

    Serial.print("Pending Laravel command: id=");
    Serial.print(commandId);
    Serial.print(" type=");
    Serial.println(commandType.length() ? commandType : "missing");

    if (commandId <= 0)
    {
        Serial.println("Pending command ignored: missing command id.");
        return true;
    }

    if (commandType != "state_apply")
    {
        Serial.println("Unsupported command type for current firmware. Marking failed.");
        ackCommand(commandId, "failed", "Unsupported command type for Smart Fountain V1 firmware.");
        stateReportPending = true;
        return true;
    }

    if (!ackCommand(commandId, "acknowledged", "Command received by ESP32."))
    {
        Serial.println("Command was not applied because acknowledge failed.");
        return false;
    }

    String failureMessage;
    const bool applied = applyStateApplyCommand(
        command["payload"].as<JsonObjectConst>(),
        fountainController,
        failureMessage);

    stateReportPending = true;

    if (applied)
    {
        commandApplied = true;
        lastCommandId = commandId;
        completedCommandId = commandId;
        Serial.println("Queueing final actual-state report after Laravel command.");
        return ackCommand(commandId, "executed", "state_apply executed by ESP32.");
    }

    if (failureMessage.length() == 0)
    {
        failureMessage = "state_apply could not be fully applied by ESP32.";
    }

    return ackCommand(commandId, "failed", failureMessage);
}

bool LaravelApiClient::ackCommand(int commandId, const char *status, const String &message)
{
    HTTPClient http;
    const String endpoint = api.url("/api/device/commands/" + String(commandId) + "/ack");

    http.setTimeout(ApiClient::HttpTimeoutMs);

    if (!http.begin(endpoint))
    {
        Serial.println("Command ACK request could not start.");
        registerApiFailure("command_ack_begin");
        return false;
    }

    api.addDeviceHeaders(http);

    JsonDocument payloadDoc;
    payloadDoc["device_uuid"] = uuid;
    payloadDoc["status"] = status;
    payloadDoc["message"] = message;

    String payload;
    serializeJson(payloadDoc, payload);

    const int httpStatus = http.POST(payload);
    const String body = http.getString();
    http.end();

    Serial.print("Command ACK HTTP status: id=");
    Serial.print(commandId);
    Serial.print(" status=");
    Serial.print(status);
    Serial.print(" http=");
    Serial.println(httpStatus);

    if (httpStatus != 200)
    {
        if (body.length() > 0)
        {
            Serial.print("Command ACK response: ");
            Serial.println(body.substring(0, 240));
        }
        registerApiFailure("command_ack");
        return false;
    }

    registerApiSuccess("command_ack");
    return true;
}

bool LaravelApiClient::postState(
    const HardwareOutputs &hardwareOutputs,
    const WaterLevelSensor &waterLevelSensor,
    const char *reason,
    int completedCommandIdForReport)
{
    HTTPClient http;
    const String endpoint = api.url("/api/device/state");

    http.setTimeout(ApiClient::HttpTimeoutMs);

    if (!http.begin(endpoint))
    {
        Serial.println("Laravel state sync request could not start.");
        registerApiFailure("state_begin");
        return false;
    }

    api.addDeviceHeaders(http);

    const bool waterLow = waterLevelSensor.isWaterLow();
    const bool pumpLockout = waterLow;
    const char *operationState = waterLow ? "water_low_lockout" : "online";
    const char *source = reason == nullptr ? "device_state" : reason;

    JsonDocument payloadDoc;
    payloadDoc["device_uuid"] = uuid;
    payloadDoc["device_type"] = "smart_fountain";
    payloadDoc["firmware_version"] = firmware;
    payloadDoc["operation_state"] = operationState;

    if (config.configRevision.length() == 64)
    {
        payloadDoc["config_revision"] = config.configRevision;
    }

    const int commandIdForReport = completedCommandIdForReport > 0 ? completedCommandIdForReport : completedCommandId;
    if (commandIdForReport > 0)
    {
        payloadDoc["last_completed_command_id"] = commandIdForReport;
    }

    JsonObject outputs = payloadDoc["outputs"].to<JsonObject>();

    JsonObject pump = outputs["pump"].to<JsonObject>();
    pump["enabled"] = hardwareOutputs.isPumpEnabled();
    pump["source"] = source;

    JsonObject cobLight = outputs["cob_light"].to<JsonObject>();
    cobLight["enabled"] = hardwareOutputs.isCobEnabled();
    cobLight["source"] = source;

    JsonObject rgbLight = outputs["rgb_light"].to<JsonObject>();
    rgbLight["enabled"] = hardwareOutputs.areNeoPixelsEnabled();
    rgbLight["color"] = colorToHex(
        hardwareOutputs.neoPixelRed(),
        hardwareOutputs.neoPixelGreen(),
        hardwareOutputs.neoPixelBlue());
    rgbLight["brightness_percent"] = 100;
    rgbLight["effect"] = "solid";
    rgbLight["source"] = source;

    JsonObject safety = payloadDoc["safety"].to<JsonObject>();
    safety["water_low"] = waterLow;
    safety["pump_lockout"] = pumpLockout;

    JsonObject clock = payloadDoc["clock"].to<JsonObject>();
    clock["valid"] = false;
    clock["source"] = "not_configured";
    clock["rtc_available"] = false;

    String payload;
    serializeJson(payloadDoc, payload);

    const int status = http.POST(payload);
    const String body = http.getString();
    http.end();

    Serial.print("Laravel state sync HTTP status: ");
    Serial.print(status);
    Serial.print(" reason=");
    Serial.println(source);

    if (status != 200)
    {
        if (body.length() > 0)
        {
            Serial.print("Laravel state sync response: ");
            Serial.println(body.substring(0, 240));
        }
        registerApiFailure("state_sync");
        return false;
    }

    JsonDocument responseDoc;
    const DeserializationError error = deserializeJson(responseDoc, body);

    if (!error)
    {
        const int updatedOutputs = responseDoc["platform_outputs_updated"] | -1;
        const int acceptedCompletedCommandId = responseDoc["accepted_completed_command_id"] | 0;

        if (updatedOutputs >= 0)
        {
            Serial.print("platform_outputs_updated: ");
            Serial.println(updatedOutputs);
        }

        if (acceptedCompletedCommandId > 0)
        {
            Serial.print("accepted_completed_command_id: ");
            Serial.println(acceptedCompletedCommandId);
        }
    }

    if (commandIdForReport > 0)
    {
        completedCommandId = 0;
    }

    stateSynced = true;
    stateReportPending = false;
    registerApiSuccess("state_sync");
    return true;
}

bool LaravelApiClient::applyStateApplyCommand(JsonObjectConst payload, FountainController &fountainController, String &failureMessage)
{
    if (payload.isNull())
    {
        failureMessage = "state_apply payload missing.";
        Serial.println(failureMessage);
        return false;
    }

    JsonObjectConst outputs = payload["outputs"].as<JsonObjectConst>();

    if (outputs.isNull())
    {
        failureMessage = "state_apply outputs missing.";
        Serial.println(failureMessage);
        return false;
    }

    bool touchedOutput = false;
    bool fullyApplied = true;

    JsonObjectConst pump = outputs["pump"].as<JsonObjectConst>();
    if (!pump.isNull())
    {
        touchedOutput = true;
        const bool requested = pump["enabled"] | false;
        const bool accepted = fountainController.requestPumpState(requested, ControlSource::Laravel);

        if (!accepted && requested)
        {
            fullyApplied = false;
            appendFailure(failureMessage, "Pump ON rejected by local water safety.");
        }
    }

    JsonObjectConst cobLight = outputs["cob_light"].as<JsonObjectConst>();
    if (!cobLight.isNull())
    {
        touchedOutput = true;
        const bool requested = cobLight["enabled"] | false;
        fountainController.requestCobState(requested, ControlSource::Laravel);
    }

    JsonObjectConst rgbLight = outputs["rgb_light"].as<JsonObjectConst>();
    if (!rgbLight.isNull())
    {
        touchedOutput = true;
        const bool requested = rgbLight["enabled"] | false;
        const String color = rgbLight["color"] | "#000000";
        const int brightnessPercent = clampPercent(rgbLight["brightness_percent"] | 100);

        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;

        if (!parseHexColor(color, red, green, blue))
        {
            fullyApplied = false;
            appendFailure(failureMessage, "RGB color was invalid.");
        }
        else
        {
            fountainController.requestNeoPixelState(
                requested,
                applyBrightness(red, brightnessPercent),
                applyBrightness(green, brightnessPercent),
                applyBrightness(blue, brightnessPercent),
                ControlSource::Laravel);
        }
    }

    if (!touchedOutput)
    {
        failureMessage = "state_apply did not contain supported outputs.";
        Serial.println(failureMessage);
        return false;
    }

    if (fullyApplied)
    {
        Serial.println("state_apply command applied through FountainController.");
    }
    else
    {
        Serial.print("state_apply command was safety-adjusted/failed: ");
        Serial.println(failureMessage);
    }

    return fullyApplied;
}

void LaravelApiClient::registerApiSuccess(const char *context)
{
    consecutiveApiFailures = 0;
    setCloudMode(LaravelCloudMode::Online);

    (void)context;
}

void LaravelApiClient::registerApiFailure(const char *context)
{
    if (consecutiveApiFailures < 255)
    {
        consecutiveApiFailures++;
    }

    Serial.print("Laravel API failure count: ");
    Serial.print(consecutiveApiFailures);
    Serial.print(" context=");
    Serial.println(context == nullptr ? "unknown" : context);

    if (consecutiveApiFailures >= MaxConsecutiveApiFailures)
    {
        setCloudMode(LaravelCloudMode::Offline);
        Serial.println("Laravel API marked offline after repeated failures. Local controls and water safety continue.");
    }
}

void LaravelApiClient::setCloudMode(LaravelCloudMode mode)
{
    if (cloudMode == mode && cloudModeLogged)
    {
        return;
    }

    cloudMode = mode;
    cloudModeLogged = true;

    Serial.print("Laravel cloud mode: ");
    Serial.println(cloudModeName());
}

bool LaravelApiClient::shouldPostState(unsigned long now) const
{
    return stateReportPending || !stateSynced || static_cast<long>(now - nextStateReportAt) >= 0;
}

String LaravelApiClient::buildReportId()
{
    const uint32_t a = esp_random();
    const uint32_t b = esp_random();
    const uint32_t c = esp_random();
    const uint32_t d = esp_random();

    char buffer[37];
    snprintf(
        buffer,
        sizeof(buffer),
        "%08lx-%04lx-4%03lx-%04lx-%04lx%08lx",
        static_cast<unsigned long>(a),
        static_cast<unsigned long>((b >> 16) & 0xFFFF),
        static_cast<unsigned long>(b & 0x0FFF),
        static_cast<unsigned long>(0x8000 | ((c >> 16) & 0x3FFF)),
        static_cast<unsigned long>(c & 0xFFFF),
        static_cast<unsigned long>(d));

    return String(buffer);
}

String LaravelApiClient::colorToHex(uint8_t red, uint8_t green, uint8_t blue) const
{
    char buffer[8];
    snprintf(buffer, sizeof(buffer), "#%02X%02X%02X", red, green, blue);
    return String(buffer);
}

bool LaravelApiClient::secretsLookReal() const
{
    return !isPlaceholder(api.baseUrl()) && !isPlaceholder(uuid);
}

bool LaravelApiClient::isPlaceholder(const String &value) const
{
    return value.length() == 0 ||
           value.indexOf("YOUR_") >= 0 ||
           value.indexOf("xxx") >= 0 ||
           value.indexOf("127.0.0.1") >= 0 ||
           value.indexOf("localhost") >= 0;
}

bool LaravelApiClient::parseHexColor(const String &color, uint8_t &red, uint8_t &green, uint8_t &blue) const
{
    if (color.length() != 7 || color.charAt(0) != '#')
    {
        return false;
    }

    const int r1 = hexNibble(color.charAt(1));
    const int r2 = hexNibble(color.charAt(2));
    const int g1 = hexNibble(color.charAt(3));
    const int g2 = hexNibble(color.charAt(4));
    const int b1 = hexNibble(color.charAt(5));
    const int b2 = hexNibble(color.charAt(6));

    if (r1 < 0 || r2 < 0 || g1 < 0 || g2 < 0 || b1 < 0 || b2 < 0)
    {
        return false;
    }

    red = static_cast<uint8_t>((r1 << 4) | r2);
    green = static_cast<uint8_t>((g1 << 4) | g2);
    blue = static_cast<uint8_t>((b1 << 4) | b2);
    return true;
}

int LaravelApiClient::hexNibble(char value) const
{
    if (value >= '0' && value <= '9')
    {
        return value - '0';
    }

    if (value >= 'a' && value <= 'f')
    {
        return value - 'a' + 10;
    }

    if (value >= 'A' && value <= 'F')
    {
        return value - 'A' + 10;
    }

    return -1;
}

uint8_t LaravelApiClient::applyBrightness(uint8_t value, int brightnessPercent) const
{
    return static_cast<uint8_t>((static_cast<uint16_t>(value) * clampPercent(brightnessPercent)) / 100);
}

int LaravelApiClient::clampPercent(int value) const
{
    if (value < 0)
    {
        return 0;
    }

    if (value > 100)
    {
        return 100;
    }

    return value;
}

void LaravelApiClient::appendFailure(String &message, const String &detail) const
{
    if (message.length() > 0)
    {
        message += " ";
    }

    message += detail;
}

void LaravelApiClient::scheduleConfigRetry()
{
    nextConfigFetchAt = millis() + RetryAfterFailureMs;
    Serial.println("Laravel config retry scheduled. Local controls and water safety continue.");
}

String LaravelApiClient::queryEncode(const String &value) const
{
    String encoded;

    for (size_t i = 0; i < value.length(); ++i)
    {
        const char c = value.charAt(i);
        const bool safe =
            (c >= 'A' && c <= 'Z') ||
            (c >= 'a' && c <= 'z') ||
            (c >= '0' && c <= '9') ||
            c == '-' || c == '_' || c == '.' || c == '~';

        if (safe)
        {
            encoded += c;
            continue;
        }

        char buffer[4];
        snprintf(buffer, sizeof(buffer), "%%%02X", static_cast<unsigned char>(c));
        encoded += buffer;
    }

    return encoded;
}

String LaravelApiClient::jsonEscape(const String &value) const
{
    String escaped;
    escaped.reserve(value.length() + 8);

    for (size_t i = 0; i < value.length(); ++i)
    {
        const char c = value.charAt(i);

        switch (c)
        {
        case '\\':
            escaped += "\\\\";
            break;
        case '"':
            escaped += "\\\"";
            break;
        case '\n':
            escaped += "\\n";
            break;
        case '\r':
            escaped += "\\r";
            break;
        case '\t':
            escaped += "\\t";
            break;
        default:
            escaped += c;
            break;
        }
    }

    return escaped;
}

String LaravelApiClient::variantToString(JsonVariantConst value) const
{
    if (value.isNull())
    {
        return "";
    }

    if (value.is<const char *>())
    {
        return String(value.as<const char *>());
    }

    String result;
    serializeJson(value, result);
    result.replace("\"", "");
    return result;
}
