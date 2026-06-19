# Biztola Smart Fountain Device — ESP32-WROOM-32

Firmware for the Biztola Smart Fountain using an ESP32-WROOM-32 / generic 38-pin ESP32 board.

The firmware is local-first: physical controls, water protection, storage, Wi-Fi, and Laravel communication are separated into modules.

## Current milestone

**Stage 2 Laravel command polling — ready for hardware validation**

```text
Branch:   feature/laravel-api-handshake
Firmware: smart-fountain-32s-state-0.4-command-polling
```

This stage keeps the Laravel API handshake and adds the first interactive command path:

- `GET /api/device/config?device_uuid=<DEVICE_UUID>`
- `POST /api/device/heartbeat`
- `GET /api/device/commands?device_uuid=<DEVICE_UUID>`
- `POST /api/device/commands/{command}/ack`
- shared `X-DEVICE-KEY` device-auth header
- `state_apply` command execution through `FountainController`

Command polling runs about every 2 seconds while Wi-Fi and Laravel are healthy. Laravel already updates `last_seen_at` when the device checks `/api/device/commands`, so this polling loop is the V1 interactive Online signal for the dashboard.

This stage intentionally does **not** add MQTT, offline timelines, RTC sync, a NeoPixel effect engine, or full `/api/device/state` actual-state reporting. Actual-state reporting is the next major stage.

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
  after Wi-Fi connects, firmware fetches config
  if config.device_type is smart_fountain, firmware starts command polling
  heartbeat remains a slower diagnostics/background check
```

Pump and COB are ON/OFF only. RGB accepts enabled/color/brightness from Laravel. Brightness is applied by scaling the static RGB color. Animated RGB effects are intentionally left for a later NeoPixel effect-engine stage.

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

## Stage 2 validation target

Expected serial result after Wi-Fi and Laravel are reachable:

```text
Wi-Fi connected.
Laravel config HTTP status: 200
server_time_utc: ...
config.device_type: smart_fountain
config.config_revision: ...
Command poll HTTP status: 200
Laravel command polling is active; dashboard presence should stay fresh.
Heartbeat HTTP status: 200
```

Expected result when dashboard queues a Smart Fountain output action:

```text
Pending Laravel command: id=... type=state_apply
Command ACK HTTP status: id=... status=acknowledged http=200
Pump state: ... source=laravel
COB state: ... source=laravel
NeoPixel state: ... source=laravel
state_apply command applied through FountainController.
Command ACK HTTP status: id=... status=executed http=200
```

Also validate that local Pump/COB buttons, restored state, and water protection continue to work even when Laravel is unreachable.

## Current API behavior

```text
Command poll:
  GET /api/device/commands?device_uuid=<uuid>

Supported command type:
  state_apply

Unsupported command types:
  marked failed, not applied

ACK flow:
  pending -> acknowledged -> executed
  pending -> failed when unsupported or local protection prevents full application

Heartbeat:
  remains enabled for diagnostics: firmware version, IP, RSSI, last_seen_at
```

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
