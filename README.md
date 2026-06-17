# Biztola Smart Fountain Device — ESP32-WROOM-32

Firmware for the Biztola Smart Fountain using an ESP32-WROOM-32 / generic 38-pin ESP32 board.

The firmware is local-first: physical controls, water protection, storage, Wi-Fi, and future Laravel communication are separated into modules.

## Current milestone

**Safe actual-state persistence — hardware validated**

```text
Branch:   feature/state-persistence
Firmware: smart-fountain-32s-state-0.2-persistence
```

The Wi-Fi onboarding baseline is complete. Pump and COB state now save locally and restore after restart or power loss. A stored pump-ON state is rejected when the live water input reports low water, and the resulting safe OFF state becomes the next restored state.

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
  Wi-Fi setup or stored-network runtime starts
```

Pump and COB are ON/OFF only. The persistence record also supports WS2812B enabled status and RGB color, although a production NeoPixel command path is still future work.

Actual-state persistence uses the existing `fountain` Preferences namespace with key:

```text
state_blob
```

Wi-Fi reset clears only stored SSID/password and preserves actual state by design.

## Validated persistence result

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
- Keep water protection local.
- Initialize outputs OFF before restoration.
- Keep network, storage, and future effects cooperative.
- Keep actual state separate from compact Laravel configuration.
- Never commit credentials or device/API secrets.
- Do not merge to `main` without explicit approval.
