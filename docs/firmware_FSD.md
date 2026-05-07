# BlastGateV5 Firmware Functional Specification Document (FSD)

## 1. Document Control

- Project: BlastGateV5
- Scope: Full firmware behavior implemented in current codebase
- Firmware baseline: as represented by source files under src and include
- Audience: Firmware engineers, controls engineers, operations/maintenance engineers
- Style: Engineering design specification (implementation-traceable)

## 2. System Purpose

BlastGateV5 controls dust-collection blast gates and related signals using an ESP32-based controller. The firmware supports multiple operating modes, including stepper-controlled gates, servo-controlled gates, static pressure reporting, manual gate support, and passive usage logging.

At runtime, the firmware:

- Samples local electrical/mechanical inputs (analog trigger, limit switch, switch A/B)
- Determines gate actions based on configured gate mode
- Commands actuators (stepper, relays, servos)
- Publishes state and telemetry through MQTT
- Maintains Wi-Fi/MQTT connectivity with background tasks
- Keeps OTA update handling available during normal operation and fault loops

## 3. In-Scope and Out-of-Scope

### 3.1 In-Scope

- Boot/initialization behavior
- Runtime behavior for gate modes A, B, C, D, L, M, P, S, X
- State transitions and published gate state semantics
- Debounce/countdown/timing behavior
- EEPROM persistence model used by board configuration
- MQTT/Wi-Fi/OTA behavior implemented in firmware
- Error/failsafe behavior and recoverability limits

### 3.2 Out-of-Scope

- PlatformIO packaging/build pipeline specifics
- External dashboards and Home Assistant configuration details
- Mechanical design details of gates/actuators
- External service ownership and SLAs (for Google Script endpoints)

## 4. System Architecture

## 4.1 Functional Modules

- Initialization orchestration: src/setupTasks.cpp
- Main runtime dispatch loop: src/main.cpp
- Stepper gate control/homing and manual gate actuator helpers: src/controlFunctions.cpp
- Servo controller and alternate task modes: src/ControllerTasks.cpp
- Messaging, OTA, display, switch-state processing, and error loop: src/utilities.cpp
- Settings defaults: src/settings.cpp
- Board and machine/gate lookup tables: src/tables.cpp
- Board config + EEPROM interaction: src/boardConfig.cpp
- Shared global state and enums: include/globals.h, src/globals.cpp

## 4.2 Concurrency Model

- Foreground loop: loop() in src/main.cpp
- Background FreeRTOS tasks created during setup:
  - pingBroker
  - keepWiFiAlive
  - keepMqttAlive

The foreground and background paths cooperate through global variables and periodic yields/delays.

## 5. Operating Modes (Gate Type)

The effective gate mode is derived from machine ID lookup (tools() in src/tables.cpp), then used in runtime dispatch.

Implemented runtime branches:

- A/B/C/D: Stepper gate control path in main loop using openGate()/closeGate()
- L: gateTypeL_Tasks() path in src/ControllerTasks.cpp
- X: servoControllerLoop() path in src/main.cpp
- M: manualGateTasks() path
- P: passive tool activity path using toolOn()/toolOff() without stepper travel
- S: static pressure reporting path

Note: Machine table includes additional labels (for example W, Y, Z), but only the branches above are implemented in runtime dispatch.

## 6. Startup and Initialization Behavior

setup() performs:

1. setupTasks()
2. servoControllerSetup()

setupTasks() sequence (summarized):

1. Load defaults via settings()
2. Initialize serial, I2C, and SSD1306 OLED (0x3C fallback 0x3D)
3. Connect Wi-Fi (primary + alternate fallback path)
4. Initialize EEPROM; run boardconfiguration(); apply table-based pin configuration
5. Build MQTT topics and OTA hostname (prepNames())
6. Configure pin modes and initial output states
7. Configure MQTT client and connect
8. Create background tasks (pingBroker, keepWiFiAlive, keepMqttAlive)
9. Emit boot diagnostics, write boot HTTP log, initialize OTA
10. Validate gateType and enter errorState() if unsupported
11. Publish availability online

servoControllerSetup() then initializes switch A/B baseline states and servo outputs, and force-publishes initial servo-derived gate state.

## 7. Main Loop Dispatch Behavior

In each loop iteration:

1. ArduinoOTA.handle()
2. client.loop()
3. printInputStates()
4. updateSensorInOnOled()
5. Gate-type-specific behavior

### 7.1 Mode S (Static Pressure)

- Calls S_StaticPressureTasks()
- Samples analog pressure input and publishes backPressure when change exceeds staticDelta or max interval is reached

### 7.2 Mode M (Manual)

- Calls manualGateTasks()
- Uses analog level logic to call manualGateOpen() / manualGateClose()
- Updates state and UI accordingly

### 7.3 Mode P (Passive)

- No stepper travel
- Uses analog trigger thresholds to call toolOn() and toolOff()
- Publishes open/closed gate state semantics for tool activity

### 7.4 Mode X (Servo Controller Loop)

- Calls checkSwitchState() then servoControllerLoop()
- Implements dual-switch debounce + servo A/B logic + B-side countdown behavior

### 7.5 Modes A/B/C/D (Stepper)

- checkSwitchState()
- Read analog trigger
- If above trigger + triggerDelta (with confirm delay), open gate path
- If below trigger, close gate path with delayed homing

### 7.6 Mode L

- Calls gateTypeL_Tasks() in src/ControllerTasks.cpp
- This is a distinct task path from servoControllerLoop()

## 8. Stepper Control Functional Behavior (A/B/C/D Core)

## 8.1 Homing (homePosition)

Behavior:

- Verifies raw home switch state first (LOW means HOME)
- If already HOME with short confirm delay, marks CLOSED and exits
- Else runs homing step loop with:
  - bounded step budget derived from fullRunSteps + maxMissedSteps
  - timeout bound derived from configured step pulse timing
  - periodic OTA/yield servicing
- On successful HOME detection:
  - sets STATE_CLOSED
  - updates gateOpenState/gateCloseState/limitSwitchState
  - updates display trace and LEDs

Fault triggers in homing:

- eCode 5: homing timeout
- eCode 1: exceeded homing step budget

Both faults transfer to errorState().

## 8.2 Open Path (openGate)

Behavior:

- Requires CLOSED/HOME confirmation; if not confirmed, runs homing first
- If homing still fails to establish closed state, aborts open and sets STATE_UNKNOWN
- Drives relay/output and stepper direction for open travel
- Steps fullRunSteps pulses with periodic OTA/yield
- On completion sets gate state OPEN and marks gateOpenState true

## 8.3 Close Path (closeGate)

Behavior:

- Enters CLOSING state
- Starts/updates countdown using gateDelaySeconds
- Displays countdown on OLED (if available)
- When countdown expires, invokes homePosition()
- On successful close: trace CLOSED, state update, logCycle()

## 9. Servo Controller Functional Behavior

The primary documented servo state machine is implemented in servoControllerLoop(). It aligns to the flow represented in docs/servoControllerLoop_flowchart.mmd.

## 9.1 Inputs and Debounce

- Switches use INPUT_PULLUP polarity: LOW = ON, HIGH = OFF
- Debounce windows are maintained independently for A and B
- Stable transitions drive control decisions

## 9.2 Servo A Semantics

- A ON:
  - setGateState(STATE_OPENING)
  - commandServoA(true)
  - force Servo B off
  - clear B countdown and resume flags
- A OFF:
  - commandServoA(false)
  - B logic becomes active

## 9.3 Servo B Semantics

When A is OFF and B transitions:

- B ON:
  - if countdown active, capture pausedRemainingB and set resumeCountdownOnBHigh
  - stop active countdown
  - command servo B based on current A state

- B OFF:
  - calculate possible doubleTap window using bDoubleTriggerMs
  - if doubleTap, force close B and clear countdown state
  - else if resume flag set, start countdown with pausedRemainingB + gateDelaySeconds (capped)
  - else:
    - if countdown already active, extend remaining time by gateDelaySeconds (capped)
    - otherwise start a new countdown for gateDelaySeconds

## 9.4 Countdown Service

- runCountdown() updates OLED countdown display when active
- On countdown expiry:
  - commandServoB(false)
  - clear countdown/resume state
  - update trace

## 9.5 Servo-Derived Published Gate State

publishServoGateState() derives gate state as:

- ERROR_1/2/3 when eCode indicates fault
- UNKNOWN if servos not attached
- CLOSING when B countdown active
- OPEN when either servo is ON
- CLOSED otherwise

## 10. State Model and Publishing

Primary state enum (include/globals.h):

- unknown
- closed
- closing
- open
- opening
- error_1
- error_2
- error_3

State update path:

- setGateState(newState, forcePublish)
- publishGateState(force)

Publishing behavior:

- Gate state publishes to BGtopic
- State publishes on change, or force publish during selected events

## 11. Interfaces

## 11.1 Hardware Interfaces

- Stepper: stepPin, dirPin, enablePin
- Home/limit switch: limitSwitchPin
- Relay/output: reedRelayPin, gateOn, gateOff
- Servo control: servoPinA, servoPinB
- Servo mode input switches: switchPinA, switchPinB
- Analog input: ANALOG_PIN_IN (trigger/static pressure)
- OLED over I2C

Pin assignments are selected by board ID lookup table in src/tables.cpp via applyBoardConfiguration().

## 11.2 MQTT Interfaces

Topic patterns prepared in prepNames():

- home-assistant/blastgates/M<machineID>/state
- home-assistant/blastgates/M<machineID>/availability
- M<machineID>RSSI
- M<machineID>CycleData
- backPressure (static pressure mode)

Availability behavior:

- publish online on successful connect and periodic heartbeat
- publish offline in errorState() and MQTT will settings during disconnect events

## 11.3 OTA Interface

- OTA initialized in OTA() once per boot lifecycle (guarded by otaOn)
- OTA handle serviced in loop and multiple long-running control paths
- Hostname built in prepNames() from setup/tool identity and sanitized to mDNS-safe characters

## 11.4 HTTP Boot Logging

- writeToBootLog() issues an HTTP GET to configured Google Script endpoint when Wi-Fi is connected
- Intended as boot telemetry; does not gate core control loop startup beyond request timeout behavior

## 12. Persistence and Configuration

## 12.1 EEPROM Model

- EEPROM marker at byte 3 value 200 indicates initialized board data
- Byte 0 stores board ID
- boardconfiguration() rewrites EEPROM when requested or when marker/ID mismatch is detected

## 12.2 Settings Defaults

settings() provides defaults including:

- boardID, machineID
- gateDelaySeconds
- closeSwitchDebounceMs
- bDoubleTriggerMs
- servo open/closed angles
- MQTT broker addresses
- Wi-Fi credentials
- trigger and triggerDelta
- maxMissedSteps
- delayTime
- boot log endpoint

## 12.3 Board/Mode Resolution

- tools() maps machineID to toolString and gateType
- gateTypeConfig() adjusts motion parameters by gateType
- applyBoardConfiguration() sets pin assignments and board metadata from boardTable

## 13. Safety, Faults, and Recoverability

## 13.1 Fault Codes in Active Use

- Code 1: excessive missed steps during homing budget
- Code 2: reserved/mismatch class (supported in state mapping)
- Code 3: unsupported gateType configuration
- Code 4: unknown board ID table lookup failure
- Code 5: homing timeout

## 13.2 Fault Handling Behavior

errorState() behavior:

- Sets gate state to mapped error state (or unknown)
- Disables stepper drive enable
- Publishes MQTT availability offline
- Displays error code on OLED when available
- Enters infinite loop with periodic OTA handling

Recoverability implication:

- No automatic self-clear from error loop in current implementation
- Recovery requires external intervention (for example reboot or OTA update)

## 13.3 Connectivity Resilience

- keepWiFiAlive task attempts reconnect when disconnected
- keepMqttAlive task services loop and reconnect attempts
- pingBroker task publishes periodic availability and telemetry

## 14. Timing and Debounce Characteristics

- Home switch debounce floor in checkSwitchState(): minimum 5 ms
- closeSwitchDebounceMs default: 250 ms
- Servo A/B debounce delay in servoControllerLoop(): 50 ms
- Passive/stepper trigger confirmation delay: 200 ms re-sample
- Servo B double-trigger window default: 750 ms
- Gate close countdown base: gateDelaySeconds

## 15. Assumptions and TBD Items

The following are intentionally documented as unresolved or implementation-dependent:

1. Gate types W/Y/Z appear in machine table values but have no explicit runtime dispatch branches.
2. Relationship between gateType L path and gateType X servoControllerLoop path should be finalized as product intent.
3. Many board table rows use repeated placeholder-like pin values (for example 5); electrical intent for each board revision should be verified against hardware BOM/schematics.
4. eCode 2 handling exists but triggering path is not explicit in currently reviewed control flow.
5. Legacy docs mention older setup/loop patterns; current code uses setupTasks() plus active loop dispatch.

## 16. Verification Checklist (for Future Changes)

For any firmware change, verify:

1. Every gateType branch in loop() has corresponding FSD section updates.
2. Any change to GateState transitions updates both behavior text and MQTT topic semantics.
3. Debounce/timing constant changes are reflected in Section 14.
4. Board table or tools() changes are reflected in persistence/configuration sections.
5. Error path changes preserve stated recoverability behavior or document new recovery paths.

## 17. Source Traceability Map

- Main runtime dispatch: src/main.cpp
- Boot orchestration: src/setupTasks.cpp
- Stepper and actuator actions: src/controlFunctions.cpp
- Servo loop and alternate modes: src/ControllerTasks.cpp
- State publish/connectivity/error/OTA/display: src/utilities.cpp
- Settings defaults: src/settings.cpp
- EEPROM/board config: src/boardConfig.cpp
- Board + machine lookup tables: src/tables.cpp
- Shared global state model: include/globals.h, src/globals.cpp

## 18. Related Internal Documents

- MAIN_DOC.md
- GLOBALS_DOC.md
- docs/closeGate_homePosition_reference.md
- docs/gateTypeL_Tasks_reference.md
- docs/servoControllerLoop_flowchart.md
- docs/servoControllerLoop_flowchart.mmd
