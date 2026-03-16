#include "controlFunctions.h"
#include "globals.h"

// ***************************************************************************
void homePosition() {
  checkSwitchState();
  digitalWrite(enablePin, LOW);
  digitalWrite(dirPin, HIGH);  //turn clockwise
  if (rotation == true) {
    digitalWrite(dirPin, LOW);
  }
  trace = "Closing";
  displayStat();
  setGateState(STATE_CLOSING);

  stepPosition = 0;
  const unsigned long homeStartTime = millis();
  const unsigned long homeStepBudget = (unsigned long)(fullRunSteps + maxMissedSteps);
  // Time-based failsafe for unexpected stalls in close/homing state.
  const unsigned long maxHomeDurationMs = (homeStepBudget * (unsigned long)(delayTime * 2 + 350) / 1000UL) + 3000UL;
  unsigned long lastYield = 0;
  while ((digitalRead(limitSwitchPin) == HIGH)) {
    dbNew="CF16";
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(delayTime);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(delayTime);
    stepPosition++;
    
    // Yield to background tasks every 100 steps (~17ms)
    if (stepPosition % 100 == 0) {
      ArduinoOTA.handle();
      yield();
    }

    if ((millis() - homeStartTime) > maxHomeDurationMs) {
      Serial.println("Home timeout waiting for limit switch");
      setGateState(STATE_UNKNOWN);
      eCode = 5; // Error code 5: close/homing timeout
      errorState();
    }
    
    if (stepPosition >= (fullRunSteps + maxMissedSteps)) {
      Serial.println("Home line 22 ");
      setGateState(STATE_UNKNOWN);
      
      eCode=1; //                                         Error code 1
      errorState();
    }
  }

  setGateState(STATE_CLOSED);
  digitalWrite(enablePin, HIGH);
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
  Serial.println(" Close Gate");
  if (gateType == "A" || gateType == "B") {
    digitalWrite(reedRelayPin, LOW);
  }
  Serial.println(BGtopic);
  moveState = true;
  setGateState(STATE_CLOSING);

  if (startTime == 0) {
    startTime = millis();
    toolRunTime = ((millis() -onTime)/3600000.0);
    runTime = int((startTime - gateOpenTime) / 1000);
    closeTime = closeDelayTime + startTime;
    /*
        Serial.print("Close_Gate Line 448  currentTime=  ");
        Serial.print(currentTime);
        Serial.print(" startTime=  ");
        Serial.print(startTime);
        Serial.print("  closeDelayTime=  ");
        Serial.print(closeDelayTime);
        Serial.print("  closeTime=  ");
        Serial.println(closeTime);
    */
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
  /*
    Serial.print(" Close_Gate Line 476  currentTime=  ");
    Serial.print(currentTime);
    Serial.print(" startTime=  ");
    Serial.print(startTime);
    Serial.print("  closeDelayTime=  ");
    Serial.print(closeDelayTime);
    Serial.print("  closeTime=  ");
    Serial.println(closeTime);
  */
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
  moveState = true;
  //Serial.println (" Open Gate");
  gateOpenTime = millis();
  onTime= millis();
  
  sensor = sensorIn;

  digitalWrite(reedRelayPin, HIGH);
  digitalWrite(gateOn, HIGH);
  setGateState(STATE_OPENING);
  //Serial.println(BGtopic);
  //Serial.println(" OpenGate line 102");
  homePosition();
  //Serial.println(" OpenGate line 104");
  setGateState(STATE_OPENING);
  //Serial.println(BGtopic);

  if (oledReady) {
    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(3);
    display.setCursor(0, 20);
    display.print("Ready");
    display.display();
  }
  //Serial.println(" OpenGate line 115");
  digitalWrite(enablePin, LOW);
  digitalWrite(dirPin, LOW);  // turn counterClockwise
  if (rotation == true) {
    digitalWrite(dirPin, HIGH);
  }
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
  digitalWrite(enablePin, HIGH);
  gateOpenState = true;
  trace = "OPEN";
  displayStat();
  startTime = 0;
  moveState = false;
  digitalWrite(enablePin, HIGH);
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
