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

    if (showDisplay && remaining != lastDisplayedRemainingSec) {
      // Center the number horizontally; at setTextSize(7) each char is 42px wide
      int numDigits = (remaining >= 10) ? 2 : 1;
      int xPos = (SCREEN_WIDTH - numDigits * 42) / 2;

      display.clearDisplay();
      display.setTextSize(1);
      display.setCursor(0, 0);
      display.print(label);
      display.setTextSize(7);          // 42x56 px per char — fills the 64px screen
      display.setCursor(xPos, 8);
      display.print(remaining);
      display.display();

      lastDisplayedRemainingSec = remaining;
    }

        return false;  // still counting
    }

    // Countdown finished
    display.clearDisplay();
    display.setTextSize(1);              // restore default size
    display.display();
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
    servoA.write(openA);
    trace = "ON";
  } else {
    servoA.write(closedA);
    trace = "OFF";
  }

  // Apply B interlock at startup without changing the A-based trace text.
  if (lastStableStateB == LOW && lastStableStateA == HIGH) {
    servoB.write(openB);
  } else {
    servoB.write(closedB);
  }

  displayStat();
}

/**
 * Poll switches, debounce inputs, apply interlock logic, and drive servos.
 *
 * Interlock rule:
 * - B can only energize when A is stable HIGH (OFF).
 */
void servoControllerLoop() {

  // Keep OTA responsive in this mode even if display update frequency changes.
  ArduinoOTA.handle();
  yield();

  // -------------------- Read Inputs --------------------
  bool readingA = digitalRead(switchPinA);
  bool readingB = digitalRead(switchPinB);

  // -------------------- Debounce A --------------------
  if (readingA != lastReadingA) {
    lastDebounceTimeA = millis();
  }

  if ((millis() - lastDebounceTimeA) > debounceDelay) {
    if (readingA != lastStableStateA) {
      lastStableStateA = readingA;

      // A goes LOW → activate immediately
      if (lastStableStateA == LOW) {
        servoA.write(openA);
        trace = "ON";
        displayStat();

        // Force B off; cancel any B countdown
        servoB.write(closedB);
        // Keep B debounce state aligned to the real pin level to avoid stale edge state.
        lastReadingB = digitalRead(switchPinB);
        countdownActiveB = false;
        resumeCountdownOnBHigh = false;
        pausedRemainingB = 0;
      }

      // A goes HIGH → close servo immediately
      else {
        servoA.write(closedA);
        trace = "OFF";
        displayStat();
      }
    }
  }
  lastReadingA = readingA;

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
          servoB.write(openB);
          trace = "ON";
          displayStat();
        } else {
          // A is active — B not allowed
          servoB.write(closedB);
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
          servoB.write(closedB);
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
      servoB.write(closedB);
      countdownActiveB = false;
      resumeCountdownOnBHigh = false;
      pausedRemainingB = 0;
      trace = "OFF";
      displayStat();
    }
  }

  // Keep WiFi/OTA background work serviced even when no switch edge occurs.
  yield();
}

