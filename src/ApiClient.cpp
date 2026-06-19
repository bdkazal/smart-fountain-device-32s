#include "ApiClient.h"

void ApiClient::begin(const char *baseUrl, const char *deviceApiKey)
{
    normalizedBaseUrl = String(baseUrl);
    normalizedBaseUrl.trim();
    apiKey = String(deviceApiKey);
    apiKey.trim();

    while (normalizedBaseUrl.endsWith("/"))
    {
        normalizedBaseUrl.remove(normalizedBaseUrl.length() - 1);
    }
}

String ApiClient::url(const String &path) const
{
    if (path.startsWith("/"))
    {
        return normalizedBaseUrl + path;
    }

    return normalizedBaseUrl + "/" + path;
}

void ApiClient::addDeviceHeaders(HTTPClient &http) const
{
    http.addHeader("Accept", "application/json");
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Connection", "close");
    http.addHeader("X-DEVICE-KEY", apiKey);
}

bool ApiClient::isConfigured() const
{
    return normalizedBaseUrl.length() > 0 && apiKey.length() > 0;
}

const String &ApiClient::baseUrl() const
{
    return normalizedBaseUrl;
}
