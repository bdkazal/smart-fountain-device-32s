# Project Status

## Project

**Repository:** `bdkazal/smart-fountain-device-32s`

ESP32-WROOM-32 firmware for the Biztola Smart Fountain product.

## Current milestone

**Project foundation and hardware validation**

The immediate goal is to establish a clean modular firmware structure and verify the selected ESP32-WROOM-32 board before adding Wi-Fi onboarding, Laravel HTTP integration, offline behavior, or MQTT.

## Hardware direction

- ESP32-WROOM-32 / generic 38-pin ESP32 development board
- Pump controlled through an external MOSFET driver
- COB light controlled through an external MOSFET or suitable driver
- WS2812B / NeoPixel decorative lighting
- Float-switch water safety input
- Physical Wi-Fi setup/reset button

## Planned initial pin map

| Function | GPIO | Status |
| --- | ---: | --- |
| Pump MOSFET signal | 25 | Provisional until tested |
| COB driver signal | 26 | Provisional until tested |
| WS2812B data | 27 | Provisional until tested |
| Water-level float switch | 32 | Provisional until tested |
| Wi-Fi setup/reset button | 33 | Provisional until tested |

## Completed

- [x] PlatformIO project created
- [x] PlatformIO board target set to `esp32dev`
- [x] GitHub repository created
- [x] Initial source pushed to `main`
- [x] Documentation convention selected: project documents under `Docs/`
- [x] Code convention selected: headers under `include/`, implementations under `src/`

## In progress

- [ ] Establish project documentation and architecture
- [ ] Create safe hardware pin configuration
- [ ] Create minimal modular hardware validation firmware

## Next steps

1. Build and upload a minimal serial/board test.
2. Confirm all high-power outputs boot OFF.
3. Validate the float-switch input on GPIO32.
4. Validate the setup/reset button input on GPIO33.
5. Validate a short WS2812B test strip on GPIO27 using proper external 5 V power and common ground.
6. Add Wi-Fi onboarding only after the hardware foundation is stable.

## Development rule

Do not add Laravel API or MQTT integration until the board, GPIO assignments, output safety, float switch, and NeoPixel hardware pass the initial test checklist.
