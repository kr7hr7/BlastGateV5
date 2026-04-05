#include "ControllerTasks.h"
#include <ESP32Servo.h>
#include <math.h>
#include "globals.h"

// -------------------- Servo Objects ----------------------
static Servo servoA;
static Servo servoB;
static bool servoAOn = false;
static bool servoBOn = false;

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

  servoA.attach(servoPinA);
  servoB.attach(servoPinB);

  if (lastStableStateA == LOW) {
    commandServoA(true);
    trace = "ON";
  } else {
    commandServoA(false);
    trace = "OFF";
  }

  if (lastStableStateB == LOW && lastStableStateA == HIGH) {
    commandServoB(true);
  } else {
    commandServoB(false);
  }

  displayStat();
  publishServoGateState(true);
}

void servoControllerLoop() {
  ArduinoOTA.handle();
  yield();

  bool readingA = digitalRead(switchPinA);
  bool readingB = digitalRead(switchPinB);

  updateRelayFromServoStates();

  if (readingA != lastReadingA) {
    lastDebounceTimeA = millis();
  }

  if ((millis() - lastDebounceTimeA) > debounceDelay) {
    if (readingA != lastStableStateA) {
      lastStableStateA = readingA;

      if (lastStableStateA == LOW) {
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

  if (lastStableStateA == LOW) {
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

      if (lastStableStateB == LOW) {
        if (countdownActiveB) {
          unsigned long elapsed = (millis() - countdownStartB) / 1000;
          int remaining = countdownDurationB - (int)elapsed;
          if (remaining < 0) remaining = 0;
          pausedRemainingB = remaining;
          resumeCountdownOnBHigh = true;
        }
        countdownActiveB = false;
        if (lastStableStateA == HIGH) {
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
  // Mirror the A/B/C/D gate flow, but invert trigger polarity:
  // L opens when analog input is below threshold and closes when above.
  static unsigned long aboveTriggerStart = 0;
  static unsigned long notClosedStart = 0;
  static bool closeLatchedForHighSignal = false;
  const unsigned long highSignalCloseDelayMs = 1000;
  const unsigned long notClosedDebounceMs = (unsigned long)((closeSwitchDebounceMs < 25) ? 25 : closeSwitchDebounceMs);

  checkSwitchState();
  sensorIn = (analogRead(ANALOG_PIN_IN));
  if (sensorIn < trigger)
  {
    aboveTriggerStart = 0;
    notClosedStart = 0;
    closeLatchedForHighSignal = false;
    delay(200);
    sensorIn = (analogRead(ANALOG_PIN_IN));
    if (sensorIn < trigger)
    {
      startTime = 0;
      dbNew = "LL140";
      if (gateOpenState != true)
      {
        dbNew = "LL142";
        openGate();
      }

      // If tool turns on during countdown to close, force open state.
      if (gateOpenState == true)
      {
        dbNew = "LL149";
        gateOpenTime = millis();
        sensor = sensorIn;
        digitalWrite(reedRelayPin, HIGH);
        gateOpenState = true;
        trace = "OPEN";
        displayStat();
        startTime = 0;
        digitalWrite(enablePin, HIGH);
        setGateState(STATE_OPEN);
      }
    }
  }

  if (sensorIn > (trigger + triggerDelta))
  {
    if (aboveTriggerStart == 0)
    {
      aboveTriggerStart = millis();
      notClosedStart = 0;
      closeLatchedForHighSignal = false;
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

    dbNew = "LL166";
    const bool gateIndicatesOpen = (gateOpenState == true) || (gateState == STATE_OPEN) || (gateState == STATE_OPENING);
    const bool closeInProgress = (startTime != 0) || (gateState == STATE_CLOSING);
    const bool gatePhysicallyNotClosed = (notClosedStart != 0) && ((millis() - notClosedStart) >= notClosedDebounceMs);
    const bool eligibleToStartClose = (gateIndicatesOpen || gatePhysicallyNotClosed) &&
                                      (millis() - aboveTriggerStart >= highSignalCloseDelayMs) &&
                                      (closeLatchedForHighSignal == false);
    if (closeInProgress || eligibleToStartClose)
    {
      if (eligibleToStartClose)
      {
        closeLatchedForHighSignal = true;
      }
      closeGate();
    }

    if ((gateState == STATE_CLOSED) && (gateCloseState == true))
    {
      closeLatchedForHighSignal = true;
    }

    delay(holdTime);
  }
  else
  {
    aboveTriggerStart = 0;
    notClosedStart = 0;
    closeLatchedForHighSignal = false;
  }
  ArduinoOTA.handle();
}

// ***************************************************************************