# Architecture

## Purpose

This firmware is the ESP32-WROOM-32 implementation of the Biztola Smart Fountain. It should follow the same platform-minded rules as `bdkazal/biztola-iot-platform` while keeping hardware-specific behavior isolated.

## Directory convention

```text
Docs/      project documentation and operating instructions
include/   public module headers (`.h`)
src/       module implementations (`.cpp`) and `main.cpp`
lib/       optional project-local libraries
 test/      PlatformIO tests
```

Do not place long-form documentation in the repository root. The root `README.md` is only the project entry point and documentation map.

## Module rule

Each substantial module should normally have:

```text
include/ModuleName.h
src/ModuleName.cpp
```

Headers define the public interface. Implementation details belong in `.cpp` files.

## `main.cpp` responsibility

`src/main.cpp` should remain small and orchestration-focused. It may:

- initialize modules
- define the startup order
- call non-blocking module update methods
- coordinate high-level online/offline behavior

It should not contain:

- setup-portal HTML
- detailed HTTP request code
- large JSON parsers
- NeoPixel animation implementations
- GPIO debounce algorithms
- large command-processing branches

## Planned module groups

### Shared device/platform modules

- `DeviceIdentity`
- `DeviceStorage`
- `WifiManager`
- `WifiReset`
- `SetupPortal`
- `ApiClient`
- `CommandClient`
- `DeviceStateReporter`
- `DeviceClock`
- `MqttClient`

### Smart Fountain modules

- `FountainTypes`
- `FountainConfig`
- `FountainController`
- `HardwareOutputs`
- `WaterLevelSensor`
- `NeoPixelController`
- `OfflineTimeline`

Modules should be introduced only when their development stage begins. Do not create empty speculative modules.

## Product behavior

The Smart Fountain is a persistent-state device. The firmware maintains the actual applied state of:

- pump
- COB light
- decorative WS2812B lighting

The device must report actual applied state, not merely requested state.

## Safety rules

### Safe boot

On every boot:

- pump output starts OFF
- COB output starts OFF
- WS2812B output starts OFF
- cached or cloud state is applied only after hardware initialization

### Water safety

Water-low protection is local and always active.

When low water is confirmed:

- pump turns OFF immediately
- pump-on requests are rejected or overridden locally
- COB and decorative lighting may remain available

This rule must not depend on Wi-Fi, Laravel, MQTT, or internet access.

## Runtime rules

- Keep the main loop cooperative and non-blocking.
- Avoid long `delay()` calls outside controlled startup or hardware-test steps.
- NeoPixel effects advance through `update()` calls based on `millis()`.
- Wi-Fi, HTTP, MQTT, safety inputs, and output updates must remain responsive.

## Configuration and secrets

Tracked example:

```text
include/DeviceSecrets.example.h
```

Local ignored file:

```text
include/DeviceSecrets.h
```

Never commit Wi-Fi passwords, Laravel API keys, device keys, or MQTT passwords.

## Cloud communication direction

Development order:

1. stable hardware foundation
2. Wi-Fi setup and stored credentials
3. Laravel HTTP configuration/state/command flow
4. offline cache and fallback behavior
5. MQTT live command, presence, event, and state flow

HTTP remains responsible for configuration and fallback workflows. MQTT will later handle live commands and events.
