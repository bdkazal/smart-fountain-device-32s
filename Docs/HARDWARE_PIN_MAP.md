# Hardware Pin Map

## Target board

```text
ESP32-WROOM-32 / generic 38-pin ESP32 development board
PlatformIO board: esp32dev
Framework: Arduino
```

All assignments remain provisional until verified on the actual board.

## Initial pin assignments

| Function | GPIO | Direction | Behavior |
| --- | ---: | --- | --- |
| Pump control signal | 25 | Output | Active HIGH, safe boot LOW |
| COB control signal | 26 | Output | Active HIGH, safe boot LOW |
| WS2812B data | 27 | Output | Digital data |
| Water-level float switch | 32 | Input | `INPUT_PULLUP`, switch to GND |
| Wi-Fi setup/reset button | 33 | Input | `INPUT_PULLUP`, button to GND |

## Safety notes

- High-current loads must use suitable external driver hardware.
- Do not power the pump, COB light, or a full LED strip from an ESP32 GPIO.
- All control-system grounds must share a common reference.
- Pump, COB, and decorative lighting must initialize OFF.

## WS2812B / NeoPixel

Use GPIO27 for the data signal. A proper logic-level shifter and a small series data resistor are recommended for reliable 5 V WS2812B operation.

Use a regulated external 5 V supply sized for the LED count. Start testing with a short strip and low brightness.

## Water-level float switch

```text
GPIO32 -> float switch -> GND
```

Expected logic:

```text
switch open   -> HIGH -> water OK
switch closed -> LOW  -> low water
```

## Wi-Fi setup/reset button

```text
GPIO33 -> momentary button -> GND
```

Expected logic:

```text
released -> HIGH
pressed  -> LOW
```

## Pins avoided by the initial design

- GPIO6-GPIO11: normally connected to onboard flash
- GPIO1/GPIO3: serial TX/RX when serial logging is required
- GPIO34-GPIO39: input-only and no internal pull-up
- GPIO0, GPIO2, GPIO5, GPIO12, GPIO15: boot-strapping pins

## Validation order

1. Verify serial output.
2. Verify input pins remain stable.
3. Verify output pins boot LOW.
4. Test control signals before attaching final loads.
5. Test a short WS2812B section at low brightness.
