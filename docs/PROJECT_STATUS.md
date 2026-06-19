# Project Status

## Repository

```text
bdkazal/smart-fountain-device-32s
```

## Current branch

```text
feature/state-persistence
```

Base branch:

```text
feature/wifi-onboarding
```

Do not merge to `main` without explicit approval.

## Current firmware

```text
smart-fountain-32s-state-0.2-persistence
```

## Completed baseline

- [x] ESP32-WROOM-32 hardware foundation
- [x] Local buttons and water protection
- [x] WS2812B validation
- [x] Wi-Fi setup portal
- [x] Stored Wi-Fi connection and retry
- [x] Wrong-password retry
- [x] GPIO33 reset and reprovisioning baseline

## State persistence implementation

- [x] Versioned actual-state record in Preferences
- [x] Pump and COB enabled state
- [x] WS2812B enabled state and RGB color fields
- [x] CRC32 validation
- [x] Unchanged records skip flash writes
- [x] Short write-coalescing delay
- [x] Restore through `FountainController`
- [x] Live water input checked before pump restore
- [x] Wi-Fi reset preserves actual state by design

## Hardware validation passed

- [x] `pio run` succeeds
- [x] Firmware uploads to ESP32-WROOM-32
- [x] First boot without `state_blob` remains safely OFF
- [x] Initial safe state saves successfully
- [x] Pump and COB local changes save
- [x] Pump and COB restore after power cycle
- [x] Restore source is reported correctly
- [x] Unchanged restored state skips another flash write
- [x] Low water rejects stored pump ON
- [x] COB remains restored during low water
- [x] Safety-adjusted pump OFF is saved
- [x] Final normal-water reboot restores pump OFF
- [x] Wi-Fi connection remains operational during save and restore

Recorded build result:

```text
Platform: Espressif 32 6.10.0
RAM: 14.3% (46736 / 327680 bytes)
Flash: 63.8% (835877 / 1310720 bytes)
Build: SUCCESS
```

Recorded runtime evidence:

```text
Stored actual fountain state loaded and verified.
COB state: ON source=restore
Pump state: ON source=restore
Actual fountain state unchanged. Flash write skipped.
```

The user also confirmed the low-water restore and final safe-OFF reboot tests passed.

## Additional regression tests not yet recorded

- [ ] Inject invalid-size or invalid-checksum state data
- [ ] Exercise WS2812B state/color through a production control path
- [ ] Deliberately test rapid-toggle write coalescing
- [ ] Deliberately simulate a failed NVS write
- [ ] Re-run GPIO33 reprovisioning while confirming `state_blob` survives

These are useful regression tests, but they do not block the validated pump/COB persistence and water-safety milestone.

## Next stage

Inspect `bdkazal/biztola-iot-platform` before writing firmware API code. Then create `feature/laravel-http-integration` from this validated branch. Keep actual device state separate from the future compact schedule/timezone/scene configuration cache.
