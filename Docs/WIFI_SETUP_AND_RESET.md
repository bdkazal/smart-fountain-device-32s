# Wi-Fi Setup and Reset

## Purpose

The Smart Fountain uses a local setup hotspot to collect router Wi-Fi credentials. Local buttons, output control, and water safety remain active during setup, connection attempts, disconnection, and reconnect.

## Setup hotspot

```text
SSID: Fountain-Setup
URL:  http://192.168.4.1
```

The hotspot uses a password configured in `include/WifiConfig.h`.

## First boot

If no stored Wi-Fi credentials exist:

```text
safe hardware startup
initialize local controls and water safety
mark provisioning required
start Fountain-Setup hotspot
start captive DNS redirect
serve setup page
```

The firmware does not silently use development-router credentials.

## Setup page behavior

The page loads before scanning begins.

```text
GET /
  return the page immediately

GET /networks
  start or read an asynchronous Wi-Fi scan

POST /save
  start a non-blocking credential test
  return immediately

GET /setup-status
  report idle, connecting, failed, or success
```

The browser polls while the device tests the submitted credentials. The ESP32 loop continues updating local controls and pump safety.

The setup page supports:

- scanned nearby networks
- manual SSID / hidden network entry
- password show/hide
- wrong-password retry without leaving the page
- successful save followed by restart

## Credential storage

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

Saving credentials verifies the stored SSID/password after writing. The provisioning flag is cleared only after the credential test succeeds and the values are saved correctly.

## Persistent provisioning rule

Provisioning mode uses a persistent flag, not a one-boot request.

```text
provision=true
  every boot starts the setup hotspot

valid credentials successfully saved
  provision=false
```

If the device restarts before setup is completed, it returns to the setup hotspot.

## Wi-Fi reset button

```text
GPIO33 -> momentary button -> GND
INPUT_PULLUP
released = HIGH
pressed  = LOW
```

Reset flow:

```text
1. Hold GPIO33 during boot.
2. Keep holding for 3 seconds.
3. Stored Wi-Fi SSID/password are cleared.
4. Provisioning is marked required.
5. The setup hotspot starts during the same boot.
```

Releasing before 3 seconds cancels the reset.

Wi-Fi reset does not clear future cached Laravel config, schedules, timezone, output state, or device identity.

## Normal stored-Wi-Fi boot

```text
stored credentials exist
provision=false
  start non-blocking station connection
```

The connection state machine uses:

```text
connecting
connected
waiting_to_retry
```

A failed connection does not automatically expose the setup hotspot. The device remains locally operational and retries the stored network periodically. The customer must deliberately hold the Wi-Fi reset button during boot to change networks.

## Local-first rule

The following must remain responsive in every network state:

- GPIO18 pump button
- GPIO19 COB button
- GPIO32 water-level input
- pump low-water safety
- output updates
- WS2812B runtime

## Test order

1. Upload firmware with no stored credentials.
2. Confirm `Fountain-Setup` appears.
3. Connect a phone and verify the captive page or open `http://192.168.4.1`.
4. Confirm the page loads before the network scan completes.
5. Test manual SSID entry.
6. Submit a wrong password and confirm the page remains usable.
7. Submit correct credentials and confirm save/restart.
8. Confirm the next boot connects using stored Wi-Fi.
9. Turn off the router and confirm local controls remain responsive while reconnect retries occur.
10. Hold GPIO33 during boot for 3 seconds and confirm provisioning mode returns.
