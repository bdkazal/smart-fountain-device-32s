# Architecture

## Purpose

This firmware is the ESP32-WROOM-32 implementation of the Biztola Smart Fountain. Shared platform concerns stay separate from product behavior, and hardware safety remains local.

## Directory convention

```text
Docs/      project documentation
include/   module interfaces
src/       implementations and main.cpp
lib/       optional project-local libraries
test/      PlatformIO tests
```

`README.md` is the project entry point. Long-form documentation belongs in `Docs/`.

## Module rule

A substantial module normally uses:

```text
include/ModuleName.h
src/ModuleName.cpp
```

`src/main.cpp` remains orchestration-focused. It initializes modules, defines startup order, calls cooperative update methods, and coordinates high-level runtime state. Detailed networking, parsing, storage formats, effects, and debounce logic belong in modules.

## Current module groups

### Shared device modules

- `DeviceStorage`
- `WifiManager`
- `WifiReset`
- `SetupPortal`

Future HTTP modules:

- `DeviceIdentity`
- `ApiClient`
- `CommandClient`
- `DeviceStateReporter`
- `DeviceClock`

### Smart Fountain modules

- `FountainController`
- `HardwareOutputs`
- `WaterLevelSensor`
- `LocalControls`

Future product modules:

- `FountainConfig`
- `NeoPixelController`
- `OfflineTimeline`

Do not create empty speculative modules.

## Shared control path

```text
local button ----\
restore ----------\
Laravel command ----> FountainController -> HardwareOutputs
schedule ----------/
water protection has final authority
```

All sources use the same controller. The device reports actual applied hardware state, not merely requested state.

Current control sources:

```text
boot
restore
local_button
laravel
schedule
safety
```

## Output model

Current hardware behavior is intentionally simple:

```text
pump: ON/OFF
COB: ON/OFF
WS2812B: enabled + RGB color
```

The abandoned ESP32-C3 prototype's pump PWM, COB PWM, and analog RGB implementation are not part of this board.

## Safe boot

Every boot follows this order:

```text
1. Configure pump, COB, and WS2812B physically OFF.
2. Initialize GPIO32 and read the live water state.
3. Initialize local controls and FountainController.
4. Initialize DeviceStorage.
5. Load and validate actual-state storage.
6. Restore COB and WS2812B through FountainController.
7. Request pump restore through FountainController.
8. Start setup-portal or stored-Wi-Fi runtime.
```

Pump restoration is last. A low-water reading rejects a restored pump request.

## Storage architecture

Use one `DeviceStorage` module and one Preferences namespace:

```text
fountain
```

Use separate logical records because they have different meanings and write frequency:

```text
actual-state record
  state_blob
  pump/COB/WS2812B actual applied state

future compact configuration record
  schedules, timezone, scenes, config revision
```

Wi-Fi keys remain:

```text
wifi_ssid
wifi_pass
provision
```

GPIO33 Wi-Fi reset removes only `wifi_ssid` and `wifi_pass`. It preserves actual state and future compact configuration.

The actual-state record is versioned, checksum-protected, compared before writing, and written after a short coalescing delay. Invalid records leave safe defaults active.

See `Docs/STATE_PERSISTENCE.md`.

## Water protection

Water protection is always local.

When low water is confirmed:

- pump is disabled
- pump-enable requests are rejected
- COB and decorative lighting remain available
- the safety-adjusted actual state becomes the state to persist and later report

This rule must not depend on Wi-Fi, Laravel, MQTT, or internet access.

## Runtime rules

- Keep the loop cooperative and non-blocking.
- Avoid long delays outside controlled startup or hardware tests.
- Local inputs and water protection run in every network mode.
- Storage writes must not control physical output timing.
- NeoPixel effects must later advance through `update()` and `millis()`.
- HTTP and MQTT must not create alternate hardware-control paths.

## Secrets

Never commit Wi-Fi passwords, Laravel device/API keys, or MQTT credentials. Tracked examples may document required fields; real values stay in ignored local files or future secure provisioning.

## Development order

1. hardware foundation
2. Wi-Fi onboarding
3. actual-state persistence
4. Laravel HTTP state/config/command flow
5. compact configuration and offline timeline
6. MQTT live flow

HTTP remains responsible for configuration and fallback workflows. MQTT is later.
