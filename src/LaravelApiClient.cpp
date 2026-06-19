#include "LaravelApiClient.h"

#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>

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
    Serial.println("Laravel API handshake client initialized.");

    if (!configured)
    {
        Serial.println("Laravel API handshake disabled: copy include/DeviceSecrets.example.h to include/DeviceSecrets.h and fill real values.");
        return;
    }

    Serial.print("Laravel API base URL: ");
    Serial.println(api.baseUrl());
    Serial.println("Reminder: ESP32 must use your Mac/server LAN IP, not 127.0.0.1 or localhost.");
}

void LaravelApiClient::update(bool wifiConnected)
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
        return;
    }

    offlineLogged = false;

    const unsigned long now = millis();

    if (static_cast<long>(now - nextActionAt) < 0)
    {
        return;
    }

    if (!configFetched || step == RuntimeStep::FetchConfig)
    {
        if (fetchConfig())
        {
            configFetched = true;
            step = RuntimeStep::SendHeartbeat;
            scheduleSoon();
            return;
        }

        step = RuntimeStep::FetchConfig;
        scheduleRetry();
        return;
    }

    if (!heartbeatSent || step == RuntimeStep::SendHeartbeat || now - lastHeartbeatAt >= HeartbeatIntervalMs)
    {
        if (sendHeartbeat())
        {
            heartbeatSent = true;
            lastHeartbeatAt = now;
            step = RuntimeStep::Waiting;
            nextActionAt = now + HeartbeatIntervalMs;
            return;
        }

        step = RuntimeStep::SendHeartbeat;
        scheduleRetry();
    }
}

bool LaravelApiClient::hasFetchedConfig() const
{
    return configFetched;
}

bool LaravelApiClient::hasSentHeartbeat() const
{
    return heartbeatSent;
}

const LaravelConfigSnapshot &LaravelApiClient::configSnapshot() const
{
    return config;
}

bool LaravelApiClient::fetchConfig()
{
    HTTPClient http;
    const String endpoint = api.url("/api/device/config?device_uuid=" + queryEncode(uuid));

    http.setTimeout(ApiClient::HttpTimeoutMs);

    if (!http.begin(endpoint))
    {
        Serial.println("Laravel config request could not start.");
        return false;
    }

    api.addDeviceHeaders(http);

    const int status = http.GET();
    const String body = http.getString();
    http.end();

    Serial.print("Laravel config HTTP status: ");
    Serial.println(status);

    if (status != 200)
    {
        if (body.length() > 0)
        {
            Serial.print("Laravel config response: ");
            Serial.println(body.substring(0, 240));
        }
        return false;
    }

    JsonDocument doc;
    const DeserializationError error = deserializeJson(doc, body);

    if (error)
    {
        Serial.print("Laravel config JSON parse failed: ");
        Serial.println(error.c_str());
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

    Serial.print("server_time_utc: ");
    Serial.println(config.serverTimeUtc.length() ? config.serverTimeUtc : "missing");

    Serial.print("config.device_type: ");
    Serial.println(config.deviceType.length() ? config.deviceType : "missing");

    Serial.print("config.config_revision: ");
    Serial.println(config.configRevision.length() ? config.configRevision : "missing");

    if (config.deviceType != "smart_fountain")
    {
        Serial.println("Warning: config.device_type is not smart_fountain. Commands remain disabled in Stage 1.");
    }

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
        return false;
    }

    api.addDeviceHeaders(http);

    String payload = "{";
    payload += "\"device_uuid\":\"";
    payload += jsonEscape(uuid);
    payload += "\",\"firmware_version\":\"";
    payload += jsonEscape(firmware);
    payload += "\",\"ip_address\":\"";
    payload += WiFi.localIP().toString();
    payload += "\",\"wifi_rssi\":";
    payload += String(WiFi.RSSI());
    payload += "}";

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
        return false;
    }

    JsonDocument doc;
    const DeserializationError error = deserializeJson(doc, body);

    if (!error)
    {
        const String serverTimeUtc = doc["server_time_utc"] | "";
        if (serverTimeUtc.length() > 0)
        {
            Serial.print("heartbeat.server_time_utc: ");
            Serial.println(serverTimeUtc);
        }
    }

    return true;
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

void LaravelApiClient::scheduleRetry()
{
    nextActionAt = millis() + RetryAfterFailureMs;
    Serial.println("Laravel API retry scheduled. Local controls and water safety continue.");
}

void LaravelApiClient::scheduleSoon()
{
    nextActionAt = millis() + 50;
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
