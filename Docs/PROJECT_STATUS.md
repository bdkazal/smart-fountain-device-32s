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

## Current implementation

- [x] Versioned actual-state record in Preferences
- [x] Pump and COB enabled state
- [x] WS2812B enabled state and RGB color
- [x] CRC32 validation
- [x] Unchanged records skip flash writes
- [x] Short write-coalescing delay
- [x] Restore through `FountainController`
- [x] Live water input checked before pump restore
- [x] Wi-Fi reset preserves actual state

## Validation pending

- [ ] `pio run`
- [ ] Upload to ESP32-WROOM-32
- [ ] Power-cycle restoration
- [ ] Low-water restore protection
- [ ] Rapid-toggle write coalescing
- [ ] Wi-Fi reprovisioning preserves state
- [ ] Local runtime remains responsive

No build result is claimed yet because the current execution environment could not download the repository or PlatformIO dependencies.

## Next stage

After hardware validation, inspect `bdkazal/biztola-iot-platform` and create `feature/laravel-http-integration` from this branch. Keep actual device state separate from the future compact schedule/timezone/scene configuration cache.
