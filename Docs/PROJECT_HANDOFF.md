# Smart Fountain ESP32 Project Handoff

Use this document when continuing in a new thread or with another developer.

## Repository and branch

```text
Firmware: bdkazal/smart-fountain-device-32s
Current branch: feature/state-persistence
Branch base: feature/wifi-onboarding
Laravel: bdkazal/biztola-iot-platform
```

Do not continue from or merge to `main` without explicit approval.

## Product and board

```text
Brand: Biztola
Product: Smart Fountain
Board: ESP32-WROOM-32 generic 38-pin
PlatformIO board: esp32dev
Framework: Arduino
Firmware: smart-fountain-32s-state-0.2-persistence
```

## Pin map

| Function | GPIO | Rule |
| --- | ---: | --- |
| Pump button | 18 | `INPUT_PULLUP`, button to GND |
| COB button | 19 | `INPUT_PULLUP`, button to GND |
| Pump output | 25 | active HIGH |
| COB output | 26 | active HIGH |
| WS2812B | 27 | GRB order |
| Water switch | 32 | LOW means low water |
| Wi-Fi reset | 33 | hold during boot |

Final high-current loads require suitable drivers. Do not connect them directly to ESP32 GPIO.

## Product rules

```text
pump: ON/OFF only
COB: ON/OFF only
WS2812B: enabled + RGB color
```

Latest valid action wins, but local water protection has absolute priority. COB and decorative lighting remain available during low water.

Every source uses:

```text
FountainController -> HardwareOutputs
```

Current sources:

```text
boot
restore
local_button
laravel
schedule
safety
```

Actual applied hardware state is final truth.

## Completed baseline

Validated before this branch:

- safe output startup
- local pump and COB controls
- GPIO32 debounce and water protection
- WS2812B GRB output
- local-first runtime
- setup hotspot and captive page
- credential storage and verification
- wrong-password retry
- successful retry, restart, and stored-network connection
- stored-network retry direction
- GPIO33 reset/reprovisioning baseline

Recorded Wi-Fi test:

```text
SSID: Andromeda
IP: 192.168.0.102
wrong password: failed without restart
correct retry: saved, restarted, reconnected
```

The user confirmed the Wi-Fi baseline complete. Do not return to timing-specific credential-button testing unless a regression appears.

## Current milestone

**Actual-state persistence implementation is committed but not yet built or tested on hardware.**

Current Preferences namespace:

```text
fountain
```

Keys:

```text
wifi_ssid
wifi_pass
provision
state_blob
```

`state_blob` is a small versioned record containing:

- pump enabled
- COB enabled
- WS2812B enabled
- red, green, blue
- CRC32 checksum

The future compact Laravel configuration remains a different logical record for schedules, timezone, scenes, and configuration revision.

## Boot sequence

```text
1. Outputs initialize OFF.
2. GPIO32 initializes from the live switch.
3. Local controls and FountainController initialize.
4. DeviceStorage initializes.
5. state_blob loads and validates.
6. COB and WS2812B restore through FountainController.
7. Pump restore is requested last.
8. Water protection may reject pump restore.
9. Wi-Fi setup or station runtime starts.
```

Water state is never loaded from flash.

## Save behavior

A shared state-change notification queues:

```text
local persistence
future Laravel state sync
```

Persistence waits 300 ms to coalesce rapid changes. Unchanged records skip flash writes. Failed writes remain pending for retry.

GPIO33 reset removes only Wi-Fi SSID/password and preserves `state_blob`.

## Important modules

```text
include/DeviceStorage.h        src/DeviceStorage.cpp
include/FountainController.h   src/FountainController.cpp
include/HardwareOutputs.h      src/HardwareOutputs.cpp
include/WaterLevelSensor.h     src/WaterLevelSensor.cpp
include/LocalControls.h        src/LocalControls.cpp
include/WifiManager.h          src/WifiManager.cpp
include/WifiReset.h            src/WifiReset.cpp
include/SetupPortal.h          src/SetupPortal.cpp
src/main.cpp
```

`main.cpp` orchestrates startup and update order. Do not move HTTP parsing, large storage formats, or effects into it.

## Documentation source of truth

Read:

```text
Docs/PROJECT_STATUS.md
Docs/TESTING_CHECKLIST.md
Docs/STATE_PERSISTENCE.md
Docs/ARCHITECTURE.md
Docs/WIFI_SETUP_AND_RESET.md
Docs/HARDWARE_PIN_MAP.md
```

## Required next work

Validate `feature/state-persistence` one test at a time:

1. Build.
2. Upload and confirm safe first boot.
3. Toggle pump and COB, allow the save delay, restart, and confirm restore.
4. Set a WS2812B state in code or a controlled test path, then confirm color restore.
5. Store pump enabled, force low water before restart, and confirm pump remains disabled.
6. Confirm the safe result replaces the unsafe stored request.
7. Rapidly toggle and confirm only the final state is restored.
8. Enter Wi-Fi reprovisioning and confirm state remains preserved.
9. Confirm controls and water protection remain responsive during writes.

Record exact serial evidence and commit SHA in the checklist.

## Build commands

```bash
git fetch origin
git checkout feature/state-persistence
git pull origin feature/state-persistence
git status
pio run
pio run --target upload
pio device monitor
```

No build result is currently claimed. The assistant execution environment could not resolve GitHub or download PlatformIO dependencies.

## Laravel direction

Only after persistence validation:

1. Inspect the current backend repository and API routes.
2. Create `feature/laravel-http-integration` from this branch.
3. Add device identity and `X-DEVICE-KEY`.
4. Report actual state after boot, reconnect, and changes.
5. Fetch configuration and poll commands.
6. Apply every action through `FountainController`.
7. Keep desired configuration separate from reported actual state.
8. Add compact schedule/timezone/scene cache after confirming the contract.

MQTT remains later.

## Conventions

- Documentation directory is capital `Docs/`.
- Interfaces belong in `include/`.
- Implementations belong in `src/`.
- Network operations must not block local control.
- Outputs start OFF before restore.
- Never commit credentials, API keys, or device secrets.
- Do not use destructive Git commands.
