# Project Status

## Project

**Repository:** `bdkazal/smart-fountain-device-32s`

ESP32-WROOM-32 firmware for the Biztola Smart Fountain product.

## Current branch

```text
feature/wifi-onboarding
```

## Current milestone

**Wi-Fi onboarding and local-first runtime — initial provisioning flow validated**

The hardware foundation has passed. First-boot setup, credential storage, restart, and stored-Wi-Fi connection have now been validated on the ESP32-WROOM-32 board.

## Completed hardware foundation

- [x] ESP32-WROOM-32 PlatformIO target (`esp32dev`)
- [x] Modular `include/*.h` and `src/*.cpp` structure
- [x] Safe pump, COB, and WS2812B startup OFF
- [x] GPIO18 pump local button
- [x] GPIO19 COB local button
- [x] GPIO25 pump output / indicator
- [x] GPIO26 COB output / indicator
- [x] GPIO27 WS2812B output and GRB color order
- [x] GPIO32 debounced water-level switch
- [x] GPIO33 Wi-Fi reset input
- [x] Local pump safety blocks and forces pump OFF during low water
- [x] Local actions produce state-change events ready for future Laravel sync

## Wi-Fi onboarding implementation

- [x] Dedicated `feature/wifi-onboarding` branch
- [x] `DeviceStorage` using Preferences/NVS namespace `fountain`
- [x] Wi-Fi writes verified after storage
- [x] Wi-Fi reset is designed to clear only Wi-Fi SSID/password
- [x] Persistent provisioning-required flag
- [x] GPIO33 boot-time 3-second hold implementation
- [x] No automatic development-router fallback
- [x] Non-blocking station connection state machine
- [x] Stored-network reconnect without exposing the portal automatically
- [x] `Fountain-Setup` access point
- [x] Captive DNS redirect
- [x] Setup page loads before Wi-Fi scan finishes
- [x] Asynchronous network scan
- [x] Manual/hidden SSID entry
- [x] Password show/hide
- [x] Non-blocking submitted-credential test
- [x] Browser status polling
- [x] Wrong-password retry support
- [x] Save, verify, clear provisioning flag, and restart after success
- [x] Setup page separated into `SetupPortalPage.h`

## Validated on hardware

Firmware:

```text
smart-fountain-32s-wifi-0.1-onboarding
```

Confirmed from the latest serial test:

- [x] Firmware builds and uploads
- [x] First boot without stored Wi-Fi starts setup mode
- [x] Setup portal accepts router credentials
- [x] Submitted SSID `Andromeda` connects successfully
- [x] Wi-Fi credentials are saved and verified
- [x] Device restarts after successful save
- [x] Next boot finds the stored SSID
- [x] Station connection starts without blocking the local runtime
- [x] Device connects at `192.168.0.102`
- [x] Connected RSSI observed around `-35` to `-36 dBm`
- [x] Pump, COB, and NeoPixels remain safely OFF through restart
- [x] Water state remains available during setup and connected runtime

## Remaining Wi-Fi validation

1. Confirm a wrong password leaves the setup portal active and allows retry.
2. Confirm pump/COB buttons and low-water safety remain responsive while credential testing is in progress.
3. Turn off the router and confirm reconnect retries without starting the setup hotspot.
4. Confirm local buttons and pump safety while the router is unavailable.
5. Restore the router and confirm automatic reconnection.
6. Release GPIO33 before 3 seconds during boot and confirm reset cancellation.
7. Hold GPIO33 during boot for 3 seconds and confirm:
   - stored Wi-Fi is cleared,
   - provisioning remains required,
   - `Fountain-Setup` returns,
   - new credentials can be saved.

## Next development stage

After the remaining Wi-Fi tests pass, create a Laravel HTTP integration branch and add:

- device identity and `X-DEVICE-KEY`
- Smart Fountain config fetch
- dashboard command polling
- command acknowledgement and completion
- actual state synchronization after local buttons, Laravel commands, schedules, and safety overrides
- compact cached config for offline behavior

MQTT follows only after HTTP behavior is stable.
