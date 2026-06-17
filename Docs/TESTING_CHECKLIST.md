# Testing Checklist

## Stage 1 — Foundation

- [x] PlatformIO build and upload
- [x] Serial monitor at 115200
- [x] Main loop remains responsive
- [x] Pump, COB, and WS2812B start disabled

## Stage 2 — Local hardware

- [x] GPIO18 toggles pump
- [x] GPIO19 toggles COB
- [x] One press produces one toggle
- [x] GPIO32 water input is debounced
- [x] Low water disables and blocks pump
- [x] COB remains usable during low water
- [x] GPIO27 WS2812B GRB order validated

## Stage 3 — Wi-Fi onboarding

- [x] No stored network starts `Fountain-Setup`
- [x] Captive/manual setup page works
- [x] Asynchronous network scan works
- [x] Manual SSID and password controls work
- [x] Correct credentials save and verify
- [x] Successful setup restarts and reconnects
- [x] Wrong password leaves the portal active for retry
- [x] Local controls remain available in setup mode
- [x] Stored-network connection is non-blocking
- [x] Ordinary outage does not deliberately start the portal
- [x] Periodic reconnect behavior is accepted
- [x] GPIO33 short hold cancels reset
- [x] GPIO33 confirmed hold clears Wi-Fi only
- [x] Provisioning mode survives until setup succeeds
- [x] New credentials can be saved after reprovisioning

Latest recorded evidence:

```text
Firmware: smart-fountain-32s-wifi-0.1-onboarding
SSID: Andromeda
Wrong-password retry: passed
Stored-Wi-Fi IP: 192.168.0.102
```

## Stage 4 — Actual-state persistence

Implementation branch:

```text
feature/state-persistence
```

Firmware:

```text
smart-fountain-32s-state-0.2-persistence
```

### Build and startup

- [ ] `pio run` succeeds
- [ ] Firmware uploads
- [ ] No stored state keeps safe defaults
- [ ] Invalid-size record keeps safe defaults
- [ ] Invalid-checksum record keeps safe defaults

### Save behavior

- [ ] Pump change saves after the coalescing delay
- [ ] COB change saves after the coalescing delay
- [ ] WS2812B enabled state and RGB color save
- [ ] Unchanged state skips the flash write
- [ ] Rapid toggles save only the final state
- [ ] A failed save remains pending for retry

### Restore behavior

- [ ] Pump state restores after normal restart
- [ ] COB state restores after normal restart
- [ ] WS2812B enabled state and RGB color restore
- [ ] Restored changes report source `restore`
- [ ] Live GPIO32 state is read before pump restore
- [ ] Low water rejects stored pump operation
- [ ] The resulting safe state replaces the unsafe stored request

### Integration safety

- [ ] Local buttons remain responsive during persistence
- [ ] Water protection remains responsive during persistence
- [ ] Setup portal remains responsive during persistence
- [ ] Stored-network retry remains responsive during persistence
- [ ] GPIO33 Wi-Fi reset preserves `state_blob`

## Stage 5 — Laravel HTTP

- [ ] Inspect the current backend contract first
- [ ] Authenticate with `X-DEVICE-KEY`
- [ ] Report actual state after boot and reconnect
- [ ] Local changes update Laravel
- [ ] Config fetch succeeds
- [ ] Command polling succeeds
- [ ] Commands acknowledge and complete correctly
- [ ] All actions use `FountainController`
- [ ] Safety-adjusted actual state is reported
- [ ] Compact configuration cache remains separate from actual state

## Stage 6 — Offline schedules

- [ ] Cache approved timezone and timeline data
- [ ] Validate time before applying a cached schedule
- [ ] Handle ranges that cross midnight
- [ ] Apply schedules through `FountainController`
- [ ] Sync locally applied state after Laravel recovers

## Stage 7 — MQTT

- [ ] MQTT work begins only after HTTP is stable
- [ ] Commands remain device-scoped
- [ ] Presence and actual state are published
- [ ] HTTP fallback remains functional

## Evidence rule

For each completed milestone record:

- commit SHA
- board and wiring
- firmware version
- serial summary
- pass/fail result
- known limitations
