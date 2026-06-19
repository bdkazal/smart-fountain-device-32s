# Hardware Pin Map

## Target board

```text
ESP32-WROOM-32 / generic 38-pin ESP32 development board
PlatformIO board: esp32dev
Framework: Arduino
```

## Pin assignments

| Function | GPIO | Direction | Behavior |
| --- | ---: | --- | --- |
| Pump local button | 18 | Input | `INPUT_PULLUP`, button to GND |
| COB local button | 19 | Input | `INPUT_PULLUP`, button to GND |
| Pump control signal | 25 | Output | Active HIGH, safe boot LOW |
| COB control signal | 26 | Output | Active HIGH, safe boot LOW |
| WS2812B data | 27 | Output | Digital data |
| Water-level float switch | 32 | Input | `INPUT_PULLUP`, switch to GND |
| Wi-Fi setup/reset button | 33 | Input | `INPUT_PULLUP`, button to GND |

## Local control buttons

```text
GPIO18 -> momentary pump button -> GND
GPIO19 -> momentary COB button  -> GND
```

Each debounced press toggles the related output. A local button action updates the same firmware state that future Laravel commands and schedules will use. There is no permanent local override mode; the latest valid action wins.

## Pump safety

Water safety has priority over every control source.

```text
water OK  -> button, Laravel, or schedule may set pump ON/OFF
water low -> pump forced OFF; pump ON requests are rejected
```

COB control is not restricted by water level.

## Output validation wiring

Temporary indicator LEDs may be used before MOSFET integration:

```text
GPIO25 -> 330 ohm resistor -> pump indicator LED -> GND
GPIO26 -> 330 ohm resistor -> COB indicator LED  -> GND
```

The final pump and COB loads must use suitably rated external driver hardware. Do not power high-current loads from an ESP32 GPIO.

## WS2812B / NeoPixel

Use GPIO27 for the data signal. A proper logic-level shifter and a small series data resistor are recommended for reliable 5 V WS2812B operation.

Use a regulated external 5 V supply sized for the LED count, with a common ground between the ESP32 and LED supply.

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
