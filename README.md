# Biztola Smart Fountain Device — ESP32-WROOM-32

Firmware for the Biztola Smart Fountain using an ESP32-WROOM-32 / generic 38-pin ESP32 development board.

This repository follows the Biztola IoT Platform architecture: shared device concerns are separated from Smart Fountain product behavior, hardware safety remains local, and long-form documentation lives under `Docs/`.

## Current milestone

**Project foundation and hardware validation**

The project is intentionally starting with board, GPIO, output-safety, float-switch, reset-button, and WS2812B validation before Wi-Fi onboarding, Laravel HTTP, offline scheduling, or MQTT.

## Hardware direction

- ESP32-WROOM-32 development board
- pump control through suitable external driver hardware
- COB-light control through suitable external driver hardware
- WS2812B / NeoPixel decorative lighting
- float-switch water safety
- physical Wi-Fi setup/reset button

## Initial pin map

| Function | GPIO |
| --- | ---: |
| Pump control | 25 |
| COB control | 26 |
| WS2812B data | 27 |
| Water-level float switch | 32 |
| Wi-Fi setup/reset button | 33 |

The assignments remain provisional until hardware validation is complete.

## Documentation

| File | Purpose |
| --- | --- |
| [`Docs/PROJECT_STATUS.md`](Docs/PROJECT_STATUS.md) | Current milestone, completed work, and next steps |
| [`Docs/ARCHITECTURE.md`](Docs/ARCHITECTURE.md) | Module boundaries, runtime rules, and development order |
| [`Docs/HARDWARE_PIN_MAP.md`](Docs/HARDWARE_PIN_MAP.md) | Board target, GPIO assignments, and hardware rules |
| [`Docs/TESTING_CHECKLIST.md`](Docs/TESTING_CHECKLIST.md) | Stage-by-stage validation checklist |

## Project structure

```text
Docs/      documentation and instructions
include/   module headers
src/       module implementations and main.cpp
lib/       optional project-local libraries
test/      PlatformIO tests
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
- Keep NeoPixel effects non-blocking.
- Never commit Wi-Fi, Laravel device, API, or MQTT secrets.
