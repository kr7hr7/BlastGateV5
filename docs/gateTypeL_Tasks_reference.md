# gateTypeL_Tasks Reference

## Purpose
`gateTypeL_Tasks()` implements Type L behavior:
- Switch A (active-low) drives the stepper gate.
- Switch B (active-low) drives Servo B.
- Close sequencing mirrors the A/B/C/D close pattern (delay window, not-closed debounce, and close latch).

Function location:
- `src/ControllerTasks.cpp:403`

---

## High-Level Behavior (With Line Numbers)

1. Debounce Switch A and derive stable ON/OFF
   - `src/ControllerTasks.cpp:420-431`
2. Read Switch B and detect B edges
   - Read at `src/ControllerTasks.cpp:431`
   - Edge handling at `src/ControllerTasks.cpp:508-517`
3. Synchronize limit-switch-derived gate-close state
   - `checkSwitchState()` at `src/ControllerTasks.cpp:434`
4. Switch A ON path (open/hold open)
   - `src/ControllerTasks.cpp:437-457`
5. Switch A OFF path (A/B/C/D-style close state machine)
   - `src/ControllerTasks.cpp:460-504`
6. OTA service call
   - `src/ControllerTasks.cpp:520`

---

## Variable Inventory (Definition and Usage Lines)

### Function-local static state

- `lastSwitchBState`
  - Definition: `src/ControllerTasks.cpp:409`
  - Used: `src/ControllerTasks.cpp:508-509`

- `belowTriggerStart`
  - Definition: `src/ControllerTasks.cpp:410`
  - Used: `src/ControllerTasks.cpp:439,463,465,486`

- `notClosedStart`
  - Definition: `src/ControllerTasks.cpp:411`
  - Used: `src/ControllerTasks.cpp:440,466,472,474,479,484`

- `closeLatchedForLowSignal`
  - Definition: `src/ControllerTasks.cpp:412`
  - Used: `src/ControllerTasks.cpp:441,467,487,493,501`

- `lLastReadingA`
  - Definition: `src/ControllerTasks.cpp:413`
  - Used: `src/ControllerTasks.cpp:421,424,428`

- `lStableReadingA`
  - Definition: `src/ControllerTasks.cpp:414`
  - Used: `src/ControllerTasks.cpp:428,430`

- `lLastDebounceA`
  - Definition: `src/ControllerTasks.cpp:415`
  - Used: `src/ControllerTasks.cpp:423,426`

### Function-local constants

- `lDebounceDelayA`
  - Definition: `src/ControllerTasks.cpp:416`
  - Used: `src/ControllerTasks.cpp:426`

- `lowSignalCloseDelayMs`
  - Definition: `src/ControllerTasks.cpp:417`
  - Used: `src/ControllerTasks.cpp:486`

- `notClosedDebounceMs`
  - Definition: `src/ControllerTasks.cpp:418`
  - Used: `src/ControllerTasks.cpp:484`

### Function-local computed values

- `rawSwitchA`
  - Definition: `src/ControllerTasks.cpp:420`
  - Used: `src/ControllerTasks.cpp:421,424`

- `switchA`
  - Definition: `src/ControllerTasks.cpp:430`
  - Used: `src/ControllerTasks.cpp:437`

- `switchB`
  - Definition: `src/ControllerTasks.cpp:431`
  - Used: `src/ControllerTasks.cpp:508-509`

- `gateIndicatesOpen`
  - Definition: `src/ControllerTasks.cpp:482`
  - Used: `src/ControllerTasks.cpp:485`

- `closeInProgress`
  - Definition: `src/ControllerTasks.cpp:483`
  - Used: `src/ControllerTasks.cpp:489`

- `gatePhysicallyNotClosed`
  - Definition: `src/ControllerTasks.cpp:484`
  - Used: `src/ControllerTasks.cpp:485`

- `eligibleToStartClose`
  - Definition: `src/ControllerTasks.cpp:485-487`
  - Used: `src/ControllerTasks.cpp:489,491`

---

## Global Variables Referenced

Each entry shows declaration in `include/globals.h`, definition in `src/globals.cpp`, and representative usage in `gateTypeL_Tasks()`.

- `gateOpenState`
  - Decl: `include/globals.h:198`
  - Def: `src/globals.cpp:103`
  - Use: `src/ControllerTasks.cpp:443,452,482`

- `gateCloseState`
  - Decl: `include/globals.h:197`
  - Def: `src/globals.cpp:102`
  - Use: `src/ControllerTasks.cpp:470,499`

- `gateState`
  - Decl: `include/globals.h:139`
  - Def: `src/globals.cpp:44`
  - Use: `src/ControllerTasks.cpp:482,483,499`

- `startTime`
  - Decl: `include/globals.h:175`
  - Def: `src/globals.cpp:80`
  - Use: `src/ControllerTasks.cpp:455,483`

- `gateOpenTime`
  - Decl: `include/globals.h:174`
  - Def: `src/globals.cpp:79`
  - Use: `src/ControllerTasks.cpp:450`

- `closeSwitchDebounceMs`
  - Decl: `include/globals.h:216`
  - Def: `src/globals.cpp:121`
  - Use: `src/ControllerTasks.cpp:418`

- `holdTime`
  - Decl: `include/globals.h:168`
  - Def: `src/globals.cpp:73`
  - Use: `src/ControllerTasks.cpp:504`

- `switchPinA`
  - Decl: `include/globals.h:117`
  - Def: `src/globals.cpp:22`
  - Use: `src/ControllerTasks.cpp:420`

- `switchPinB`
  - Decl: `include/globals.h:119`
  - Def: `src/globals.cpp:24`
  - Use: `src/ControllerTasks.cpp:431`

- `reedRelayPin`
  - Decl: `include/globals.h:108`
  - Def: `src/globals.cpp:13`
  - Use: `src/ControllerTasks.cpp:451`

- `enablePin`
  - Decl: `include/globals.h:116`
  - Def: `src/globals.cpp:21`
  - Use: `src/ControllerTasks.cpp:456`

- `openB`
  - Decl: `include/globals.h:218`
  - Def: `src/globals.cpp:123`
  - Use: `src/ControllerTasks.cpp:512`

- `closedB`
  - Decl: `include/globals.h:220`
  - Def: `src/globals.cpp:125`
  - Use: `src/ControllerTasks.cpp:515`

---

## External Functions Called by gateTypeL_Tasks

- `checkSwitchState()`
  - Definition: `src/utilities.cpp:225`
  - Call site: `src/ControllerTasks.cpp:434`

- `openGate()`
  - Definition: `src/controlFunctions.cpp:204`
  - Call site: `src/ControllerTasks.cpp:445`

- `closeGate()`
  - Definition: `src/controlFunctions.cpp:103`
  - Call site: `src/ControllerTasks.cpp:495`

- `setGateState(...)`
  - Call sites: `src/ControllerTasks.cpp:457`

- `displayStat()`
  - Call sites: `src/ControllerTasks.cpp:454`

- `ArduinoOTA.handle()`
  - Call site: `src/ControllerTasks.cpp:520`

---

## Related Setup Function

- `gateTypeLServoSetup()`
  - Definition: `src/ControllerTasks.cpp:12`
  - Behavior: attaches Servo B and sets initial position to `closedB`.

---

## Notes

- Type L currently combines stepper control (Switch A) and servo control (Switch B) in one loop function.
- Close behavior is intentionally patterned after A/B/C/D logic to preserve proven countdown/close semantics.
- Line numbers in this document reflect the current checked-in file state and may shift after refactors.

---

## Related References (Index)

- `gateTypeL_Tasks` variable and flow reference:
  - `docs/gateTypeL_Tasks_reference.md`

- `closeGate` and `homePosition` variable and flow reference:
  - `docs/closeGate_homePosition_reference.md`
