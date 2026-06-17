# Project Status

## Project

**Repository:** `bdkazal/smart-fountain-device-32s`

ESP32-WROOM-32 firmware for the Biztola Smart Fountain product.

## Current milestone

**Project foundation and hardware validation**

The modular foundation is running on the target ESP32-WROOM-32 board. Board startup, input handling, safe output initialization, and WS2812B color output have been verified.

## Hardware direction

- ESP32-WROOM-32 / generic 38-pin ESP32 development board
- Pump controlled through an external MOSFET driver
- COB light controlled through an external MOSFET or suitable driver
- WS2812B / NeoPixel decorative lighting
- Float-switch water safety input
- Physical Wi-Fi setup/reset button

## Initial pin map

| Function | GPIO | Status |
| --- | ---: | --- |
| Pump control signal | 25 | Boots safely OFF; ON signal still to validate |
| COB control signal | 26 | Boots safely OFF; ON signal still to validate |
| WS2812B data | 27 | Validated |
| Water-level float switch | 32 | Validated |
| Wi-Fi setup/reset button | 33 | Input validated |

## Completed

- [x] PlatformIO project created with `esp32dev`
- [x] Modular documentation and source structure established
- [x] Central hardware pin configuration added
- [x] Firmware identity module added
- [x] Safe hardware output module added
- [x] Debounced water-level input module added
- [x] Modular hardware-validation runtime uploaded successfully
- [x] Pump, COB, and WS2812B initialize OFF
- [x] GPIO32 water input validated HIGH/LOW with clean debounce
- [x] GPIO33 button input validated HIGH/LOW
- [x] WS2812B OFF, red, green, blue, and white validated
- [x] GRB color order confirmed
- [x] NeoPixel validation remains non-blocking
- [x] White test intensity reduced for fairer visual comparison

## In progress

- [ ] Validate GPIO25 pump control signal without connecting the pump
- [ ] Validate GPIO26 COB control signal without connecting the COB load
- [ ] Implement reliable boot-time hold detection for the setup/reset button

## Next steps

1. Validate GPIO25 and GPIO26 using only a meter or small indicator test circuit.
2. Keep the actual pump and COB load disconnected during signal validation.
3. Add the boot-time Wi-Fi reset hold module.
4. Begin modular Wi-Fi storage, connection, and setup-portal work after output-signal validation.

## Development rule

Do not add Laravel API or MQTT integration until board GPIO assignments, output safety, local water safety, Wi-Fi onboarding, and basic offline behavior are stable.
