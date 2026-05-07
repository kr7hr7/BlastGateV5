#include "ControllerTasks.h"
#include <ESP32Servo.h>
#include <math.h>
#include "globals.h"

// -------------------- Servo Objects ----------------------
static Servo servoA;
static Servo servoB;
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
  //   Switch B (LOW=ON) controls servo B — ON opens servo, OFF closes servo.

  static bool lastSwitchBState = HIGH;
  static unsigned long belowTriggerStart = 0;
  static unsigned long notClosedStart = 0;
  static bool closeLatchedForLowSignal = false;
  static int lLastReadingA = HIGH;
  static int lStableReadingA = HIGH;
  static unsigned long lLastDebounceA = 0;
  const unsigned long lDebounceDelayA = 35;
  const unsigned long lowSignalCloseDelayMs = 1000;
  const unsigned long notClosedDebounceMs = (unsigned long)((closeSwitchDebounceMs < 25) ? 25 : closeSwitchDebounceMs);

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
  const bool switchA = (lStableReadingA == LOW);
  const bool switchB = (digitalRead(switchPinB) == LOW);

  // Keep gateCloseState in sync with the physical limit switch.
  checkSwitchState();

  // ---- Stepper: Switch A ON behaves like A/B/C/D "sensor high" (open/hold open) ----
  if (switchA)
  {
    belowTriggerStart = 0;
    notClosedStart = 0;
    closeLatchedForLowSignal = false;

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
    if (belowTriggerStart == 0)
    {
      belowTriggerStart = millis();
      notClosedStart = 0;
      closeLatchedForLowSignal = false;
    }

    if (gateCloseState == false)
    {
      if (notClosedStart == 0)
      {
        notClosedStart = millis();
      }
    }
    else
    {
      notClosedStart = 0;
    }

    const bool gateIndicatesOpen = (gateOpenState == true) || (gateState == STATE_OPEN) || (gateState == STATE_OPENING);
    const bool closeInProgress = (startTime != 0) || (gateState == STATE_CLOSING);
    const bool gatePhysicallyNotClosed = (notClosedStart != 0) && ((millis() - notClosedStart) >= notClosedDebounceMs);
    const bool eligibleToStartClose = (gateIndicatesOpen || gatePhysicallyNotClosed) &&
                                      (millis() - belowTriggerStart >= lowSignalCloseDelayMs) &&
                                      (closeLatchedForLowSignal == false);

    if (closeInProgress || eligibleToStartClose)
    {
      if (eligibleToStartClose)
      {
        closeLatchedForLowSignal = true;
      }
      closeGate();
    }

    // Once a gate is confirmed closed during this low-signal period, suppress retrigger.
    if ((gateState == STATE_CLOSED) && (gateCloseState == true))
    {
      closeLatchedForLowSignal = true;
    }

    delay(holdTime);
  }

  // ---- Servo: driven by Switch B ----
  if (switchB != (lastSwitchBState == LOW)) {
    lastSwitchBState = switchB ? LOW : HIGH;

    if (switchB) {
      servoB.write(openB);
      Serial.println("[TypeL] Servo B OPEN");
    } else {
      servoB.write(closedB);
      Serial.println("[TypeL] Servo B CLOSED");
    }
  }

  ArduinoOTA.handle();
}

// ***************************************************************************

void gateL_Tasks()
{
  // This function is for gate type L, which uses switch A to control the stepper and switch B to control a servo.
  // The logic is implemented in gateTypeL_Tasks() for better organization, so this function can be left empty or used for any additional tasks if needed.




}