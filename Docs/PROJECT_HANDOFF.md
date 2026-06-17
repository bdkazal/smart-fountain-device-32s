# Smart Fountain ESP32 Project Handoff

## Repository

```text
Firmware: bdkazal/smart-fountain-device-32s
Current branch: feature/state-persistence
Base: feature/wifi-onboarding
Laravel: bdkazal/biztola-iot-platform
Firmware version: smart-fountain-32s-state-0.2-persistence
```

Do not merge to `main` without explicit approval.

## Hardware

| Function | GPIO |
| --- | ---: |
| Pump button | 18 |
| COB button | 19 |
| Pump output | 25 |
| COB output | 26 |
| WS2812B | 27 |
| Water switch | 32 |
| Wi-Fi reset | 33 |

Pump and COB are ON/OFF only. GPIO32 LOW means low water. Water protection always has final authority.

## Control architecture

```text
local button
restore
Laravel command
schedule
safety
  -> FountainController
  -> HardwareOutputs
```

Actual applied hardware state is final truth.

## Completed baselines

- hardware initialization and local controls
- GPIO32 water protection
- WS2812B GRB validation
- Wi-Fi setup, retry, reset, and reprovisioning
- wrong-password retry
- stored-network connection
- actual pump/COB persistence and restoration
- safety-corrected pump persistence

## Preferences

Namespace:

```text
fountain
```

Current keys:

```text
wifi_ssid
wifi_pass
provision
state_blob
```

`state_blob` is versioned and CRC32-protected. It stores pump, COB, WS2812B enabled state, and RGB values.

Future schedules, timezone, scenes, and configuration revision must use a separate logical record in the same storage module.

## Validated persistence behavior

- PlatformIO build succeeded
- upload succeeded
- first boot without a record stayed safely OFF
- initial OFF state saved
- pump and COB local states saved
- pump and COB restored after power cycle
- restore source reported correctly
- unchanged restored state skipped a flash write
- GPIO32 initialized before pump restore
- low water rejected stored pump ON
- COB remained ON during low water
- safe pump OFF replaced the stored ON state
- later WATER OK reboot restored pump OFF
- Wi-Fi remained operational through the tested flow

Build result:

```text
RAM: 14.3% (46736 / 327680 bytes)
Flash: 63.8% (835877 / 1310720 bytes)
Result: SUCCESS
```

Recorded serial evidence:

```text
Stored actual fountain state loaded and verified.
COB state: ON source=restore
NeoPixel state: OFF color=0,0,0 source=restore
Pump state: ON source=restore
Actual fountain state unchanged. Flash write skipped.
```

The user confirmed the low-water boot and final safe-OFF reboot passed.

## Boot order

```text
outputs OFF
live GPIO32 state
FountainController
DeviceStorage
load state_blob
restore COB and WS2812B
request pump restore last
start Wi-Fi runtime
```

Water state is never restored from flash.

## Save behavior

State changes queue local persistence and future Laravel synchronization. Writes wait 300 ms to coalesce changes. Unchanged records skip writing. Failed writes retry with backoff.

## Remaining optional regression coverage

- invalid-size/checksum record injection
- rapid-toggle coalescing measurement
- deliberate NVS write-failure simulation
- WS2812B save/restore through a production command path
- GPIO33 reprovisioning while directly observing `state_blob`

These do not block the validated pump/COB persistence milestone.

## Next stage

Before firmware HTTP work:

1. Inspect `bdkazal/biztola-iot-platform` routes, controllers, models, migrations, resources, and existing device contracts.
2. Confirm desired state, reported actual state, commands, config revisions, and authentication.
3. Create `feature/laravel-http-integration` from this validated branch.
4. Add `X-DEVICE-KEY`, state reporting, config fetch, and command polling.
5. Keep all hardware actions inside `FountainController`.
6. Add compact offline configuration only after the API contract is confirmed.

MQTT remains later.
