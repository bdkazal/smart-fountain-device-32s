#pragma once

#include <Arduino.h>
#include <HTTPClient.h>

class ApiClient
{
public:
    static constexpr uint16_t HttpTimeoutMs = 1200;

    void begin(const char *baseUrl, const char *deviceApiKey);

    String url(const String &path) const;
    void addDeviceHeaders(HTTPClient &http) const;

    bool isConfigured() const;
    const String &baseUrl() const;

private:
    String normalizedBaseUrl;
    String apiKey;
};
