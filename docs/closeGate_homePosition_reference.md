# closeGate and homePosition Reference

## Scope
This document describes:
- `homePosition()` in `src/controlFunctions.cpp:9`
- `closeGate()` in `src/controlFunctions.cpp:103`

It includes flow summaries and line-numbered variable references.

---

## homePosition

### Purpose
`homePosition()` drives the stepper toward the limit switch until a stable HOME condition is detected, then marks gate state as closed.

### Flow (line-numbered)
1. Initialize close direction and state: `src/controlFunctions.cpp:10-17`
2. Reset step position and print initial home diagnostics: `src/controlFunctions.cpp:19-22`
3. Compute homing pulse and limits:
- `homePulseUs`: `src/controlFunctions.cpp:26`
- `homeStepBudget`: `src/controlFunctions.cpp:29`
- `maxHomeDurationMs`: `src/controlFunctions.cpp:35`
4. Homing loop:
- Debounced HOME check via `checkSwitchState()`: `src/controlFunctions.cpp:39-45`
- Step pulse output: `src/controlFunctions.cpp:49-53`
- OTA/yield servicing: `src/controlFunctions.cpp:56-59`
- Periodic stepping diagnostics: `src/controlFunctions.cpp:61-66`
- Timeout error path: `src/controlFunctions.cpp:68-72`
- Step-budget error path: `src/controlFunctions.cpp:74-79`
5. Finalize closed state: `src/controlFunctions.cpp:83-96`

### Local constants/locals (definition and use)
- `LIMIT_SWITCH_HOME_STATE`
  - Definition: `src/controlFunctions.cpp:5`
  - Used: `src/controlFunctions.cpp:22,63,87`

- `minHomePulseUs`
  - Definition: `src/controlFunctions.cpp:25`
  - Used: `src/controlFunctions.cpp:26`

- `homePulseUs`
  - Definition: `src/controlFunctions.cpp:26`
  - Used: `src/controlFunctions.cpp:31,35,50,52`

- `homeStartTime`
  - Definition: `src/controlFunctions.cpp:28`
  - Used: `src/controlFunctions.cpp:68`

- `homeStepBudget`
  - Definition: `src/controlFunctions.cpp:29`
  - Used: `src/controlFunctions.cpp:33,74`

- `maxHomeDurationMs`
  - Definition: `src/controlFunctions.cpp:35`
  - Used: `src/controlFunctions.cpp:68`

- `lastYield`
  - Definition: `src/controlFunctions.cpp:36`
  - Current note: defined but not used in current implementation.

- `rawLimit`
  - Definition: `src/controlFunctions.cpp:62`
  - Used: `src/controlFunctions.cpp:64,66`

### Global variables used (decl/def/use)
- `enablePin`
  - Decl: `include/globals.h:116`
  - Def: `src/globals.cpp:21`
  - Use: `src/controlFunctions.cpp:11,90`

- `dirPin`
  - Decl: `include/globals.h:115`
  - Def: `src/globals.cpp:20`
  - Use: `src/controlFunctions.cpp:12,14`

- `rotation`
  - Decl: `include/globals.h:140`
  - Def: `src/globals.cpp:45`
  - Use: `src/controlFunctions.cpp:13`

- `stepPosition`
  - Decl: `include/globals.h:149`
  - Def: `src/globals.cpp:54`
  - Use: `src/controlFunctions.cpp:19,53,56,61,74,91`

- `limitSwitchPin`
  - Decl: `include/globals.h:107`
  - Def: `src/globals.cpp:12`
  - Use: `src/controlFunctions.cpp:20,62,85`

- `delayTime`
  - Decl: `include/globals.h:233`
  - Def: `src/globals.cpp:141`
  - Use: `src/controlFunctions.cpp:26`

- `fullRunSteps`
  - Decl: `include/globals.h:159`
  - Def: `src/globals.cpp:64`
  - Use: `src/controlFunctions.cpp:29`

- `maxMissedSteps`
  - Decl: `include/globals.h:232`
  - Def: `src/globals.cpp:140`
  - Use: `src/controlFunctions.cpp:29`

- `stepPin`
  - Decl: `include/globals.h:114`
  - Def: `src/globals.cpp:19`
  - Use: `src/controlFunctions.cpp:49,51`

- `gateCloseState`
  - Decl: `include/globals.h:197`
  - Def: `src/globals.cpp:102`
  - Use: `src/controlFunctions.cpp:41,44,92`

- `gateOpenState`
  - Decl: `include/globals.h:198`
  - Def: `src/globals.cpp:103`
  - Use: `src/controlFunctions.cpp:93`

- `gateState`
  - Decl: `include/globals.h:139`
  - Def: `src/globals.cpp:44`
  - Use via `setGateState`: `src/controlFunctions.cpp:17,70,76,83`

- `trace`
  - Decl: `include/globals.h` (String trace)
  - Def: `src/globals.cpp`
  - Use: `src/controlFunctions.cpp:15,94`

- `eCode`
  - Decl: `include/globals.h`
  - Def: `src/globals.cpp`
  - Use: `src/controlFunctions.cpp:71,78`

- `dbNew`
  - Decl: `include/globals.h`
  - Def: `src/globals.cpp`
  - Use: `src/controlFunctions.cpp:48`

### External functions called by homePosition
- `checkSwitchState()`
  - Def: `src/utilities.cpp:225`
  - Calls: `src/controlFunctions.cpp:10,40,43,96`

- `displayStat()`
  - Calls: `src/controlFunctions.cpp:16,95`

- `setGateState(...)`
  - Calls: `src/controlFunctions.cpp:17,70,76,83`

- `errorState()`
  - Calls: `src/controlFunctions.cpp:72,79`

- `ArduinoOTA.handle()` and `yield()`
  - Calls: `src/controlFunctions.cpp:57,58`

---

## closeGate

### Purpose
`closeGate()` starts and services a close countdown; when countdown expires, it homes the gate and finalizes CLOSED state.

### Flow (line-numbered)
1. Set closing intent and relay behavior: `src/controlFunctions.cpp:104-109`
2. One-time initialization for this close cycle (when `startTime == 0`):
- set `startTime`, `toolRunTime`, `runTime`, `closeTime`: `src/controlFunctions.cpp:111-114`
- optional OLED setup screen: `src/controlFunctions.cpp:115-146`
3. Every call:
- refresh `currentTime` and `countDown`: `src/controlFunctions.cpp:151-152`
- optional countdown OLED screen: `src/controlFunctions.cpp:153-184`
4. Completion:
- when `currentTime >= closeTime`, call `homePosition()` and finalize state: `src/controlFunctions.cpp:186-195`

### Local variables (definition and use)
- `mqttTopic`
  - Definition: `src/controlFunctions.cpp:104`
  - Current note: defined for consistency with other routines; not otherwise used.

- `delayText`
  - Definition: `src/controlFunctions.cpp:116`
  - Used: `src/controlFunctions.cpp:131,136`

- `countdownText`
  - Definition: `src/controlFunctions.cpp:154`
  - Used: `src/controlFunctions.cpp:169,174`

- `x1`, `y1`, `textW`, `textH`, and computed cursor variables
  - Definitions in OLED blocks: `src/controlFunctions.cpp:117-145` and `src/controlFunctions.cpp:155-183`
  - Used for text centering on display.

### Global variables used (decl/def/use)
- `gateType`
  - Decl: `include/globals.h`
  - Def: `src/globals.cpp`
  - Use: `src/controlFunctions.cpp:105`

- `reedRelayPin`
  - Decl: `include/globals.h:108`
  - Def: `src/globals.cpp:13`
  - Use: `src/controlFunctions.cpp:106,187`

- `moveState`
  - Decl: `include/globals.h`
  - Def: `src/globals.cpp`
  - Use: `src/controlFunctions.cpp:108,189`

- `startTime`
  - Decl: `include/globals.h:175`
  - Def: `src/globals.cpp:80`
  - Use: `src/controlFunctions.cpp:111,193`

- `toolRunTime`
  - Decl: `include/globals.h:206`
  - Def: `src/globals.cpp:111`
  - Use: `src/controlFunctions.cpp:112`

- `onTime`
  - Decl: `include/globals.h:177`
  - Def: `src/globals.cpp:82`
  - Use: `src/controlFunctions.cpp:112`

- `runTime`
  - Decl: `include/globals.h:181`
  - Def: `src/globals.cpp:86`
  - Use: `src/controlFunctions.cpp:113`

- `gateOpenTime`
  - Decl: `include/globals.h:174`
  - Def: `src/globals.cpp:79`
  - Use: `src/controlFunctions.cpp:113`

- `closeDelayTime`
  - Decl: `include/globals.h:172`
  - Def: `src/globals.cpp:77`
  - Use: `src/controlFunctions.cpp:114`

- `closeTime`
  - Decl: `include/globals.h:176`
  - Def: `src/globals.cpp:81`
  - Use: `src/controlFunctions.cpp:114,152,186`

- `gateDelaySeconds`
  - Decl: `include/globals.h:214`
  - Def: `src/globals.cpp:119`
  - Use: `src/controlFunctions.cpp:116`

- `oledReady`, `display`
  - Decl: `include/globals.h`
  - Def: `src/globals.cpp`
  - Use: `src/controlFunctions.cpp:115-146,153-184`

- `currentTime`
  - Decl: `include/globals.h:173`
  - Def: `src/globals.cpp:78`
  - Use: `src/controlFunctions.cpp:151,152,186`

- `countDown`
  - Decl: `include/globals.h:151`
  - Def: `src/globals.cpp:56`
  - Use: `src/controlFunctions.cpp:152,154`

- `trace`
  - Decl: `include/globals.h`
  - Def: `src/globals.cpp`
  - Use: `src/controlFunctions.cpp:190`

- `sensor`
  - Decl: `include/globals.h:148`
  - Def: `src/globals.cpp:53`
  - Use: `src/controlFunctions.cpp:194`

### External functions called by closeGate
- `setGateState(STATE_CLOSING)`
  - Call: `src/controlFunctions.cpp:109`

- `homePosition()`
  - Call: `src/controlFunctions.cpp:188`

- `displayStat()`
  - Call: `src/controlFunctions.cpp:191`

- `logCycle()`
  - Call: `src/controlFunctions.cpp:195`

---

## Notes
- Line numbers reflect the current source snapshot and may shift after future edits.
- `closeGate()` is intentionally stateful and must be called repeatedly while close is active (countdown + completion path).
