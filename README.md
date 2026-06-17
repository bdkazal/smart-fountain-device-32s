# Biztola Smart Fountain Device — ESP32-WROOM-32

Firmware for the Biztola Smart Fountain using an ESP32-WROOM-32 / generic 38-pin ESP32 development board.

This repository follows the Biztola IoT Platform architecture: shared device concerns are separated from Smart Fountain product behavior, hardware safety remains local, and long-form documentation lives under `Docs/`.

## Current milestone

**Wi-Fi onboarding and local-first runtime**

The hardware foundation, local pump/COB buttons, water safety, and WS2812B output have been validated. The current branch adds stored Wi-Fi credentials, a boot-time reset button, a captive setup hotspot, asynchronous Wi-Fi scanning, non-blocking credential testing, and non-blocking station reconnect.

## Hardware pin map

| Function | GPIO |
| --- | ---: |
| Pump local button | 18 |
| COB local button | 19 |
| Pump control/output | 25 |
| COB control/output | 26 |
| WS2812B data | 27 |
| Water-level float switch | 32 |
| Wi-Fi setup/reset button | 33 |

## Wi-Fi behavior

```text
No stored Wi-Fi:
  start Fountain-Setup portal

Stored Wi-Fi:
  connect and reconnect without blocking local controls

GPIO33 held during boot for 3 seconds:
  clear only Wi-Fi credentials
  keep provisioning required until valid Wi-Fi is saved
  start setup portal
```

Local pump safety and physical controls continue running in setup, connecting, connected, and disconnected states.

## Documentation

| File | Purpose |
| --- | --- |
| [`Docs/PROJECT_STATUS.md`](Docs/PROJECT_STATUS.md) | Current milestone, completed work, and next steps |
| [`Docs/ARCHITECTURE.md`](Docs/ARCHITECTURE.md) | Module boundaries, runtime rules, and development order |
| [`Docs/HARDWARE_PIN_MAP.md`](Docs/HARDWARE_PIN_MAP.md) | Board target, GPIO assignments, and hardware rules |
| [`Docs/WIFI_SETUP_AND_RESET.md`](Docs/WIFI_SETUP_AND_RESET.md) | Wi-Fi storage, reset, captive portal, and test flow |
| [`Docs/TESTING_CHECKLIST.md`](Docs/TESTING_CHECKLIST.md) | Stage-by-stage validation checklist |

## Main source modules

```text
include/DeviceStorage.h       src/DeviceStorage.cpp
include/WifiManager.h         src/WifiManager.cpp
include/WifiReset.h           src/WifiReset.cpp
include/SetupPortal.h         src/SetupPortal.cpp
include/SetupPortalPage.h
include/LocalControls.h       src/LocalControls.cpp
include/FountainController.h  src/FountainController.cpp
include/HardwareOutputs.h     src/HardwareOutputs.cpp
include/WaterLevelSensor.h    src/WaterLevelSensor.cpp
```

## Build

```bash
pio run
```

## Upload

```bash
pio run --target upload
```

## Serial monitor

```bash
pio device monitor
```

## Development rules

- Keep `main.cpp` focused on orchestration.
- Put module interfaces in `include/*.h`.
- Put module implementations in `src/*.cpp`.
- Keep pump water-safety enforcement inside firmware.
- Initialize all outputs OFF.
- Keep network work and NeoPixel effects non-blocking.
- Wi-Fi reset must clear Wi-Fi only, never future Laravel cached config.
- Never commit Wi-Fi, Laravel device, API, or MQTT secrets.
