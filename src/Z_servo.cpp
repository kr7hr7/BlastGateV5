#include <ESP32Servo.h>
#include "globals.h"

/*
 * Servo Interlock Controller
 *
 * Behavior summary:
 * - Switch A is the primary command source.
 * - A LOW (active with INPUT_PULLUP) turns servo A ON immediately.
 * - A HIGH closes servo A immediately.
 * - B LOW turns servo B ON (only allowed when A is OFF).
 * - B HIGH starts a shutdown countdown; servo B closes when countdown ends.
 *
 * This module is intentionally stateful and uses static file-scope variables
 * so setup/loop can be called from the main scheduler without class wrappers.
 */



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
static int countdownDurationB = 0;         // current effective duration in seconds
static int pausedRemainingB = 0;           // remaining seconds when countdown is paused by B LOW
static bool resumeCountdownOnBHigh = false;
static int lastDisplayedRemainingSec = -1;
static unsigned long lastBHighTime = 0;   // tracks last B LOW→HIGH for double-tap detection

static const int COUNTDOWN_MAX_SEC = 180;  // hard ceiling on total countdown

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

// ---------------------------------------------------------
//                COUNTDOWN FUNCTION
// ---------------------------------------------------------
/**
 * Render a countdown on the OLED and report when it has completed.
 *
 * @param startTime   Timestamp from millis() when countdown began.
 * @param durationSec Countdown duration in seconds.
 * @param label       Short message shown above the remaining time.
 * @return true when countdown has elapsed; false while still counting.
 */
static bool runCountdown(unsigned long startTime, int durationSec, const char* label, bool showDisplay)
{
    unsigned long elapsed = (millis() - startTime) / 1000;

    if (elapsed < durationSec) {
        int remaining = durationSec - elapsed;

    if (oledReady && showDisplay && remaining != lastDisplayedRemainingSec) {
      // Center the number horizontally; at setTextSize(4) each char is 24px wide
      int numDigits = (remaining >= 10) ? 2 : 1;
      int xPos = (SCREEN_WIDTH - numDigits * 24) / 2;

      display.clearDisplay();
      display.setTextSize(1);
      display.setCursor(0, 0);
      display.print(label);
      display.setTextSize(4);          // 24x32 px per char
      display.setCursor(xPos, 8);
      display.print(remaining);
      display.display();

      lastDisplayedRemainingSec = remaining;
    }

        return false;  // still counting
    }

    // Countdown finished
    if (oledReady) {
      display.clearDisplay();
      display.setTextSize(1);              // restore default size
      display.display();
    }
  lastDisplayedRemainingSec = -1;
    return true;
}

/**
 * Configure GPIO inputs and attach the two servo outputs.
 *
 * Input switches use INPUT_PULLUP, so active state is LOW.
 */
void servoControllerSetup() {
  pinMode(switchPinA, INPUT_PULLUP);
  pinMode(switchPinB, INPUT_PULLUP);

  // Prevent rare I2C OLED stalls from blocking the control loop/OTA forever.
  Wire.setTimeOut(20);

  // Initialize debounced states from real pin levels to avoid false startup transitions.
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

  // Apply B interlock at startup without changing the A-based trace text.
  if (lastStableStateB == LOW && lastStableStateA == HIGH) {
    commandServoB(true);
  } else {
    commandServoB(false);
  }

  displayStat();
  // Publish current servo-derived state once at startup.
  publishServoGateState(true);
}

/**
 * Poll switches, debounce inputs, apply interlock logic, and drive servos.
 *
 * Interlock rule:
 * - While A is active, all B input changes are ignored.
 * - B can only energize when A is stable HIGH (OFF).
 */
void servoControllerLoop() {

  // Keep OTA responsive in this mode even if display update frequency changes.
  ArduinoOTA.handle();
  yield();

  // -------------------- Read Inputs --------------------
  bool readingA = digitalRead(switchPinA);
  bool readingB = digitalRead(switchPinB);

  // Keep relay aligned to commanded servo states even if another path wrote the pin.
  updateRelayFromServoStates();

  // -------------------- Debounce A --------------------
  if (readingA != lastReadingA) {
    lastDebounceTimeA = millis();
  }

  if ((millis() - lastDebounceTimeA) > debounceDelay) {
    if (readingA != lastStableStateA) {
      lastStableStateA = readingA;

      // A goes LOW → activate immediately
      if (lastStableStateA == LOW) {
        setGateState(STATE_OPENING);
        commandServoA(true);
        trace = "ON";
        displayStat();

        // Force B off; cancel any B countdown
        commandServoB(false);
        // Keep B debounce state aligned to the real pin level to avoid stale edge state.
        lastReadingB = digitalRead(switchPinB);
        countdownActiveB = false;
        resumeCountdownOnBHigh = false;
        pausedRemainingB = 0;
      }

      // A goes HIGH → close servo immediately
      else {
        commandServoA(false);
        trace = "OFF";
        displayStat();
      }
    }
  }
  lastReadingA = readingA;

  // Ignore all B input activity while A is active and keep debounce state aligned
  // so B does not replay stale edges once A is released.
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

  // -------------------- Debounce B --------------------
  if (readingB != lastReadingB) {
    lastDebounceTimeB = millis();
  }

  if ((millis() - lastDebounceTimeB) > debounceDelay) {
    if (readingB != lastStableStateB) {
      lastStableStateB = readingB;

      if (lastStableStateB == LOW) {
        // B turned ON: pause countdown so next B HIGH can add delay
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
          // A is active — B not allowed
          commandServoB(false);
          trace = "OFF";
          displayStat();
        }
      } else {
        // B turned OFF → start countdown, or extend if already counting
        unsigned long now = millis();
        bool doubleTap = (lastBHighTime != 0) && (now - lastBHighTime < (unsigned long)bDoubleTriggerMs);
        lastBHighTime = now;

        if (doubleTap) {
          // Two triggers within 1 s — abort countdown and close servo immediately
          countdownActiveB = false;
          countdownDurationB = 0;
          resumeCountdownOnBHigh = false;
          pausedRemainingB = 0;
          commandServoB(false);
          lastDisplayedRemainingSec = -1;
          trace = "OFF";
          displayStat();
        } else if (resumeCountdownOnBHigh) {
          // Resume paused countdown and add gateDelaySeconds.
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
            // Extend: add gateDelaySeconds to remaining time, cap at COUNTDOWN_MAX_SEC
            unsigned long elapsed = (now - countdownStartB) / 1000;
            int remaining = countdownDurationB - (int)elapsed;
            if (remaining < 0) remaining = 0;
            int newTotal = remaining + gateDelaySeconds;
            if (newTotal > COUNTDOWN_MAX_SEC) newTotal = COUNTDOWN_MAX_SEC;
            // Reset start to now with the new total as duration
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

  // -------------------- Handle B countdown --------------------
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

  // Publish only when derived state changed (open/closed/opening/closing/unknown).
  publishServoGateState();

  // Keep WiFi/OTA background work serviced even when no switch edge occurs.
  yield();
}

