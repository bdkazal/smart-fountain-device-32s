# Testing Checklist

## Stage 1 — Project foundation

- [x] `pio run` completes successfully
- [x] Firmware uploads successfully
- [x] Serial monitor opens at 115200 baud
- [x] Firmware prints board and version information
- [x] Main loop remains responsive

## Stage 2 — Safe hardware initialization

- [x] Pump control pin initializes LOW/OFF
- [x] COB control pin initializes LOW/OFF
- [x] WS2812B output initializes OFF
- [x] No unexpected output activation observed during validation

## Stage 3 — Inputs and local controls

### Water-level input

- [x] GPIO32 reads HIGH with switch open
- [x] GPIO32 reads LOW with switch closed to GND
- [x] Input remains stable during validation
- [x] Debounce prevents false rapid changes
- [x] Low water forces pump OFF
- [x] Low water blocks local pump ON
- [x] COB remains usable during low water

### Local buttons

- [x] GPIO18 pump button toggles pump output
- [x] GPIO19 COB button toggles COB output
- [x] One press produces one toggle
- [x] Local changes create state-change notifications

### Wi-Fi setup/reset button

- [x] GPIO33 reads HIGH when released
- [x] GPIO33 reads LOW when pressed
- [ ] Releasing before 3 seconds cancels Wi-Fi reset
- [ ] Holding during boot for 3 seconds clears Wi-Fi only
- [ ] Persistent provisioning mode starts after confirmed reset

## Stage 4 — NeoPixel validation

- [x] Short WS2812B strip powers from an external 5 V supply
- [x] ESP32 and LED supply grounds are common
- [x] Data is connected to DIN
- [x] Strip starts OFF
- [x] Red displays correctly
- [x] Green displays correctly
- [x] Blue displays correctly
- [x] White displays correctly
- [x] GRB color order confirmed
- [x] Validation runs without blocking safety/input monitoring

## Stage 5 — Wi-Fi onboarding

### First-boot setup

- [ ] Branch builds successfully
- [ ] Firmware uploads successfully
- [ ] No stored credentials starts `Fountain-Setup`
- [ ] Captive setup page opens automatically on phone
- [ ] Manual fallback opens at `http://192.168.4.1`
- [ ] Setup page loads before network scan completes
- [ ] Nearby network scan completes without dropping the hotspot
- [ ] Duplicate SSIDs are filtered
- [ ] Manual/hidden SSID works
- [ ] Password show/hide works

### Credential test and save

- [ ] `POST /save` returns without freezing local controls
- [ ] Pump and COB buttons remain responsive during credential test
- [ ] Water safety remains responsive during credential test
- [ ] Wrong password keeps the setup page active
- [ ] Correct password connects successfully
- [ ] Credentials are saved and verified in Preferences
- [ ] Provisioning-required flag clears only after successful save
- [ ] Device restarts after successful save

### Stored Wi-Fi and reconnect

- [ ] Next boot uses stored credentials
- [ ] Station connection is non-blocking
- [ ] Router outage does not start the setup hotspot automatically
- [ ] Device retries stored Wi-Fi periodically
- [ ] Local buttons remain responsive while disconnected
- [ ] Pump safety remains active while disconnected
- [ ] Device reconnects after router recovery

### Reset and reprovisioning

- [ ] GPIO33 boot hold clears SSID/password only
- [ ] Future cached Laravel config would remain untouched
- [ ] Provisioning-required flag survives restart
- [ ] Device returns directly to `Fountain-Setup`
- [ ] New credentials can be saved successfully

## Stage 6 — Laravel HTTP

- [ ] Device authentication works with `X-DEVICE-KEY`
- [ ] Config fetch succeeds
- [ ] Actual output state sync succeeds
- [ ] Local button changes update Laravel state
- [ ] Command polling succeeds
- [ ] Commands are acknowledged and completed correctly
- [ ] Laravel pump ON is blocked during low water
- [ ] Schedule actions use the shared `FountainController`
- [ ] Compact config cache avoids unnecessary flash writes

## Stage 7 — Offline behavior

- [ ] Local buttons work without Wi-Fi
- [ ] Water safety works without Wi-Fi
- [ ] Last trusted persistent state restores safely
- [ ] Cached timeline follows approved product rules
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
