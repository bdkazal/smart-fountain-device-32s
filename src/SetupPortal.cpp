#include "SetupPortal.h"

#include <WiFi.h>

#include "DeviceStorage.h"
#include "SetupPortalPage.h"
#include "WifiConfig.h"

SetupPortal::SetupPortal()
    : server(80)
{
}

bool SetupPortal::begin(DeviceStorage &deviceStorage)
{
    storage = &deviceStorage;

    WiFi.persistent(false);
    WiFi.setAutoReconnect(false);
    WiFi.disconnect(false, false);
    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_AP_STA);

    const bool apStarted = WiFi.softAP(
        WifiConfig::SetupApSsid,
        WifiConfig::SetupApPassword,
        WifiConfig::SetupApChannel,
        WifiConfig::SetupApHidden,
        WifiConfig::SetupApMaxConnections);

    if (!apStarted)
    {
        Serial.println("Setup portal failed: hotspot could not start.");
        active = false;
        return false;
    }

    const IPAddress portalIp = WiFi.softAPIP();
    dnsServer.start(WifiConfig::DnsPort, "*", portalIp);

    registerRoutes();
    server.begin();

    credentialState = SetupCredentialState::Idle;
    scanState = SetupScanState::Idle;
    setupMessage = "Waiting for Wi-Fi details.";
    active = true;

    Serial.println();
    Serial.println("Wi-Fi setup portal started.");
    Serial.print("Setup hotspot SSID: ");
    Serial.println(WifiConfig::SetupApSsid);
    Serial.print("Setup portal IP: ");
    Serial.println(portalIp);
    Serial.println("Captive DNS redirect enabled.");

    return true;
}

void SetupPortal::update()
{
    if (!active)
    {
        return;
    }

    dnsServer.processNextRequest();
    server.handleClient();
    updateAsyncScan();
    updateCredentialTest();

    if (restartAt > 0 && millis() >= restartAt)
    {
        Serial.println("Restarting with saved Wi-Fi credentials...");
        delay(100);
        ESP.restart();
    }
}

bool SetupPortal::isActive() const
{
    return active;
}

const char *SetupPortal::credentialStateName() const
{
    switch (credentialState)
    {
    case SetupCredentialState::Idle:
        return "idle";
    case SetupCredentialState::Connecting:
        return "connecting";
    case SetupCredentialState::Failed:
        return "failed";
    case SetupCredentialState::Success:
        return "success";
    }

    return "unknown";
}

void SetupPortal::registerRoutes()
{
    server.on("/", HTTP_GET, [this]() { handleRoot(); });
    server.on("/networks", HTTP_GET, [this]() { handleNetworks(); });
    server.on("/save", HTTP_POST, [this]() { handleSave(); });
    server.on("/setup-status", HTTP_GET, [this]() { handleSetupStatus(); });

    server.on("/generate_204", HTTP_GET, [this]() { handleCaptivePortalRedirect(); });
    server.on("/hotspot-detect.html", HTTP_GET, [this]() { handleCaptivePortalRedirect(); });
    server.on("/ncsi.txt", HTTP_GET, [this]() { handleCaptivePortalRedirect(); });
    server.on("/connecttest.txt", HTTP_GET, [this]() { handleCaptivePortalRedirect(); });
    server.on("/favicon.ico", HTTP_GET, [this]() { server.send(204, "text/plain", ""); });
    server.onNotFound([this]() { handleCaptivePortalRedirect(); });
}

void SetupPortal::handleRoot()
{
    server.send_P(200, "text/html", SetupPortalPage);
}

void SetupPortal::handleNetworks()
{
    if (server.hasArg("refresh"))
    {
        startAsyncScan();
        server.send(202, "application/json", "{\"status\":\"scanning\"}");
        return;
    }

    if (scanState == SetupScanState::Idle || scanState == SetupScanState::Failed)
    {
        startAsyncScan();
    }

    if (scanState == SetupScanState::Scanning)
    {
        server.send(202, "application/json", "{\"status\":\"scanning\"}");
        return;
    }

    server.send(200, "application/json", networksJson);
}

void SetupPortal::handleSave()
{
    if (credentialState == SetupCredentialState::Connecting)
    {
        server.send(409, "application/json", "{\"ok\":false,\"message\":\"A Wi-Fi test is already running.\"}");
        return;
    }

    String ssid = server.arg("ssid");
    String password = server.arg("password");
    ssid.trim();

    if (ssid.length() == 0 || ssid.length() > 32)
    {
        server.send(400, "application/json", "{\"ok\":false,\"message\":\"Enter a valid Wi-Fi SSID.\"}");
        return;
    }

    if (password.length() > 63)
    {
        server.send(400, "application/json", "{\"ok\":false,\"message\":\"Wi-Fi password is too long.\"}");
        return;
    }

    startCredentialTest(ssid, password);
    server.send(202, "application/json", "{\"ok\":true,\"status\":\"connecting\"}");
}

void SetupPortal::handleSetupStatus()
{
    server.send(200, "application/json", setupStatusJson());
}

void SetupPortal::handleCaptivePortalRedirect()
{
    String location = "http://";
    location += WiFi.softAPIP().toString();
    location += "/";

    server.sendHeader("Location", location, true);
    server.send(302, "text/plain", "");
}

void SetupPortal::startAsyncScan()
{
    WiFi.scanDelete();
    scanState = SetupScanState::Scanning;
    networksJson = "{\"networks\":[]}";

    const int result = WiFi.scanNetworks(true, true);

    if (result == WIFI_SCAN_FAILED)
    {
        scanState = SetupScanState::Failed;
        Serial.println("Wi-Fi scan could not start.");
        return;
    }

    Serial.println("Asynchronous Wi-Fi scan started.");
}

void SetupPortal::updateAsyncScan()
{
    if (scanState != SetupScanState::Scanning)
    {
        return;
    }

    const int result = WiFi.scanComplete();

    if (result == WIFI_SCAN_RUNNING)
    {
        return;
    }

    if (result == WIFI_SCAN_FAILED)
    {
        scanState = SetupScanState::Failed;
        networksJson = "{\"networks\":[]}";
        Serial.println("Wi-Fi scan failed.");
        return;
    }

    networksJson = buildNetworksJson(result);
    scanState = SetupScanState::Ready;
    WiFi.scanDelete();

    Serial.print("Wi-Fi scan completed. Networks found: ");
    Serial.println(result);
}

void SetupPortal::startCredentialTest(const String &ssid, const String &password)
{
    pendingSsid = ssid;
    pendingPassword = password;
    credentialState = SetupCredentialState::Connecting;
    setupMessage = "Testing Wi-Fi credentials.";
    credentialTestStartedAt = millis();
    restartAt = 0;

    WiFi.mode(WIFI_AP_STA);
    WiFi.disconnect(false, false);
    WiFi.begin(pendingSsid.c_str(), pendingPassword.c_str());

    Serial.println();
    Serial.print("Testing submitted Wi-Fi SSID: ");
    Serial.println(pendingSsid);
}

void SetupPortal::updateCredentialTest()
{
    if (credentialState != SetupCredentialState::Connecting)
    {
        return;
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        if (storage == nullptr || !storage->saveWifiCredentials(pendingSsid, pendingPassword))
        {
            failCredentialTest("Wi-Fi connected, but credentials could not be saved.");
            return;
        }

        credentialState = SetupCredentialState::Success;
        setupMessage = "Wi-Fi saved successfully. Device is restarting.";
        restartAt = millis() + WifiConfig::PortalRestartDelayMs;

        pendingPassword = "";

        Serial.println("Submitted Wi-Fi credentials worked and were saved.");
        Serial.print("Temporary station IP: ");
        Serial.println(WiFi.localIP());
        return;
    }

    if (millis() - credentialTestStartedAt >= WifiConfig::CredentialTestTimeoutMs)
    {
        failCredentialTest("Could not connect. Check the SSID/password and try again.");
    }
}

void SetupPortal::failCredentialTest(const String &message)
{
    credentialState = SetupCredentialState::Failed;
    setupMessage = message;
    pendingPassword = "";

    WiFi.disconnect(false, false);
    WiFi.mode(WIFI_AP_STA);

    Serial.println(message);
}

String SetupPortal::buildNetworksJson(int networkCount)
{
    String json = "{\"status\":\"ready\",\"networks\":[";
    bool first = true;

    for (int i = 0; i < networkCount; i++)
    {
        const String ssid = WiFi.SSID(i);

        if (ssid.length() == 0)
        {
            continue;
        }

        bool duplicate = false;

        for (int previous = 0; previous < i; previous++)
        {
            if (WiFi.SSID(previous) == ssid)
            {
                duplicate = true;
                break;
            }
        }

        if (duplicate)
        {
            continue;
        }

        if (!first)
        {
            json += ',';
        }

        json += "{\"ssid\":\"";
        json += jsonEscape(ssid);
        json += "\",\"rssi\":";
        json += String(WiFi.RSSI(i));
        json += '}';
        first = false;
    }

    json += "]}";
    return json;
}

String SetupPortal::jsonEscape(const String &value) const
{
    String escaped = value;
    escaped.replace("\\", "\\\\");
    escaped.replace("\"", "\\\"");
    escaped.replace("\n", "\\n");
    escaped.replace("\r", "\\r");
    return escaped;
}

String SetupPortal::setupStatusJson() const
{
    String json = "{\"status\":\"";
    json += credentialStateName();
    json += "\",\"message\":\"";
    json += jsonEscape(setupMessage);
    json += "\"}";
    return json;
}
