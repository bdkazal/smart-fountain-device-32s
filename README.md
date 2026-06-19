# Biztola Smart Fountain Device — ESP32-WROOM-32

Firmware for the Biztola Smart Fountain using an ESP32-WROOM-32 / generic 38-pin ESP32 board.

The firmware is local-first: physical controls, water protection, storage, Wi-Fi, and Laravel communication are separated into modules.

## Current milestone

**Stage 1 Laravel API handshake — ready for hardware validation**

```text
Branch:   feature/laravel-api-handshake
Firmware: smart-fountain-32s-state-0.3-laravel-handshake
```

This stage adds only the Laravel API handshake:

- `GET /api/device/config?device_uuid=<DEVICE_UUID>`
- `POST /api/device/heartbeat`
- shared `X-DEVICE-KEY` device-auth header
- serial logging for config status, `server_time_utc`, `config.device_type`, `config.config_revision`, and heartbeat status

This stage intentionally does **not** poll commands, ACK commands, apply `state_apply`, execute offline timelines, update RTC, run a NeoPixel effect engine, or report water readings.

## Hardware pin map

| Function | GPIO |
| --- | ---: |
| Pump local button | 18 |
| COB local button | 19 |
| Pump control/output | 25 |
| COB control/output | 26 |
| WS2812B data | 27 |
| Water-level switch | 32 |
| Wi-Fi setup/reset button | 33 |

## Runtime behavior

```text
boot
  outputs start OFF
  live water state initializes
  stored actual state is validated
  COB and WS2812B restore
  pump restore passes through water protection
  Laravel API client initializes without touching outputs
  Wi-Fi setup or stored-network runtime starts
  after Wi-Fi connects, firmware fetches config then posts heartbeat
```

Pump and COB are ON/OFF only. The persistence record also supports WS2812B enabled status and RGB color, although a production NeoPixel command path is still future work.

Actual-state persistence uses the existing `fountain` Preferences namespace with key:

```text
state_blob
```

Wi-Fi reset clears only stored SSID/password and preserves actual state by design.

## Device secrets

Copy the example file and fill in real local values:

```bash
cp include/DeviceSecrets.example.h include/DeviceSecrets.h
```

Required values:

```cpp
#define API_BASE_URL "http://192.168.0.xxx:8000"
#define DEVICE_UUID "YOUR_SMART_FOUNTAIN_DEVICE_UUID"
#define DEVICE_API_KEY "YOUR_SMART_FOUNTAIN_DEVICE_API_KEY"
```

From the ESP32, never use `127.0.0.1` or `localhost` for Laravel. Use your Mac/server LAN IP address, for example `http://192.168.0.105:8000`.

`WIFI_SSID` and `WIFI_PASSWORD` in `DeviceSecrets.h` are only a development fallback. The production path remains the setup portal and stored credentials.

## Stage 1 validation target

Expected serial result:

```text
Wi-Fi connected.
Laravel config HTTP status: 200
server_time_utc: ...
config.device_type: smart_fountain
config.config_revision: ...
Heartbeat HTTP status: 200
```

Also validate failure behavior:

- Stop Laravel or use a wrong API URL.
- Pump button, COB button, restored state, and low-water pump protection must still work.
- The firmware should retry Laravel later without blocking the local fountain runtime permanently.

## Previously validated persistence result

```text
Build: SUCCESS
RAM: 14.3%
Flash: 63.8%
Pump local save/restore: passed
COB local save/restore: passed
Low-water restored-pump rejection: passed
Safety-adjusted OFF persistence: passed
Unchanged flash-write skip: passed
```

## Wi-Fi behavior

```text
No stored Wi-Fi:
  start Fountain-Setup

Stored Wi-Fi:
  connect and retry without blocking local controls

Development DeviceSecrets.h Wi-Fi fallback:
  connect to WIFI_SSID when no stored credentials exist and provisioning is not forced

GPIO33 held during boot for 3 seconds:
  clear Wi-Fi credentials only
  keep provisioning required
  start Fountain-Setup
```

Wrong-password retry, successful save/restart, and stored-network connection have passed the accepted Wi-Fi baseline.

## Documentation

| File | Purpose |
| --- | --- |
| [`Docs/PROJECT_STATUS.md`](Docs/PROJECT_STATUS.md) | Current branch, milestone, and pending work |
| [`Docs/PROJECT_HANDOFF.md`](Docs/PROJECT_HANDOFF.md) | Continuation context |
| [`Docs/ARCHITECTURE.md`](Docs/ARCHITECTURE.md) | Module and runtime rules |
| [`Docs/HARDWARE_PIN_MAP.md`](Docs/HARDWARE_PIN_MAP.md) | GPIO and hardware rules |
| [`Docs/WIFI_SETUP_AND_RESET.md`](Docs/WIFI_SETUP_AND_RESET.md) | Wi-Fi onboarding and reset behavior |
| [`Docs/STATE_PERSISTENCE.md`](Docs/STATE_PERSISTENCE.md) | Actual-state storage, restore, and validation |
| [`Docs/TESTING_CHECKLIST.md`](Docs/TESTING_CHECKLIST.md) | Validation checklist |

## Main modules

```text
include/DeviceStorage.h       src/DeviceStorage.cpp
include/WifiManager.h         src/WifiManager.cpp
include/WifiReset.h           src/WifiReset.cpp
include/SetupPortal.h         src/SetupPortal.cpp
include/LocalControls.h       src/LocalControls.cpp
include/FountainController.h  src/FountainController.cpp
include/HardwareOutputs.h     src/HardwareOutputs.cpp
include/WaterLevelSensor.h    src/WaterLevelSensor.cpp
include/ApiClient.h           src/ApiClient.cpp
include/LaravelApiClient.h    src/LaravelApiClient.cpp
```

## Build and upload

```bash
pio run
pio run --target upload
pio device monitor
```

## Development rules

- Keep `main.cpp` orchestration-focused.
- Route every output source through `FountainController`.
- Keep water protection local and final.
- Initialize outputs OFF before restoration.
- Keep network, storage, API, and future effects cooperative.
- Keep actual state separate from Laravel desired state.
- Never commit credentials or device/API secrets.
- Do not merge to `main` without explicit approval.
