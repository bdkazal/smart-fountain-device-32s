#pragma once

#include <Arduino.h>
#include <DNSServer.h>
#include <WebServer.h>

class DeviceStorage;

enum class SetupCredentialState : uint8_t
{
    Idle,
    Connecting,
    Failed,
    Success
};

enum class SetupScanState : uint8_t
{
    Idle,
    Scanning,
    Ready,
    Failed
};

class SetupPortal
{
public:
    SetupPortal();

    bool begin(DeviceStorage &storage);
    void update();

    bool isActive() const;
    const char *credentialStateName() const;

private:
    WebServer server;
    DNSServer dnsServer;
    DeviceStorage *storage = nullptr;

    bool active = false;
    SetupCredentialState credentialState = SetupCredentialState::Idle;
    SetupScanState scanState = SetupScanState::Idle;

    String pendingSsid;
    String pendingPassword;
    String setupMessage;
    String networksJson = "{\"networks\":[]}";

    unsigned long credentialTestStartedAt = 0;
    unsigned long restartAt = 0;

    void registerRoutes();
    void handleRoot();
    void handleNetworks();
    void handleSave();
    void handleSetupStatus();
    void handleCaptivePortalRedirect();

    void startAsyncScan();
    void updateAsyncScan();
    void startCredentialTest(const String &ssid, const String &password);
    void updateCredentialTest();
    void failCredentialTest(const String &message);

    String buildNetworksJson(int networkCount);
    String jsonEscape(const String &value) const;
    String setupStatusJson() const;
};
