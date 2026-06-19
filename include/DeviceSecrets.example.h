#pragma once

// Copy this file to include/DeviceSecrets.h and fill in real local values.
// Never commit include/DeviceSecrets.h.

// Optional development Wi-Fi fallback.
// The production path should continue using the setup portal/stored credentials.
#define WIFI_SSID "YOUR_WIFI_NAME"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"

// From ESP32, do not use 127.0.0.1 or localhost.
// Use your Mac/Laravel server LAN IP, for example: http://192.168.0.105:8000
#define API_BASE_URL "http://192.168.0.xxx:8000"

#define DEVICE_UUID "YOUR_SMART_FOUNTAIN_DEVICE_UUID"
#define DEVICE_API_KEY "YOUR_SMART_FOUNTAIN_DEVICE_API_KEY"

#define FIRMWARE_VERSION "smart-fountain-32s-state-0.4-command-polling"
