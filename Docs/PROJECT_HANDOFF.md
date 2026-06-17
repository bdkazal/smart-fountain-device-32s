# Smart Fountain ESP32 Project Handoff

Use this document when continuing the project in a new ChatGPT thread or with another developer.

## Repository and branch

```text
Firmware repository: bdkazal/smart-fountain-device-32s
Current branch: feature/wifi-onboarding
Base branch for this stage: feature/project-foundation
Laravel repository: bdkazal/biztola-iot-platform
```

Do not continue from `main` unless the user explicitly decides to merge completed work.

## Product

```text
Brand: Biztola
Product: Smart Fountain
Board: ESP32-WROOM-32 / generic 38-pin ESP32 development board
PlatformIO board: esp32dev
Framework: Arduino
```

Current firmware version:

```text
smart-fountain-32s-wifi-0.1-onboarding
```

## Main product behavior

The fountain has:

- pump output
- COB-light output
- WS2812B decorative lighting
- water-level float switch
- physical pump button
- physical COB button
- physical boot-time Wi-Fi reset/setup button
- future Laravel dashboard commands and schedules

The firmware is local-first. Physical controls and pump safety must continue working when Wi-Fi is missing, the setup portal is active, Laravel is unavailable, or reconnect attempts are running.

## Validated hardware pin map

| Function | GPIO | Behavior |
| --- | ---: | --- |
| Pump local button | 18 | `INPUT_PULLUP`, button to GND |
| COB local button | 19 | `INPUT_PULLUP`, button to GND |
| Pump control/output | 25 | Active HIGH; temporary LED indicator validated |
| COB control/output | 26 | Active HIGH; temporary LED indicator validated |
| WS2812B data | 27 | GRB order validated |
| Water-level float switch | 32 | `INPUT_PULLUP`; LOW means low water |
| Wi-Fi reset/setup button | 33 | `INPUT_PULLUP`; hold during boot |

Temporary output test wiring:

```text
GPIO25 -> 330 ohm resistor -> pump indicator LED -> GND
GPIO26 -> 330 ohm resistor -> COB indicator LED  -> GND
```

The final pump and COB loads will use suitable MOSFET/driver hardware. Never connect high-current loads directly to ESP32 GPIO.

## Confirmed control rules

### Pump

- Physical GPIO18 button toggles pump ON/OFF.
- Laravel dashboard commands and schedules will also be allowed to change pump state.
- There is no permanent local-override mode.
- The latest valid action wins.
- Low-water safety has absolute priority.
- When water is low, pump is forced OFF.
- When water is low, local, Laravel, and schedule pump-ON requests must be rejected.

### COB

- Physical GPIO19 button toggles COB ON/OFF.
- Laravel dashboard commands and schedules will also be allowed to change COB state.
- Water level does not restrict COB operation.

### State synchronization

Every actual state change should eventually be sent to Laravel, including changes caused by:

- physical buttons
- dashboard commands
- schedules
- water-safety overrides

Actual applied hardware state is the final truth.

## Current architecture

`main.cpp` is orchestration only.

Important modules:

```text
include/FountainController.h   src/FountainController.cpp
include/LocalControls.h        src/LocalControls.cpp
include/HardwareOutputs.h      src/HardwareOutputs.cpp
include/WaterLevelSensor.h     src/WaterLevelSensor.cpp
include/DeviceStorage.h        src/DeviceStorage.cpp
include/WifiManager.h          src/WifiManager.cpp
include/WifiReset.h            src/WifiReset.cpp
include/SetupPortal.h          src/SetupPortal.cpp
include/SetupPortalPage.h
include/WifiConfig.h
```

Architecture rule:

```text
local button ----\
Laravel command --- > FountainController -> HardwareOutputs
schedule --------/
water safety has final authority
```

Do not create separate output behavior for local and remote control. All control sources must use `FountainController`.

## Hardware validation already passed

- PlatformIO build and upload
- ESP32 serial monitor at 115200
- safe output startup OFF
- GPIO18 pump button debounce/toggle
- GPIO19 COB button debounce/toggle
- GPIO25 pump indicator output
- GPIO26 COB indicator output
- GPIO32 water-level input and debounce
- pump forced OFF during low water
- pump button blocked during low water
- COB still works during low water
- GPIO33 HIGH/LOW input validation
- WS2812B OFF/red/green/blue/white
- WS2812B GRB color order
- non-blocking local runtime

## Wi-Fi design baseline

Before implementing Wi-Fi, these repositories were reviewed:

```text
bdkazal/smart-plant-bed-device
bdkazal/smart-plant-bed-device-c3
```

The Smart Fountain keeps the improved C3 user flow but uses a cleaner implementation.

Retained behavior:

- Preferences/NVS Wi-Fi storage
- setup hotspot
- setup page loads before scanning
- scanned networks plus manual/hidden SSID
- password show/hide
- wrong-password retry
- save and restart after valid credentials
- boot-time Wi-Fi reset
- local controls continue while offline/setup is active

Further improvements in the fountain firmware:

- persistent provisioning-required flag instead of a one-boot flag
- Wi-Fi reset clears only Wi-Fi keys
- no automatic development-router fallback
- non-blocking normal station connection
- asynchronous Wi-Fi scanning
- non-blocking submitted-credential test
- browser polling through `/setup-status`
- captive DNS redirect
- setup HTML separated from networking implementation
- storage writes verified after save
- configurable ESP32-WROOM behavior instead of blindly copying C3 radio settings

## Current Wi-Fi behavior

Preferences namespace:

```text
fountain
```

Keys:

```text
wifi_ssid
wifi_pass
provision
```

First boot without stored credentials:

```text
provision=true
start Fountain-Setup hotspot
start captive DNS
serve setup page at 192.168.4.1
```

Successful setup:

```text
test submitted credentials
save and verify credentials
set provision=false
restart
connect using stored Wi-Fi
```

Normal router outage:

```text
do not automatically expose setup hotspot
remain locally operational
retry stored Wi-Fi periodically
```

Changing Wi-Fi requires deliberate GPIO33 reset/setup action.

## Wi-Fi validation already passed

Latest successful hardware test:

```text
Setup portal accepted SSID: Andromeda
Wi-Fi credentials saved and verified
Device restarted automatically
Stored SSID was found on next boot
Non-blocking station connection started
Connected IP: 192.168.0.102
Observed RSSI: approximately -35 to -36 dBm
```

Relevant serial evidence:

```text
Testing submitted Wi-Fi SSID: Andromeda
Wi-Fi credentials saved and verified.
Submitted Wi-Fi credentials worked and were saved.
Temporary station IP: 192.168.0.102
Restarting with saved Wi-Fi credentials...

Loading stored Wi-Fi credentials...
Stored SSID: Andromeda
Starting non-blocking Wi-Fi connection to SSID: Andromeda
Wi-Fi connected.
IP address: 192.168.0.102
RSSI dBm: -36
```

Pump, COB, NeoPixels, and water state remained safely initialized during restart and connection.

## Remaining Wi-Fi tests

Do these before starting Laravel integration:

1. Wrong router password remains on the setup page and permits retry.
2. Pump/COB buttons remain responsive during credential testing.
3. Low-water pump safety remains responsive during credential testing.
4. Router outage does not automatically start the hotspot.
5. Stored-network reconnect retries occur while local controls continue.
6. Device reconnects when the router returns.
7. Releasing GPIO33 before 3 seconds cancels Wi-Fi reset.
8. Holding GPIO33 during boot for 3 seconds clears Wi-Fi only.
9. Persistent provisioning mode starts `Fountain-Setup` after reset.
10. New credentials can be saved after reprovisioning.

Record results in:

```text
Docs/PROJECT_STATUS.md
Docs/TESTING_CHECKLIST.md
Docs/WIFI_SETUP_AND_RESET.md
```

## Laravel direction after Wi-Fi validation

Do not begin by writing firmware API code from memory. First inspect the current Laravel repository and the existing Plant Bed device contracts.

Laravel repository:

```text
bdkazal/biztola-iot-platform
```

Device authentication uses:

```http
X-DEVICE-KEY: <device key>
```

Expected platform concepts:

- device UUID
- device API key
- heartbeat
- config fetch
- command polling
- command acknowledgement/completion
- actual state synchronization
- cached config for offline behavior

The first HTTP integration should support Smart Fountain persistent-state behavior:

```text
Laravel command -> FountainController -> safety check -> actual hardware state
actual hardware state -> Laravel state sync
```

Local button changes and water-safety overrides must update Laravel once reachable.

MQTT is later. HTTP behavior must be stable first.

Existing platform MQTT topic direction for later work:

```text
biztola/v1/devices/{uuid}/commands
biztola/v1/devices/{uuid}/command-events
biztola/v1/devices/{uuid}/state
biztola/v1/devices/{uuid}/presence
```

## Git workflow

Current work belongs on:

```text
feature/wifi-onboarding
```

Before editing:

```bash
git fetch origin
git checkout feature/wifi-onboarding
git pull origin feature/wifi-onboarding
git status
```

Do not use destructive Git commands. Do not merge to `main` without explicit approval.

After the remaining Wi-Fi tests pass, update documentation and then create a Laravel HTTP integration branch from the validated Wi-Fi branch.

Suggested next branch name:

```text
feature/laravel-http-integration
```

## Project conventions

- Documentation directory is capital `Docs/`.
- Header files belong in `include/`.
- Implementations belong in `src/`.
- `main.cpp` should orchestrate only.
- Keep code modular and reusable across future Biztola devices.
- Network operations must not block local safety/control.
- Outputs initialize OFF.
- Never commit Wi-Fi credentials, device API keys, Laravel secrets, or MQTT credentials.
- Do not repeat existing secrets in chat or documentation.
- Verify the current repository before changing code.
- Update docs after every validated milestone.

## First task in the next thread

1. Read this handoff document.
2. Inspect the current `feature/wifi-onboarding` branch.
3. Review `Docs/PROJECT_STATUS.md`, `Docs/TESTING_CHECKLIST.md`, and `Docs/WIFI_SETUP_AND_RESET.md`.
4. Continue the remaining Wi-Fi reset/offline/reconnect tests one at a time.
5. Do not start Laravel HTTP integration until Wi-Fi onboarding is fully validated and documented.
