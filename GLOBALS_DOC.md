# Globals Reference

This project centralizes shared state in:

- `include/globals.h` for declarations
- `src/globals.cpp` for definitions and default values

Most of these globals are runtime state for board configuration, MQTT/Wi-Fi, gate control, sensor processing, and OLED status display.

## Design Notes

- Many values start with a safe default in `src/globals.cpp` and are later replaced by `settings()`, `boardconfiguration()`, `prepNames()`, or runtime logic.
- Hardware pin globals are defaults, not hard guarantees. Board configuration can override them.
- Several string globals are used for MQTT topics, OLED text, and diagnostics.
- `display`, `client`, and `espClient` are shared service objects used across multiple translation units.

## Core Types

### `GateState`

Represents the logical gate state used for MQTT publishing and controller behavior.

Values:

- `STATE_UNKNOWN`: startup, disconnected, or indeterminate state
- `STATE_CLOSED`: gate confirmed closed
- `STATE_CLOSING`: close command in progress
- `STATE_OPEN`: gate confirmed open
- `STATE_OPENING`: open command in progress
- `STATE_ERROR_1`, `STATE_ERROR_2`, `STATE_ERROR_3`: fault states used by control logic

## Hardware and Pin Globals

These define GPIO assignments and EEPROM board metadata.

| Global | Type | Default | Purpose |
| --- | --- | --- | --- |
| `ANALOG_PIN_IN` | `int` | `32` | Main analog sensor input |
| `flash` | `byte` | `0` | EEPROM flag indicating board data stored |
| `boardIdByte` | `byte` | `0` | EEPROM board ID |
| `boardConfig` | `byte` | `0` | EEPROM board configuration/mode |
| `boardVer` | `byte` | `0` | Board revision/version byte |
| `analogPinInByte` | `byte` | `0` | Stored analog input pin byte |
| `limitSwitchPin` | `byte` | `19` | Limit switch input |
| `reedRelayPin` | `byte` | `23` | Reed relay input/output depending on board wiring |
| `greenLEDpin` | `byte` | `25` | Green status LED |
| `redLEDpin` | `byte` | `26` | Red status LED |
| `gateOn` | `byte` | `16` | Motor/H-bridge on/open control |
| `gateOff` | `byte` | `27` | Motor/H-bridge off/close control |
| `linkPin` | `byte` | `13` | Board-specific link/configuration pin |
| `stepPin` | `byte` | `16` | Stepper pulse pin |
| `dirPin` | `byte` | `4` | Stepper direction pin |
| `enablePin` | `byte` | `17` | Stepper enable pin |
| `boardTypeName` | `const char*` | `""` | Human-readable board description from lookup/config table |

### Board Config Meaning

Comments in `globals.h` define these `boardConfig` meanings:

- `1`: Mac style stepper
- `2`: Manual gate
- `3`: CNC and cleanup stepper
- `4`: Pressure sensor monitor
- `5`: No gate control, data logging only

## MQTT and Identity Globals

These are used to form MQTT topics, device identity strings, and online/offline reporting.

| Global | Type | Default | Purpose |
| --- | --- | --- | --- |
| `toolInt` | `int` | `0` | Numeric tool state/helper value |
| `checkInID` | `char[30]` | `""` | Device check-in message buffer |
| `mIDlen` | `int` | `0` | Length of machine ID |
| `machineIDChar` | `char[3]` | `""` | Short machine ID buffer |
| `mID` | `char[4]` | `""` | Three-character machine ID such as `M21` |
| `mIDstring` | `String` | empty | `mID` as Arduino `String` |
| `gateName` | `char[51]` | `""` | MQTT/display gate name buffer |
| `BGstatus` | `char*` | `nullptr` | MQTT status payload pointer |
| `gateNameString` | `String` | empty | Gate name as `String` |
| `gateTypeString` | `String` | empty | Gate type text representation |
| `keepAlive` | `int` | `70` | MQTT keepalive in seconds |
| `availabilityTopic` | `char[50]` | `""` | MQTT availability topic |
| `BGtopic` | `char[40]` | `""` | Main BlastGate MQTT topic |
| `rssiTopic` | `String` | empty | Wi-Fi RSSI publish topic |
| `machineIDString` | `String` | empty | Machine ID as `String` |
| `setupID` | `String` | empty | Setup/identity label shown on OLED |
| `gateState` | `GateState` | `STATE_UNKNOWN` | Logical gate state |
| `rotation` | `bool` | `false` | Mount orientation flag |
| `urlFinal` | `String` | empty | Final URL used for HTTP logging/upload |
| `eepromUpdate` | `bool` | `false` | Flag requesting EEPROM write/update |
| `cycleTopic` | `char[15]` | `""` | MQTT cycle-count topic |
| `ver` | `const char*` | `"03_13_2026"` | Firmware version string |

## Working State Globals

These support live sensor handling, UI text, motion state, and diagnostics.

| Global | Type | Default | Purpose |
| --- | --- | --- | --- |
| `sensorIn` | `int` | `0` | Current raw analog reading from the input sensor |
| `sensor` | `int` | `0` | Processed or latched sensor value used by gate logic |
| `stepPosition` | `int` | `0` | Current stepper position counter |
| `trace` | `String` | empty | Main OLED status text |
| `countDown` | `int` | `0` | Countdown seconds for close-delay UI/logic |
| `toolString` | `String` | empty | Tool description shown on OLED |
| `ipa` | `String` | empty | Local IP address string |
| `mac` | `String` | empty | Device MAC address string |
| `wifiDB` | `int` | `0` | Wi-Fi RSSI in dBm |
| `charWifiDB` | `char[10]` | `""` | RSSI text buffer |
| `verString` | `String` | empty | Version string as `String` |
| `machineIDstring` | `String` | empty | Alternate machine ID string variable |
| `fullRunSteps` | `int` | `0` | Travel distance or full-run step count |
| `eCode` | `byte` | `0` | Current error code |
| `db` | `char*` | `nullptr` | Debug/location string pointer |
| `dbNew` | `char*` | `nullptr` | New debug/location string pointer |

## Timing and Parameter Globals

These are mostly fixed tuning values plus shared timestamp variables.

### Fixed Parameters

| Global | Type | Default | Purpose |
| --- | --- | --- | --- |
| `waitTime` | `const int` | `250` | General wait/delay interval |
| `backSteps` | `const int` | `50` | Steps needed to move off the home switch |
| `stepsPerRevolution` | `const int` | `200` | Stepper motor steps per revolution |
| `holdTime` | `const int` | `500` | Settle/hold delay per cycle |
| `displayTime` | `const int` | `0` | Display timing parameter |
| `timeout` | `const int` | `120` | Timeout value used by control logic |

### Shared Timekeeping

| Global | Type | Default | Purpose |
| --- | --- | --- | --- |
| `closeDelayTime` | `unsigned long` | `0` | Delay before closing, in ms |
| `currentTime` | `unsigned long` | `0` | Shared current timestamp |
| `gateOpenTime` | `unsigned long` | `0` | Time gate opened |
| `startTime` | `unsigned long` | `0` | Start time of active cycle |
| `closeTime` | `unsigned long` | `0` | Time gate closed |
| `onTime` | `unsigned long` | `0` | Tool-on timestamp |
| `offTime` | `unsigned long` | `0` | Tool-off timestamp |
| `x` | `unsigned long` | `0` | Scratch timing/calculation value |
| `runTime` | `unsigned` | `0` | Run duration accumulator |
| `currentMillis` | `unsigned long` | `0` | Shared loop timestamp |
| `lastMsgTime` | `unsigned long` | `0` | Last publish timestamp |
| `interval` | `unsigned int` | `0` | General polling/publish interval |
| `maxInterval` | `unsigned int` | `0` | Maximum publish interval |
| `staticDelta` | `unsigned int` | `0` | Minimum pressure delta to report |

## Pressure Sensor and Reporting Globals

These are primarily used by static pressure mode (`gateType == "S"`).

| Global | Type | Default | Purpose |
| --- | --- | --- | --- |
| `lastSample` | `int` | `0` | Last analog sample used for delta comparison |
| `staticPressure` | `float` | `0.0` | Current pressure sensor reading |
| `inchesH2O` | `float` | `0.0` | Pressure converted to inches of water |
| `inchesH2O_str` | `String` | empty | Printable pressure string |
| `buffer` | `char[10]` | `""` | Numeric publish formatting buffer |
| `delta` | `int` | `0` | Current difference from last sample |

## Status and Control Flags

These values express input state, gate position assumptions, and active modes.

| Global | Type | Default | Purpose |
| --- | --- | --- | --- |
| `manualState` | `volatile bool` | `false` | Manual mode active flag |
| `limitSwitchState` | `volatile bool` | `false` | Current limit switch reading |
| `gateCloseState` | `volatile bool` | `false` | Gate physically closed indication |
| `gateOpenState` | `volatile bool` | `false` | Gate physically open indication |
| `moveState` | `volatile bool` | `false` | Gate/motor currently moving |
| `otaOn` | `volatile bool` | `false` | OTA mode active flag |
| `manualStatus` | `int` | `0` | Manual state/detail code |
| `flag` | `int` | `0` | General-purpose status flag |

## Usage Counters and Machine Metadata

| Global | Type | Default | Purpose |
| --- | --- | --- | --- |
| `toolRunTime` | `float` | `0.0` | Tool runtime statistic |
| `toolTimer` | `long` | `0` | Tool timing accumulator |
| `toolCycles` | `int` | `0` | Cycle counter |
| `loopCount` | `int` | `0` | General loop counter |
| `boardID` | `String` | empty | Human-readable board ID |
| `intboardID` | `int` | `0` | Numeric board ID |
| `machineID` | `String` | empty | Human-readable machine ID |
| `gateDelaySeconds` | `int` | `0` | Configured auto-close delay |
| `gateType` | `String` | empty | Operating mode or gate family |

## Network Credentials and Endpoints

These are normally assigned by configuration code, not hard-coded in `globals.cpp`.

| Global | Type | Default | Purpose |
| --- | --- | --- | --- |
| `mqtt_server` | `const char*` | `nullptr` | Primary MQTT broker host |
| `mqtt_serverAlt` | `const char*` | `nullptr` | Secondary MQTT broker host |
| `ssid` | `const char*` | `nullptr` | Primary Wi-Fi SSID |
| `password` | `const char*` | `nullptr` | Primary Wi-Fi password |
| `ssidAlt` | `const char*` | `nullptr` | Secondary Wi-Fi SSID |
| `passwordAlt` | `const char*` | `nullptr` | Secondary Wi-Fi password |

## Trigger and Motion Tuning Globals

| Global | Type | Default | Purpose |
| --- | --- | --- | --- |
| `trigger` | `int` | `0` | Sensor threshold for tool/gate activation |
| `triggerDelta` | `int` | `0` | Hysteresis margin above `trigger` |
| `maxMissedSteps` | `int` | `0` | Allowed missed steps before fault logic |
| `delayTime` | `int` | `0` | Generic motion delay tuning value |

## HTTP/Google Script Logging Globals

| Global | Type | Default | Purpose |
| --- | --- | --- | --- |
| `GOOGLE_SCRIPT_ID` | `String` | empty | Google Script endpoint ID |
| `Google_Script_Master` | `String` | empty | Master script URL or token |
| `GOOGLE_SCRIPT_Boot` | `String` | empty | Boot-log endpoint URL or identifier |

## Shared Service Objects

| Global | Type | Default | Purpose |
| --- | --- | --- | --- |
| `espClient` | `WiFiClient` | constructed | TCP client used by MQTT |
| `client` | `PubSubClient` | constructed with `espClient` | Shared MQTT client |
| `display` | `Adafruit_SSD1306` | constructed | Shared OLED display object |

## Practical Use Guidelines

- Prefer updating configuration globals in `settings.cpp` or board-specific code rather than scattering assignments through control logic.
- Be careful with globals of type `char*` such as `BGstatus`, `db`, and `dbNew`. They currently point to string literals in some places, which can trigger compiler warnings. `const char*` is usually safer for those.
- For new cross-file state, add declarations in `include/globals.h` and definitions in `src/globals.cpp` only if the state truly needs to be global.
- For derived display-only values, prefer local variables or helper functions over adding more shared globals.