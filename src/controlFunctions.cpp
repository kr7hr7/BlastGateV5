#include "controlFunctions.h"
#include "globals.h"

namespace {
constexpr int LIMIT_SWITCH_HOME_STATE = LOW;
constexpr unsigned long MIN_HOME_PULSE_US = 75UL;
constexpr unsigned long HOME_RAW_CONFIRM_US = 800UL;

static inline bool isHomeSwitchActiveRaw(int raw) {
  return raw == LIMIT_SWITCH_HOME_STATE;
}

// Apply logical direction and invert it when rotation is enabled.
static inline void setStepperDirection(bool clockwise) {
  bool dirHigh = clockwise;
  if (rotation) {
    dirHigh = !dirHigh;
  }
  digitalWrite(dirPin, dirHigh ? HIGH : LOW);
}
}

// ***************************************************************************
void homePosition() {
  checkSwitchState();

  // Do not trust a potentially stale software flag here; verify raw switch.
  if (isHomeSwitchActiveRaw(digitalRead(limitSwitchPin))) {
    delayMicroseconds(HOME_RAW_CONFIRM_US);
    if (isHomeSwitchActiveRaw(digitalRead(limitSwitchPin))) {
      setGateState(STATE_CLOSED);
      digitalWrite(enablePin, HIGH);  // disable stepper (idle)
      stepPosition = 0;
      gateCloseState = true;
      gateOpenState = false;
      limitSwitchState = true;
      Serial.println("[homePosition] already at home; skipping homing steps");
      return;
    }
  }

  setStepperDirection(true);  // close direction
  digitalWrite(enablePin, LOW);  // enable stepper (active during stepping)
  trace = "Closing";
  displayStat();
  setGateState(STATE_CLOSING);

  stepPosition = 0;
  Serial.print("[homePosition] limitSwitchPin raw=");
  Serial.print(digitalRead(limitSwitchPin));
  Serial.print(" state=");
  Serial.println(isHomeSwitchActiveRaw(digitalRead(limitSwitchPin)) ? "HOME" : "NOT_HOME");

  // Keep a conservative lower bound for homing, but allow the configured
  // step timing to drive close speed on faster drivers.
  const unsigned long homePulseUs = (unsigned long)((delayTime < (int)MIN_HOME_PULSE_US) ? MIN_HOME_PULSE_US : delayTime);

  const unsigned long homeStartTime = millis();
  const unsigned long homeStepBudget = (unsigned long)(fullRunSteps + (maxMissedSteps * 8));
  // Mode L close-path servo servicing while homing is in progress.
  // homePosition() is blocking, so Switch B/countdown must update the same
  // shared state used by gateTypeL_Tasks().
  static int hpLastReadingB = HIGH;
  static int hpStableReadingB = HIGH;
  static unsigned long hpLastDebounceB = 0;
  const unsigned long hpDebounceDelayB = 35;

  Serial.print("[homePosition] pulseUs=");
  Serial.print(homePulseUs);
  Serial.print(" stepBudget=");
  Serial.println(homeStepBudget);
  // Time-based failsafe for unexpected stalls in close/homing state.
  const unsigned long maxHomeDurationMs = (homeStepBudget * (homePulseUs * 2UL + 350UL) / 1000UL) + 3000UL;
  for (;;) {
    checkSwitchState();

    if (gateType == "L") {
      const unsigned long nowMs = millis();
      const int rawSwitchB = digitalRead(switchPinB);
      if (rawSwitchB != hpLastReadingB) {
        hpLastDebounceB = nowMs;
        hpLastReadingB = rawSwitchB;
      }
      if ((nowMs - hpLastDebounceB) >= hpDebounceDelayB) {
        hpStableReadingB = hpLastReadingB;
      }

      const bool switchBOn = (hpStableReadingB == LOW);
      if (switchBOn) {
        if (!modeLServoBIsOpen) {
          servoB.write(openB);
          modeLServoBIsOpen = true;
          Serial.println("[homePosition/TypeL] Switch B ON -> Servo B OPEN");
        }
        modeLServoBCountdownActive = false;
        modeLServoBPausedRemainingSec = 0;
      } else {
        if (modeLServoBIsOpen && !modeLServoBCountdownActive) {
          modeLServoBCountdownDurationSec = (gateDelaySeconds > 0) ? gateDelaySeconds : 1;
          modeLServoBCountdownStart = nowMs;
          modeLServoBCountdownActive = true;
          Serial.print("[homePosition/TypeL] Switch B OFF -> countdown ");
          Serial.print(modeLServoBCountdownDurationSec);
          Serial.println(" sec");
        }

        if (modeLServoBCountdownActive &&
            (nowMs - modeLServoBCountdownStart) >= (unsigned long)modeLServoBCountdownDurationSec * 1000UL) {
          servoB.write(closedB);
          modeLServoBIsOpen = false;
          modeLServoBCountdownActive = false;
          modeLServoBPausedRemainingSec = 0;
          Serial.println("[homePosition/TypeL] Servo B CLOSED after countdown");
        }
      }
    }

    if (limitSwitchState) {
      break;
    }

    // Home detection must be immediate and deterministic: use raw switch
    // state with a short confirm delay to reject very brief chatter.
    if (isHomeSwitchActiveRaw(digitalRead(limitSwitchPin))) {
      delayMicroseconds(HOME_RAW_CONFIRM_US);
      if (isHomeSwitchActiveRaw(digitalRead(limitSwitchPin))) {
        break;
      }
    }

    dbNew="CF16";
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(homePulseUs);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(homePulseUs);
    stepPosition++;
    
    // Yield to background tasks every 100 steps (~17ms)
    if (stepPosition % 100 == 0) {
      ArduinoOTA.handle();
      yield();
    }

    if (stepPosition % 250 == 0) {
      const int rawLimit = digitalRead(limitSwitchPin);
      Serial.print("[homePosition] stepping raw=");
      Serial.print(rawLimit);
      Serial.print(" state=");
      Serial.println(isHomeSwitchActiveRaw(rawLimit) ? "HOME" : "NOT_HOME");
    }

    if ((millis() - homeStartTime) > maxHomeDurationMs) {
      Serial.println("Home timeout waiting for limit switch");
      setGateState(STATE_UNKNOWN);
      eCode = 5; // Error code 5: close/homing timeout
      errorState();
    }
    
    if (stepPosition >= homeStepBudget) {
      Serial.println("Home line 22 ");
      setGateState(STATE_UNKNOWN);
      
      eCode=1; //                                         Error code 1
      errorState();
    }
  }

  // Disable stepper after stepping completes
  digitalWrite(enablePin, HIGH);

  setGateState(STATE_CLOSED);
  Serial.print("[homePosition] reached home raw=");
  Serial.print(digitalRead(limitSwitchPin));
  Serial.print(" state=");
  Serial.println(isHomeSwitchActiveRaw(digitalRead(limitSwitchPin)) ? "HOME" : "NOT_HOME");
  stepPosition = 0;
  gateCloseState = true;
  gateOpenState = false;
  trace = "Closed";
  displayStat();
  checkSwitchState();
}

// ***************************************************************************
void closeGate() {
  const char* mqttTopic = BGtopic;
  if (gateType == "A" || gateType == "B") {
    digitalWrite(reedRelayPin, LOW);
  }
  moveState = true;
  setGateState(STATE_CLOSING);

  if (startTime == 0) {
    startTime = millis();
    toolRunTime = ((millis() -onTime)/3600000.0);
    runTime = int((startTime - gateOpenTime) / 1000);
    closeTime = closeDelayTime + startTime;
    if (oledReady) {
      String delayText = String(gateDelaySeconds);
      int16_t x1, y1;
      uint16_t textW, textH;

      display.clearDisplay();
      display.setTextColor(WHITE);
      display.setTextSize(2);
      display.getTextBounds("Close in", 0, 0, &x1, &y1, &textW, &textH);
      int closeInX = (display.width() - textW) / 2 - x1;
      if (closeInX < 0) {
        closeInX = 0;
      }
      display.setCursor(closeInX, 0);
      display.print("Close in");
      display.setTextSize(4);
      display.getTextBounds(delayText, 0, 0, &x1, &y1, &textW, &textH);
      int delayX = (display.width() - textW) / 2 - x1;
      if (delayX < 0) {
        delayX = 0;
      }
      display.setCursor(delayX, 18);
      display.print(delayText);
      display.setTextSize(2);
      display.getTextBounds("Seconds", 0, 0, &x1, &y1, &textW, &textH);
      int secondsX = (display.width() - textW) / 2 - x1;
      if (secondsX < 0) {
        secondsX = 0;
      }
      display.setCursor(secondsX, 50);
      display.print("Seconds");
      display.display();
      display.setTextSize(4);
    }

    // write_to_google_sheet();
  }
  currentTime = millis();
  countDown = (closeTime - currentTime) / 1000;
  if (oledReady) {
    String countdownText = String(countDown);
    int16_t x1, y1;
    uint16_t textW, textH;

    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(2);
    display.getTextBounds("Close in", 0, 0, &x1, &y1, &textW, &textH);
    int closeInX = (display.width() - textW) / 2 - x1;
    if (closeInX < 0) {
      closeInX = 0;
    }
    display.setCursor(closeInX, 0);
    display.print("Close in");
    display.setTextSize(4);
    display.getTextBounds(countdownText, 0, 0, &x1, &y1, &textW, &textH);
    int countdownX = (display.width() - textW) / 2 - x1;
    if (countdownX < 0) {
      countdownX = 0;
    }
    display.setCursor(countdownX, 18);
    display.print(countdownText);
    display.setTextSize(2);
    display.getTextBounds("Seconds", 0, 0, &x1, &y1, &textW, &textH);
    int secondsX = (display.width() - textW) / 2 - x1;
    if (secondsX < 0) {
      secondsX = 0;
    }
    display.setCursor(secondsX, 50);
    display.print("Seconds");
    display.display();
  }

  if (currentTime >= closeTime) {
    digitalWrite(reedRelayPin, LOW);
    homePosition();
    moveState = false;
    trace = "CLOSED";
    displayStat();
    startTime = 0;
    sensor = 0;
    logCycle();
    
    //closeTime = 0;
  }
}

// ***************************************************************************
void openGate() {
  //Serial.println ("OpenGate line 89 ");

  const char* mqttTopic = BGtopic;
  checkSwitchState();
  if (gateCloseState != true) {
    Serial.println("[openGate] gate not confirmed CLOSED/HOME; running homing cycle first");
    homePosition();
    checkSwitchState();
    if (gateCloseState != true) {
      Serial.println("[openGate] homing did not establish CLOSED/HOME; aborting open");
      trace = "NOT_HOME";
      displayStat();
      setGateState(STATE_UNKNOWN);
      moveState = false;
      return;
    }
  }

  moveState = true;
  //Serial.println (" Open Gate");
  gateOpenTime = millis();
  onTime= millis();
  
  sensor = sensorIn;

  digitalWrite(reedRelayPin, HIGH);
  digitalWrite(gateOn, HIGH);
  setGateState(STATE_OPENING);
  // Opening is step-count driven: do not invoke homing here.
  // The close path is responsible for establishing/refreshing home.
  setGateState(STATE_OPENING);
  //Serial.println(BGtopic);

  if (oledReady) {
    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(3);
    display.setCursor(0, 20);
    display.print("Opening");
    display.display();
  }
  //Serial.println(" OpenGate line 115");
  setStepperDirection(false);  // open direction
  digitalWrite(enablePin, LOW);  // enable stepper (active during stepping)
  for (stepPosition = 0; stepPosition < fullRunSteps; stepPosition++) {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(delayTime);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(delayTime);
    
    // Yield to background tasks every 100 steps (~17ms)
    if (stepPosition % 100 == 0) {
      ArduinoOTA.handle();
      yield();
    }
    //Serial.println(" OpenGate line 126");
  }
  // Disable stepper after stepping completes
  digitalWrite(enablePin, HIGH);
  gateOpenState = true;
  gateCloseState = false;
  limitSwitchState = false;
  trace = "OPEN";
  displayStat();
  startTime = 0;
  moveState = false;
  setGateState(STATE_OPEN);
  //Serial.println(" OpenGate line 137");
}


// ***************************************************************************
void toolOn() {
  onTime= millis();
  //Serial.println ("toolOn line 177 ");
  const char* mqttTopic = BGtopic;
  Serial.println (" Tool ON line 187");
  digitalWrite(reedRelayPin, HIGH);
  digitalWrite(gateOn, HIGH);
  //Serial.println(" toolOn line 182");
  digitalWrite(enablePin, HIGH);
  gateOpenState = true;
  trace = "ON";
  displayStat();
  setGateState(STATE_OPEN);
  //Serial.println(" toolON Line # 17");
}

// ***************************************************************************
void toolOff() {
  const char* mqttTopic = BGtopic;
    toolRunTime = ((millis() -onTime)/3600000.0);
//    toolRunTime = ((millis() -onTime));
    Serial.print ("CF202  onTime= ");
    Serial.print (onTime);
    Serial.print (" Time= ");
    Serial.print (millis());    
    Serial.print (" toolRunTime= ");
  Serial.println (toolRunTime);
  logCycle();
  Serial.println(" toolOff line 207");
  //Serial.println(BGtopic);
  gateOpenState = false;
  setGateState(STATE_CLOSED);
  digitalWrite(reedRelayPin, LOW);
  digitalWrite(gateOn, LOW);
  trace = "OFF";
  displayStat();
}

// ***************************************************************************
void manualGateClose() {
  Serial.println(" manual gate Close 231");
  const char* mqttTopic = BGtopic;
  gateOpenState = false;
  setGateState(STATE_CLOSED);
  digitalWrite(reedRelayPin, LOW);
  digitalWrite(gateOn, LOW);
  trace = "Off";
  displayStat();
}

// ***************************************************************************
void manualGateOpen() {
  Serial.println ("manual gate open line 206 ");
  const char* mqttTopic = BGtopic;
  //Serial.println (" manual gate open line 208");
  digitalWrite(reedRelayPin, HIGH);
  digitalWrite(gateOn, HIGH);
  //Serial.println(" manual gate open line 211");
  digitalWrite(enablePin, HIGH);
  gateOpenState = true;
  trace = "Open";
  displayStat();
  setGateState(STATE_OPEN);
  //Serial.println(" manual gate open 218");
}


// ***************************************************************************
