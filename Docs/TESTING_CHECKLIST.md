# Testing Checklist

## Stage 1 — Project foundation

- [ ] `pio run` completes successfully
- [ ] Firmware uploads successfully
- [ ] Serial monitor opens at 115200 baud
- [ ] Firmware prints board and version information
- [ ] Main loop remains responsive without long blocking delays

## Stage 2 — Safe hardware initialization

- [ ] Pump control pin initializes LOW/OFF
- [ ] COB control pin initializes LOW/OFF
- [ ] WS2812B output initializes OFF
- [ ] No output briefly turns on during boot

## Stage 3 — Input validation

### Water-level input

- [ ] GPIO32 reads HIGH with switch open
- [ ] GPIO32 reads LOW with switch closed to GND
- [ ] Input remains stable for at least 10 minutes with switch open
- [ ] Debounce prevents rapid false state changes

### Wi-Fi setup/reset button

- [ ] GPIO33 reads HIGH when released
- [ ] GPIO33 reads LOW when pressed
- [ ] A short press does not trigger a reset action
- [ ] Planned boot-time hold can be detected reliably

## Stage 4 — NeoPixel validation

- [ ] A short WS2812B strip powers correctly from its external 5 V supply
- [ ] ESP32 and LED supply grounds are common
- [ ] Data is connected to the strip DIN side
- [ ] Strip starts OFF
- [ ] Red displays correctly
- [ ] Green displays correctly
- [ ] Blue displays correctly
- [ ] Low-brightness white displays correctly
- [ ] Brightness limit is respected
- [ ] Effects run without blocking the main loop

## Stage 5 — Wi-Fi setup

- [ ] Stored Wi-Fi credentials are read correctly
- [ ] Setup/reset hold clears only intended Wi-Fi data
- [ ] Setup hotspot appears reliably
- [ ] Phone connects on the first attempt
- [ ] Setup page opens at `http://192.168.4.1`
- [ ] Nearby network scan completes without dropping the hotspot
- [ ] Wrong router password leaves setup mode active
- [ ] Correct credentials are saved
- [ ] Device restarts and reconnects using saved credentials

## Stage 6 — Laravel HTTP

- [ ] Device authentication works with `X-DEVICE-KEY`
- [ ] Config fetch succeeds
- [ ] Actual output state sync succeeds
- [ ] Command polling succeeds
- [ ] Commands are acknowledged and completed correctly
- [ ] Low-water safety overrides pump-on requests
- [ ] Compact config cache avoids unnecessary flash writes

## Stage 7 — Offline behavior

- [ ] Last trusted persistent state restores safely
- [ ] Local water safety remains active without Wi-Fi
- [ ] Cached timeline behavior follows the approved product rules
- [ ] Actual locally applied state syncs after Laravel recovers

## Stage 8 — MQTT

- [ ] MQTT connects without blocking the main loop
- [ ] Device subscribes only to its own command topic
- [ ] Presence is published correctly
- [ ] Command acknowledged/executed/failed events are published
- [ ] Actual state is published after changes
- [ ] HTTP fallback remains functional

## Test evidence rule

For every completed stage, record:

- firmware commit SHA
- board used
- wiring/pin map used
- serial log summary
- pass/fail result
- known limitations
