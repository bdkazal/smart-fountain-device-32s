# Project Status

## Project

**Repository:** `bdkazal/smart-fountain-device-32s`

ESP32-WROOM-32 firmware for the Biztola Smart Fountain product.

## Current branch

```text
feature/wifi-onboarding
```

## Current milestone

**Wi-Fi onboarding and local-first runtime**

The hardware foundation has passed. The current work adds production-minded Wi-Fi provisioning while keeping physical controls and pump safety responsive in every network state.

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
- [x] Wi-Fi reset clears only Wi-Fi SSID/password
- [x] Persistent provisioning-required flag
- [x] GPIO33 boot-time 3-second hold
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
- [x] Wrong-password retry on the same page
- [x] Save, verify, clear provisioning flag, and restart after success
- [x] Setup page separated into `SetupPortalPage.h`

## Current firmware version

```text
smart-fountain-32s-wifi-0.1-onboarding
```

## Current validation required

1. Build the branch locally.
2. Upload with no stored credentials.
3. Confirm the `Fountain-Setup` hotspot appears.
4. Confirm the captive page opens, or use `http://192.168.4.1`.
5. Confirm local pump/COB buttons and low-water safety remain responsive during setup.
6. Confirm network scan results appear.
7. Test a wrong password and retry.
8. Save correct credentials and confirm restart.
9. Confirm stored-Wi-Fi connection on the next boot.
10. Disable the router and confirm periodic reconnect while local control remains active.
11. Hold GPIO33 during boot for 3 seconds and confirm setup mode returns.

## Next development stage

After Wi-Fi onboarding passes, create a Laravel HTTP integration branch and add:

- device identity and `X-DEVICE-KEY`
- Smart Fountain config fetch
- dashboard command polling
- command ACK handling
- actual state synchronization after local buttons, Laravel commands, schedules, and safety overrides
- compact cached config for offline behavior

MQTT follows only after HTTP behavior is stable.
