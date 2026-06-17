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

```text
Branch: feature/state-persistence
Firmware: smart-fountain-32s-state-0.2-persistence
```

### Build and startup

- [x] `pio run` succeeds
- [x] Firmware uploads
- [x] No stored state keeps safe defaults
- [ ] Invalid-size record keeps safe defaults
- [ ] Invalid-checksum record keeps safe defaults

Build evidence:

```text
RAM: 14.3% (46736 / 327680 bytes)
Flash: 63.8% (835877 / 1310720 bytes)
Result: SUCCESS
```

### Save behavior

- [x] Pump change saves after the coalescing delay
- [x] COB change saves after the coalescing delay
- [ ] WS2812B enabled state and RGB color save through a production control path
- [x] Unchanged state skips the flash write
- [ ] Rapid toggles save only the final state
- [ ] A failed save remains pending for retry

### Restore behavior

- [x] Pump state restores after normal restart
- [x] COB state restores after normal restart
- [ ] WS2812B enabled state and RGB color restore through a production control path
- [x] Restored changes report source `restore`
- [x] Live GPIO32 state is read before pump restore
- [x] Low water rejects stored pump operation
- [x] COB remains restored during low water
- [x] The resulting safe pump OFF state replaces the unsafe stored request
- [x] A later normal-water reboot restores pump OFF rather than restarting it

Recorded serial evidence:

```text
Stored actual fountain state loaded and verified.
COB state: ON source=restore
NeoPixel state: OFF color=0,0,0 source=restore
Pump state: ON source=restore
Actual fountain state unchanged. Flash write skipped.
```

The user confirmed the low-water restore and final normal-water reboot tests passed.

### Integration safety

- [x] Local buttons remain responsive during persistence
- [x] Water protection remains authoritative during restore/persistence
- [ ] Setup portal responsiveness during a state write is explicitly re-tested
- [ ] Stored-network retry responsiveness during a state write is explicitly re-tested
- [ ] GPIO33 Wi-Fi reset is re-tested while confirming `state_blob` survives

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
