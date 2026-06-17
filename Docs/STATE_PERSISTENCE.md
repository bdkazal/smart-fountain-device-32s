# Actual-State Persistence

## Purpose

The Smart Fountain is a persistent-state device. After a normal restart or power cycle, it should restore the last actual applied output state while preserving safe startup and local water protection.

This is separate from the future compact Laravel configuration cache:

```text
actual-state record
  remembers what the hardware was doing

compact configuration record
  will store schedules, timezone, scenes, and configuration revision
```

Both records belong to the existing `fountain` Preferences namespace. They are separate keys, not separate storage systems.

## Current stored state

Firmware version:

```text
smart-fountain-32s-state-0.2-persistence
```

Preferences key:

```text
state_blob
```

The version-1 record stores:

```text
pump ON/OFF
COB ON/OFF
NeoPixels ON/OFF
NeoPixel red value
NeoPixel green value
NeoPixel blue value
record version
checksum
```

Pump and COB remain ON/OFF only. The abandoned ESP32-C3 prototype's pump PWM, COB PWM, and analog RGB implementation are not used by this board.

## Storage safety

The state record is intentionally small. It uses:

- a version byte for future migrations
- flags for the three enabled states
- three NeoPixel color bytes
- reserved bytes for compatible expansion
- CRC32 validation

On load, an unexpected size, unsupported version, incomplete read, or checksum failure leaves all outputs at their safe OFF defaults.

Before writing, firmware compares the new record with the existing record. Unchanged state skips the flash write.

## Save behavior

Every actual state-change notification queues both:

```text
local state persistence
future Laravel actual-state synchronization
```

The current firmware waits 300 ms before writing. Rapid changes within that window are coalesced so only the final state is stored.

The physical output changes immediately. The delay applies only to flash persistence.

If a storage write fails, the final state remains pending and firmware retries without blocking local controls.

## Boot restore order

```text
1. Initialize pump, COB, and NeoPixels physically OFF.
2. Initialize GPIO32 and read the real water state.
3. Initialize FountainController.
4. Initialize DeviceStorage.
5. Load and validate state_blob.
6. Restore COB through FountainController.
7. Restore NeoPixel color and enabled state through FountainController.
8. Request pump restore through FountainController.
9. Water safety may reject pump ON.
10. Start Wi-Fi setup or station runtime.
```

Pump restoration is intentionally last. Water state is never restored from flash; it always comes from the live GPIO32 input.

## Control-path rule

Every output source must use the shared controller:

```text
local button
restore
Laravel command
schedule
safety
  -> FountainController
  -> HardwareOutputs
```

`ControlSource::Restore` identifies state applied from local persistence.

If stored pump state is ON while water is low, the controller keeps the physical pump OFF, marks the result as a safety state, and queues the safe OFF result for persistence and future Laravel synchronization.

## Wi-Fi reset behavior

GPIO33 Wi-Fi reset removes only:

```text
wifi_ssid
wifi_pass
```

It does not remove:

```text
state_blob
future compact Laravel config
future schedule/timezone cache
device identity
```

## Laravel reconciliation direction

After HTTP integration is added:

```text
boot restore actual state
-> connect to Laravel
-> report actual restored state
-> fetch current configuration and pending commands
-> apply newer valid actions through FountainController
-> report final actual state
```

A durable `pendingSync` flag is not required for the first implementation. Every boot and Laravel reconnection should send the current actual hardware state.

## Validation status

Implementation is committed on:

```text
feature/state-persistence
```

The code has not yet been built, uploaded, or power-cycle tested on hardware. Do not mark this milestone validated until the checklist in `Docs/TESTING_CHECKLIST.md` passes.
