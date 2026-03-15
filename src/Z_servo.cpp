#include <ESP32Servo.h>
#include "globals.h"

/*
 * Servo Interlock Controller
 *
 * Behavior summary:
 * - Switch A is the primary command source.
 * - A LOW (active with INPUT_PULLUP) turns servo A ON immediately.
 * - A HIGH starts a delayed shutdown countdown for servo A.
 * - Servo B is interlocked: it is only allowed ON while A is fully OFF
 *   and A is not in its shutdown countdown.
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

// -------------------- Serial Status Print -------------
static unsigned long lastSwitchPrintMs = 0;
static const unsigned long switchPrintIntervalMs = 3000;

// -------------------- Countdown Control ----------------
static bool countdownActiveA = false;
static unsigned long countdownStartA = 0;
static int servoGateDelaySeconds = 10;   // delay time in seconds

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
static bool runCountdown(unsigned long startTime, int durationSec, const char* label)
{
    unsigned long elapsed = (millis() - startTime) / 1000;

    if (elapsed < durationSec) {
        int remaining = durationSec - elapsed;

        display.clearDisplay();
        display.setCursor(0, 20);
        display.print(label);
        display.setCursor(0, 45);
        display.print(remaining);
        display.print(" sec");
        display.display();

        return false;  // still counting
    }

    // Countdown finished
    display.clearDisplay();
    display.display();
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

  servoA.attach(servoPinA);
  servoB.attach(servoPinB);
}

/**
 * Poll switches, debounce inputs, apply interlock logic, and drive servos.
 *
 * Interlock rule:
 * - B can only energize when A is stable HIGH (OFF) and no A countdown is active.
 */
void servoControllerLoop() {

  // -------------------- Read Inputs --------------------
  bool readingA = digitalRead(switchPinA);
  bool readingB = digitalRead(switchPinB);

  // Print raw switch states every 3 seconds (LOW = active with INPUT_PULLUP)
  if ((millis() - lastSwitchPrintMs) >= switchPrintIntervalMs) {
    lastSwitchPrintMs = millis();
    Serial.print("SW A[");
    Serial.print(switchPinA);
    Serial.print("]=");
    Serial.print(readingA == LOW ? "LOW" : "HIGH");
    Serial.print(" B[");
    Serial.print(switchPinB);
    Serial.print("]=");
    Serial.println(readingB == LOW ? "LOW" : "HIGH");
  }

  // -------------------- Debounce A --------------------
  if (readingA != lastReadingA) {
    lastDebounceTimeA = millis();
  }

  if ((millis() - lastDebounceTimeA) > debounceDelay) {
    if (readingA != lastStableStateA) {
      lastStableStateA = readingA;

      // A goes LOW → activate immediately
      if (lastStableStateA == LOW) {
        servoA.write(120);

        // Force B off
        servoB.write(45);
        lastStableStateB = HIGH;

        countdownActiveA = false;
        display.clearDisplay();
        display.display();
      }

      // A goes HIGH → start countdown
      else {
        countdownActiveA = true;
        countdownStartA = millis();
      }
    }
  }
  lastReadingA = readingA;

  // -------------------- Handle A countdown --------------------
  if (countdownActiveA) {
    if (runCountdown(countdownStartA, servoGateDelaySeconds, "A OFF in:")) {
      servoA.write(45);
      countdownActiveA = false;
    }
  }

  // -------------------- Debounce B --------------------
  if (readingB != lastReadingB) {
    lastDebounceTimeB = millis();
  }

  if ((millis() - lastDebounceTimeB) > debounceDelay) {
    if (readingB != lastStableStateB) {
      lastStableStateB = readingB;

      // Only allow B if A is OFF and not counting down
      if (lastStableStateB == LOW && lastStableStateA == HIGH && !countdownActiveA) {
        servoB.write(120);
      } else {
        servoB.write(45);
      }
    }
  }
  lastReadingB = readingB;
}

