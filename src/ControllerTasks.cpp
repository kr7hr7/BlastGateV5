#include "ControllerTasks.h"
#include <ESP32Servo.h>
#include <math.h>
#include "globals.h"

// -------------------- Servo Objects ----------------------
static bool servoAOn = false;
static bool servoBOn = false;

void gateTypeLServoSetup() {
  servoB.attach(servoPinB);
  servoB.write(closedB);  // Start closed
}

// Inputs use INPUT_PULLUP wiring: LOW = switch ON, HIGH = switch OFF.
static constexpr int SWITCH_ON = LOW;
static constexpr int SWITCH_OFF = HIGH;

static inline void updateRelayFromServoStates() {
  digitalWrite(reedRelayPin, (servoAOn || servoBOn) ? HIGH : LOW);
}

static inline void commandServoA(bool on) {
  servoA.write(on ? openA : closedA);
  servoAOn = on;
  updateRelayFromServoStates();
}

static inline void commandServoB(bool on) {
  servoB.write(on ? openB : closedB);
  servoBOn = on;
  updateRelayFromServoStates();
}

// -------------------- Debounce Variables ----------------
static unsigned long lastDebounceTimeA = 0;
static unsigned long lastDebounceTimeB = 0;
static unsigned long debounceDelay = 50;

static bool lastStableStateA = HIGH;
static bool lastReadingA = HIGH;

static bool lastStableStateB = HIGH;
static bool lastReadingB = HIGH;

static unsigned long lastSwitchDebugMs = 0;

static void logSwitchStates(const char* sourceTag, bool force = false) {
  const unsigned long now = millis();
  if (!force && (now - lastSwitchDebugMs < 500)) {
    return;
  }
  lastSwitchDebugMs = now;

  const int rawA = digitalRead(switchPinA);
  const int rawB = digitalRead(switchPinB);

  Serial.print("[");
  Serial.print(sourceTag);
  Serial.print("] switchPinA raw=");
  Serial.print(rawA);
  Serial.print(" state=");
  Serial.print((rawA == SWITCH_ON) ? "ON" : "OFF");
  Serial.print(" | switchPinB raw=");
  Serial.print(rawB);
  Serial.print(" state=");
  Serial.println((rawB == SWITCH_ON) ? "ON" : "OFF");
}

// -------------------- Countdown Control ----------------
static bool countdownActiveB = false;
static unsigned long countdownStartB = 0;
static int countdownDurationB = 0;
static int pausedRemainingB = 0;
static bool resumeCountdownOnBHigh = false;
static int lastDisplayedRemainingSec = -1;
static unsigned long lastBHighTime = 0;

static const int COUNTDOWN_MAX_SEC = 180;

static GateState deriveServoGateState() {
  if (eCode == 1) {
    return STATE_ERROR_1;
  }
  if (eCode == 2) {
    return STATE_ERROR_2;
  }
  if (eCode == 3) {
    return STATE_ERROR_3;
  }
  if (!servoA.attached() || !servoB.attached()) {
    return STATE_UNKNOWN;
  }
  if (countdownActiveB) {
    return STATE_CLOSING;
  }
  if (servoAOn || servoBOn) {
    return STATE_OPEN;
  }
  return STATE_CLOSED;
}

static inline void publishServoGateState(bool forcePublish = false) {
  setGateState(deriveServoGateState(), forcePublish);
}

static bool runCountdown(unsigned long startTime, int durationSec, const char* label, bool showDisplay)
{
    unsigned long elapsed = (millis() - startTime) / 1000;

    if (elapsed < durationSec) {
        int remaining = durationSec - elapsed;

    if (oledReady && showDisplay && remaining != lastDisplayedRemainingSec) {
      int numDigits = (remaining >= 10) ? 2 : 1;
      int xPos = (SCREEN_WIDTH - numDigits * 24) / 2;

      display.clearDisplay();
      display.setTextSize(1);
      display.setCursor(0, 0);
      display.print(label);
      display.setTextSize(4);
      display.setCursor(xPos, 8);
      display.print(remaining);
      display.display();

      lastDisplayedRemainingSec = remaining;
    }

        return false;
    }

    if (oledReady) {
      display.clearDisplay();
      display.setTextSize(1);
      display.display();
    }
  lastDisplayedRemainingSec = -1;
    return true;
}

void servoControllerSetup() {
  pinMode(switchPinA, INPUT_PULLUP);
  pinMode(switchPinB, INPUT_PULLUP);

  Wire.setTimeOut(20);

  lastReadingA = digitalRead(switchPinA);
  lastStableStateA = lastReadingA;
  lastReadingB = digitalRead(switchPinB);
  lastStableStateB = lastReadingB;

  logSwitchStates("servoSetup", true);

  servoA.attach(servoPinA);
  servoB.attach(servoPinB);

  if (lastStableStateA == SWITCH_ON) {
    commandServoA(true);
    trace = "ON";
  } else {
    commandServoA(false);
    trace = "OFF";
  }

  if (lastStableStateB == SWITCH_ON && lastStableStateA == SWITCH_OFF) {
    commandServoB(true);
  } else {
    commandServoB(false);
  }

  displayStat();
  publishServoGateState(true);
}

// ***************************************************************************
void servoControllerLoop() {
  ArduinoOTA.handle();
  yield();

  bool readingA = digitalRead(switchPinA);
  bool readingB = digitalRead(switchPinB);

  logSwitchStates("servoLoop");

  updateRelayFromServoStates();

  if (readingA != lastReadingA) {
    lastDebounceTimeA = millis();
  }

  if ((millis() - lastDebounceTimeA) > debounceDelay) {
    if (readingA != lastStableStateA) {
      lastStableStateA = readingA;
      logSwitchStates("A change", true);

      if (lastStableStateA == SWITCH_ON) {
        setGateState(STATE_OPENING);
        commandServoA(true);
        trace = "ON";
        displayStat();

        commandServoB(false);
        lastReadingB = digitalRead(switchPinB);
        countdownActiveB = false;
        resumeCountdownOnBHigh = false;
        pausedRemainingB = 0;
      }

      else {
        commandServoA(false);
        trace = "OFF";
        displayStat();
      }
    }
  }
  lastReadingA = readingA;

  if (lastStableStateA == SWITCH_ON) {
    countdownActiveB = false;
    resumeCountdownOnBHigh = false;
    pausedRemainingB = 0;
    lastDisplayedRemainingSec = -1;
    lastDebounceTimeB = millis();
    lastReadingB = readingB;
    lastStableStateB = readingB;
    yield();
    return;
  }

  if (readingB != lastReadingB) {
    lastDebounceTimeB = millis();
  }

  if ((millis() - lastDebounceTimeB) > debounceDelay) {
    if (readingB != lastStableStateB) {
      lastStableStateB = readingB;
      logSwitchStates("B change", true);

      if (lastStableStateB == SWITCH_ON) {
        if (countdownActiveB) {
          unsigned long elapsed = (millis() - countdownStartB) / 1000;
          int remaining = countdownDurationB - (int)elapsed;
          if (remaining < 0) remaining = 0;
          pausedRemainingB = remaining;
          resumeCountdownOnBHigh = true;
        }
        countdownActiveB = false;
        if (lastStableStateA == SWITCH_OFF) {
          setGateState(STATE_OPENING);
          commandServoB(true);
          trace = "ON";
          displayStat();
        } else {
          commandServoB(false);
          trace = "OFF";
          displayStat();
        }
      } else {
        unsigned long now = millis();
        bool doubleTap = (lastBHighTime != 0) && (now - lastBHighTime < (unsigned long)bDoubleTriggerMs);
        lastBHighTime = now;

        if (doubleTap) {
          countdownActiveB = false;
          countdownDurationB = 0;
          resumeCountdownOnBHigh = false;
          pausedRemainingB = 0;
          commandServoB(false);
          lastDisplayedRemainingSec = -1;
          trace = "OFF";
          displayStat();
        } else if (resumeCountdownOnBHigh) {
          int newTotal = pausedRemainingB + gateDelaySeconds;
          if (newTotal > COUNTDOWN_MAX_SEC) newTotal = COUNTDOWN_MAX_SEC;
          countdownActiveB = true;
          countdownStartB = now;
          countdownDurationB = newTotal;
          resumeCountdownOnBHigh = false;
          pausedRemainingB = 0;
          lastDisplayedRemainingSec = -1;
        } else {
          if (countdownActiveB) {
            unsigned long elapsed = (now - countdownStartB) / 1000;
            int remaining = countdownDurationB - (int)elapsed;
            if (remaining < 0) remaining = 0;
            int newTotal = remaining + gateDelaySeconds;
            if (newTotal > COUNTDOWN_MAX_SEC) newTotal = COUNTDOWN_MAX_SEC;
            countdownStartB = now;
            countdownDurationB = newTotal;
            lastDisplayedRemainingSec = -1;
          } else {
            countdownActiveB = true;
            countdownStartB = now;
            countdownDurationB = gateDelaySeconds;
            lastDisplayedRemainingSec = -1;
            trace = "OFF";
            displayStat();
          }
        }
      }
    }
  }
  lastReadingB = readingB;

  if (countdownActiveB) {
    if (runCountdown(countdownStartB, countdownDurationB, "B OFF in:", true)) {
      commandServoB(false);
      countdownActiveB = false;
      resumeCountdownOnBHigh = false;
      pausedRemainingB = 0;
      trace = "OFF";
      displayStat();
    }
  }

  publishServoGateState();
  yield();
}

// ***************************************************************************
void S_StaticPressureTasks()
{
  char payload[64];
  staticPressure = (analogRead(ANALOG_PIN_IN));
  currentMillis = millis();
  if ((((fabs(staticPressure - lastSample)) >= staticDelta)) || ((currentMillis - lastMsgTime >= maxInterval)))
  {
    delay(200);
    staticPressure = (analogRead(ANALOG_PIN_IN));
    if ((((fabs(staticPressure - lastSample)) >= staticDelta)) || ((currentMillis - lastMsgTime >= maxInterval)))
    {
      lastMsgTime = currentMillis;
      inchesH2O = ((staticPressure - 1600) * .0132);
      inchesH2O = round(inchesH2O * 10) / 10.0;
      if (inchesH2O < .1)
      {
        inchesH2O = 0;
      }
      delta = staticPressure - lastSample;
      lastSample = staticPressure;
      inchesH2O_str = String(inchesH2O, 1);
      trace = inchesH2O_str;
      displayStat();

      Serial.print(staticPressure);
      Serial.print("  ");
      Serial.print(delta);
      Serial.print("  ");
      Serial.print(inchesH2O_str);
      Serial.print("  ");

      dtostrf(inchesH2O, 1, 1, buffer);
      // Publish the message
      client.publish("backPressure", buffer);
      // Serial.print("MQTT Message Sent: ");
      Serial.println(buffer);
    }

    delay(interval);
  }
}

// ***************************************************************************
void manualGateTasks()
{
  sensorIn = (analogRead(ANALOG_PIN_IN));
  sensorIn = 4095 - sensorIn;

  if (sensorIn > (2000))
  {
    delay(200);
    if (gateOpenState != true)
    {
      digitalWrite(gateOn, HIGH);
      manualGateOpen();
    }
    else
    {
      trace = "ON";
      if (startTime != 0)
      {
        setGateState(STATE_OPEN);
        Serial.println(BGtopic);
      }
      displayStat();
      startTime = 0;
    }
  }

  if (sensorIn < trigger)
  {
    if (gateOpenState == true)
    {
      digitalWrite(gateOn, LOW);
      manualGateClose();
    }
  }
}

// ***************************************************************************
void gateTypeL_Tasks()
{
  // Gate type L:
  //   Switch A (LOW=ON) controls the stepper gate — ON opens, OFF closes.
  //   Switch B (LOW=ON) controls servo B — ON opens immediately, OFF starts
  //   a delayed close countdown.

  static bool lastSwitchBState = HIGH;
  static int lLastReadingA = HIGH;
  static int lStableReadingA = HIGH;
  static unsigned long lLastDebounceA = 0;
  static int lLastReadingB = HIGH;
  static int lStableReadingB = HIGH;
  static unsigned long lLastDebounceB = 0;
  static unsigned long lLastSwitchBTapReleaseMs = 0;
  const unsigned long lDebounceDelayA = 35;
  const unsigned long lDebounceDelayB = 35;
  const int lCountdownMaxSec = 180;

  const int rawSwitchA = digitalRead(switchPinA);
  if (rawSwitchA != lLastReadingA)
  {
    lLastDebounceA = millis();
    lLastReadingA = rawSwitchA;
  }
  if ((millis() - lLastDebounceA) >= lDebounceDelayA)
  {
    lStableReadingA = lLastReadingA;
  }
  const int rawSwitchB = digitalRead(switchPinB);
  if (rawSwitchB != lLastReadingB)
  {
    lLastDebounceB = millis();
    lLastReadingB = rawSwitchB;
  }
  if ((millis() - lLastDebounceB) >= lDebounceDelayB)
  {
    lStableReadingB = lLastReadingB;
  }

  const bool switchA = (lStableReadingA == LOW);
  const bool switchB = (lStableReadingB == LOW);
  const unsigned long nowMs = millis();

  // Keep gateCloseState in sync with the physical limit switch.
  checkSwitchState();

  // ---- Stepper: Switch A ON behaves like A/B/C/D "sensor high" (open/hold open) ----
  if (switchA)
  {
    // Opening request has priority: force Servo B off immediately.
    if (modeLServoBIsOpen || modeLServoBCountdownActive)
    {
      servoB.write(closedB);
      modeLServoBIsOpen = false;
      modeLServoBCountdownActive = false;
      modeLServoBCountdownDurationSec = 0;
      modeLServoBPausedRemainingSec = 0;
      lastDisplayedRemainingSec = -1;
      trace = "Off";
      displayStat();
      Serial.println("[TypeL] Switch A active -> Servo B FORCED OFF");
    }

    if (gateOpenState != true)
    {
      openGate();
    }
    else
    {
      // Keep OPEN state fresh while switch remains ON.
      gateOpenTime = millis();
      digitalWrite(reedRelayPin, HIGH);
      gateOpenState = true;
      trace = "OPEN";
      displayStat();
      startTime = 0;
      digitalWrite(enablePin, HIGH);
      setGateState(STATE_OPEN);
    }
  }
  // ---- Stepper: Switch A OFF behaves like A/B/C/D "sensor low" close path ----
  else
  {
    // If Switch B is active while stepper is being asked to close, command
    // Servo B ON before entering the blocking homing close path.
    if (switchB && !modeLServoBIsOpen)
    {
      servoB.write(openB);
      modeLServoBIsOpen = true;
      if (modeLServoBCountdownActive)
      {
        unsigned long elapsed = (nowMs - modeLServoBCountdownStart) / 1000;
        int remaining = modeLServoBCountdownDurationSec - (int)elapsed;
        if (remaining < 0) remaining = 0;
        modeLServoBPausedRemainingSec = remaining;
      }
      modeLServoBCountdownActive = false;
      lastDisplayedRemainingSec = -1;
      Serial.println("[TypeL] Switch B active during close -> Servo B OPEN");
    }

    // Requirement: when Switch A is deactivated in mode L, close immediately
    // without the closeGate countdown path.
    const bool stepperNeedsClose = (gateCloseState == false) ||
                                   (gateOpenState == true) ||
                                   (gateState == STATE_OPEN) ||
                                   (gateState == STATE_OPENING) ||
                                   (gateState == STATE_CLOSING);

    if (stepperNeedsClose && gateState != STATE_CLOSING)
    {
      startTime = 0;
      closeTime = 0;
      countDown = 0;
      homePosition();
    }

    // Keep mode L loop responsive for Switch B tap gestures.
    delay(5);
  }

  // Mode L interlock rule:
  //   Stepper OPENING or OPEN  -> servo forced OFF (stepper has priority)
  //   Stepper CLOSING or CLOSED -> servo controlled freely by Switch B / countdown
  const bool stepperInterlockActive = switchA ||
                                      (gateState == STATE_OPEN) ||
                                      (gateState == STATE_OPENING);

  if (stepperInterlockActive)
  {
    if (modeLServoBIsOpen || modeLServoBCountdownActive)
    {
      servoB.write(closedB);
      modeLServoBIsOpen = false;
      modeLServoBCountdownActive = false;
      modeLServoBCountdownDurationSec = 0;
      lastDisplayedRemainingSec = -1;
      trace = "Off";
      displayStat();
      Serial.println("[TypeL] Interlock: stepper active -> Servo B FORCED OFF");
    }

    // Sync edge baseline while B is ignored under interlock.
    lastSwitchBState = switchB ? LOW : HIGH;
    lLastSwitchBTapReleaseMs = 0;
  }

  // ---- Servo: driven by Switch B (countdown close on OFF) ----
  if (!stepperInterlockActive) {
    if (switchB != (lastSwitchBState == LOW)) {
      lastSwitchBState = switchB ? LOW : HIGH;

      if (switchB) {
        if (modeLServoBCountdownActive) {
          unsigned long elapsed = (nowMs - modeLServoBCountdownStart) / 1000;
          int remaining = modeLServoBCountdownDurationSec - (int)elapsed;
          if (remaining < 0) remaining = 0;
          modeLServoBPausedRemainingSec = remaining;
          modeLServoBCountdownActive = false;
        }
        servoB.write(openB);
        modeLServoBIsOpen = true;
        lastDisplayedRemainingSec = -1;
        Serial.println("[TypeL] Servo B OPEN");
      } else {
        const int tapIncrementSec = (gateDelaySeconds > 0) ? gateDelaySeconds : 1;
        const bool doubleTap = (lLastSwitchBTapReleaseMs != 0) &&
                               ((nowMs - lLastSwitchBTapReleaseMs) < (unsigned long)bDoubleTriggerMs);
        lLastSwitchBTapReleaseMs = nowMs;

        if (doubleTap) {
          modeLServoBCountdownActive = false;
          modeLServoBCountdownDurationSec = 0;
          modeLServoBPausedRemainingSec = 0;
          modeLServoBIsOpen = false;
          servoB.write(closedB);
          lastDisplayedRemainingSec = -1;
          trace = "Off";
          displayStat();
          digitalWrite(reedRelayPin, LOW);
          digitalWrite(greenLEDpin, LOW);
          Serial.println("[TypeL] Switch B double-tap -> Servo B OFF, countdown canceled");
        } else if (modeLServoBIsOpen || modeLServoBCountdownActive) {
          int baseRemaining = modeLServoBPausedRemainingSec;
          if (modeLServoBCountdownActive) {
            unsigned long elapsed = (nowMs - modeLServoBCountdownStart) / 1000;
            int remaining = modeLServoBCountdownDurationSec - (int)elapsed;
            if (remaining < 0) remaining = 0;
            baseRemaining = remaining;
          }

          int newTotal = baseRemaining + tapIncrementSec;
          if (newTotal > lCountdownMaxSec) newTotal = lCountdownMaxSec;
          modeLServoBCountdownActive = true;
          modeLServoBCountdownStart = nowMs;
          modeLServoBCountdownDurationSec = newTotal;
          modeLServoBPausedRemainingSec = 0;
          lastDisplayedRemainingSec = -1;
          Serial.print("[TypeL] Switch B single tap -> countdown +");
          Serial.print(tapIncrementSec);
          Serial.println(" sec");
        }
      }
    }

    if (modeLServoBCountdownActive) {
      if (runCountdown(modeLServoBCountdownStart, modeLServoBCountdownDurationSec, "B OFF in:", true)) {
        servoB.write(closedB);
        modeLServoBCountdownActive = false;
        modeLServoBIsOpen = false;
        modeLServoBPausedRemainingSec = 0;
        lastDisplayedRemainingSec = -1;
        trace = "Off";
        displayStat();
        Serial.println("[TypeL] Servo B CLOSED after countdown");
      }
    }
  }

  // Mode L output policy:
  // Relay/LED are ON only when either actuator path is active.
  const bool stepperActive = moveState || gateOpenState ||
                             (gateState == STATE_OPEN) ||
                             (gateState == STATE_OPENING) ||
                             (gateState == STATE_CLOSING);
  const bool servoActive = modeLServoBIsOpen || modeLServoBCountdownActive;
  const bool outputsOn = stepperActive || servoActive;

  digitalWrite(reedRelayPin, outputsOn ? HIGH : LOW);
  digitalWrite(greenLEDpin, outputsOn ? HIGH : LOW);

  ArduinoOTA.handle();
}

// ***************************************************************************

void gateL_Tasks()
{
  // This function is for gate type L, which uses switch A to control the stepper and switch B to control a servo.
  // The logic is implemented in gateTypeL_Tasks() for better organization, so this function can be left empty or used for any additional tasks if needed.




}