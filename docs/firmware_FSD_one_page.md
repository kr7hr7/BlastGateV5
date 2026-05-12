# One-Page FSD: BlastGateV5 Firmware

## Feature
ESP32-based blast-gate controller firmware for automated dust collection gate control, telemetry, and OTA maintenance.

## Objective
Provide reliable, low-latency gate behavior in the shop while keeping connectivity and diagnostics available:

- Open/close gate paths based on sensor/switch inputs
- Publish machine state to MQTT/Home Assistant topics
- Keep Wi-Fi/MQTT links alive in background tasks
- Preserve OTA accessibility during normal and fault states

## Modes (Operator View)

- A/B/C/D: Stepper gate control from analog trigger thresholds
- L: Alternate gate logic path using gateTypeL_Tasks() with immediate Switch A close and Switch B tap gestures
- X: Servo controller loop (A/B switch debounce + countdown logic)
- M: Manual gate support from analog input
- P: Passive tool usage reporting (no stepper travel)
- S: Static pressure reporting mode

## Boot Sequence (Condensed)

1. Load defaults from settings()
2. Initialize serial, I2C, and OLED
3. Connect Wi-Fi (primary, then alternate fallback)
4. Initialize EEPROM and apply board configuration table
5. Build MQTT topics and OTA hostname
6. Configure pins and output defaults
7. Connect MQTT and publish availability online
8. Start watchdog/heartbeat tasks:
   - keepWiFiAlive
   - keepMqttAlive
   - pingBroker
9. Enable OTA service
10. Validate gate type; invalid type enters error state

## Runtime Control (Condensed)

- Main loop always services OTA and MQTT loop.
- Dispatch then follows gateType branch.
- Stepper modes (A/B/C/D):
  - Open when sensorIn > trigger + triggerDelta (with confirm delay)
  - Close when sensorIn < trigger, using close countdown then homing
- Servo mode (X):
  - Switch A ON opens Servo A and locks out B countdown
  - Switch B supports pause/resume/extend countdown behavior and double-tap close
- Mode L:
  - Switch A ON opens/holds stepper; Switch A OFF triggers immediate homing close (no stepper countdown)
  - Switch B ON opens Servo B
  - Switch B single tap (release) adds gateDelaySeconds to close countdown
  - Switch B double tap (release) cancels countdown and forces Servo B OFF
  - Servo B is interlocked OFF while stepper is OPEN/OPENING, but allowed while stepper is CLOSING

## State and MQTT Summary

Primary state topic:

- home-assistant/blastgates/M<machineID>/state

Availability topic:

- home-assistant/blastgates/M<machineID>/availability

Additional telemetry:

- M<machineID>RSSI
- M<machineID>CycleData
- backPressure (static pressure mode)

Published gate states:

- unknown, closed, closing, open, opening, error_1, error_2, error_3

## Safety and Fault Handling

Error codes in active use:

- 1: missed-step/homing budget exceeded
- 3: invalid gate type configuration
- 4: board ID not found in board table
- 5: homing timeout

Fault behavior:

- Sets error gate state
- Publishes availability offline
- Displays error code on OLED (if available)
- Enters infinite fault loop while still servicing OTA

Recoverability:

- No automatic fault clear path
- Recovery is external (reboot and/or OTA fix)

## Operations Quick Checks

Startup checks:

1. OLED shows status text and setup identity
2. Wi-Fi connected, IP assigned
3. MQTT availability shows online
4. Expected gate mode is active for machine

If gate does not open/close:

1. Verify gateType and machineID mapping
2. Check limit switch behavior and home state transitions
3. Confirm trigger/triggerDelta values against sensor input
4. Check MQTT and serial logs for eCode transitions

If in persistent error screen:

1. Record displayed error code
2. Verify board ID and board table mapping
3. Verify wiring for limit switch and actuator pins
4. Apply OTA update or power-cycle after corrective action

## Configuration Surface (Most Used)

From settings():

- gateDelaySeconds
- closeSwitchDebounceMs
- bDoubleTriggerMs
- trigger / triggerDelta
- delayTime / maxMissedSteps
- MQTT broker and Wi-Fi credentials
- servo angles (openA/openB/closedA/closedB)

From board/machine tables:

- board ID to pin map
- machine ID to tool name and gate mode

## Notes

- Some machine table modes (for example W/Y/Z labels) are not implemented as loop dispatch branches.
- Mode L and mode X are both present and should be treated as distinct behavior paths unless unified by a future change.
- This one-page summary is an operations companion to the full spec in docs/firmware_FSD.md.
