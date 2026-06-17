# Wi-Fi Setup and Reset

## Purpose

The Smart Fountain uses a local setup hotspot to collect router credentials. Local buttons, outputs, water protection, and actual-state persistence remain active during setup, connection attempts, disconnection, and reconnect.

## Setup hotspot

```text
SSID: Fountain-Setup
URL:  http://192.168.4.1
```

The hotspot password is configured in `include/WifiConfig.h`.

## First boot

Without stored Wi-Fi credentials:

```text
safe hardware startup
initialize water input and local controls
restore valid actual output state if available
mark provisioning required
start Fountain-Setup and captive DNS
serve setup page
```

The firmware never silently uses development-router credentials.

## Setup page behavior

```text
GET /
  return page immediately

GET /networks
  start/read asynchronous scan

POST /save
  start non-blocking credential test
  return immediately

GET /setup-status
  report idle, connecting, failed, or success
```

The browser polls while the ESP32 continues its local runtime. The page supports scanned networks, manual SSID entry, password show/hide, wrong-password retry, and save/restart after success.

## Preferences storage

Namespace:

```text
fountain
```

Wi-Fi keys:

```text
wifi_ssid
wifi_pass
provision
```

Actual-state key introduced later:

```text
state_blob
```

The future compact Laravel configuration will use its own key in the same namespace.

Credential writes are verified. `provision` becomes false only after successful testing and storage.

## Persistent provisioning

```text
provision=true
  every boot starts Fountain-Setup

valid credentials saved
  provision=false
```

An interrupted setup returns to the portal on the next boot.

## GPIO33 reset

```text
GPIO33 -> momentary button -> GND
INPUT_PULLUP
released = HIGH
pressed  = LOW
```

Confirmed reset flow:

```text
hold during boot for 3 seconds
clear wifi_ssid and wifi_pass only
set provisioning required
start Fountain-Setup
```

Releasing before three seconds cancels reset.

Wi-Fi reset preserves:

```text
state_blob
future schedules/timezone/scenes cache
device identity
```

## Stored-network behavior

```text
stored credentials + provision=false
  use non-blocking station connection
```

States:

```text
connecting
connected
waiting_to_retry
```

An ordinary router outage does not deliberately expose the portal. The device remains locally operational and retries stored Wi-Fi. Changing networks requires the deliberate GPIO33 action.

## Local-first rule

These continue in every network state:

- GPIO18 pump button
- GPIO19 COB button
- GPIO32 water input
- water protection
- output updates
- WS2812B runtime
- queued actual-state persistence

## Accepted validation baseline

Firmware used for onboarding validation:

```text
smart-fountain-32s-wifi-0.1-onboarding
```

Recorded results:

```text
SSID: Andromeda
wrong password: connection failed, portal stayed active
correct retry: credentials saved and verified
automatic restart: passed
stored-network boot: passed
connected IP: 192.168.0.102
```

Controls were also observed working in setup mode, during asynchronous scanning, and after connection. The credential-connecting interval can be too short for a repeatable manual button press, so that timing-specific press is not treated as a separate requirement.

The user confirmed the Wi-Fi reset, outage/retry, reconnection, and reprovisioning baseline complete. Further Wi-Fi work should happen only when a regression appears or product requirements change.

## Regression test order

1. Boot with no credentials and confirm `Fountain-Setup`.
2. Open captive/manual setup page.
3. Submit an invalid password and confirm retry remains possible.
4. Submit valid credentials and confirm save/restart.
5. Confirm stored-network boot.
6. Remove router availability and confirm local runtime plus retry.
7. Restore router and confirm reconnect.
8. Test short and confirmed GPIO33 holds.
9. Confirm Wi-Fi reset preserves `state_blob`.
