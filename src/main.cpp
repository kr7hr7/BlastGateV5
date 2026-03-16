#include <Arduino.h>
#include "globals.h"
#include <ArduinoOTA.h>
#include "setupTasks.h"

// put function declarations here:
int myFunction(int, int);

// Refresh the sensor overlay only when the displayed value changes meaningfully.
static int sensorInDisplayValue = 0;
static bool sensorInDisplayInitialized = false;
static const int sensorInDisplayDelta = 25;
static bool sensorInCandidateActive = false;
static int sensorInCandidateValue = 0;
static unsigned long sensorInCandidateSinceMs = 0;
static const unsigned long sensorInDisplayConfirmMs = 250;

static void drawSensorInOnOled(bool clearField, bool pushDisplay) {
  if (!oledReady) {
    return;
  }

  const int fieldX = 98;
  const int fieldY = 50;
  const int fieldW = 30;
  const int charW = 6; // default font width at text size 1 (5px + 1px spacing)

  if (clearField) {
    // Clear only the sensor field area before redraw to avoid stale digits.
    display.fillRect(fieldX, fieldY, fieldW, 8, BLACK);
  }

  String sensorText = String(sensorInDisplayValue);
  int textX = fieldX + fieldW - (sensorText.length() * charW);
  if (textX < fieldX) {
    textX = fieldX;
  }

  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(textX, fieldY);
  display.print(sensorText);
  if (pushDisplay) {
    display.display();
  }
}

void redrawSensorInOnOled(bool pushDisplay) {
  if (!sensorInDisplayInitialized) {
    sensorInDisplayValue = sensorIn;
    sensorInDisplayInitialized = true;
  }

  // Full-screen redraw paths already clear the OLED, so no field clear is needed here.
  drawSensorInOnOled(false, pushDisplay);
}

void updateSensorInOnOled(bool forceRedraw, bool pushDisplay) {
  bool updateTriggered = false;
  const unsigned long now = millis();

  if (!sensorInDisplayInitialized) {
    sensorInDisplayValue = sensorIn;
    sensorInDisplayInitialized = true;
    sensorInCandidateActive = false;
    updateTriggered = true;
  } else {
    const int deltaFromDisplayed = abs(sensorIn - sensorInDisplayValue);
    if (deltaFromDisplayed >= sensorInDisplayDelta) {
      if (!sensorInCandidateActive) {
        sensorInCandidateActive = true;
        sensorInCandidateValue = sensorIn;
        sensorInCandidateSinceMs = now;
      } else {
        // If the candidate drifts significantly, restart confirmation timing.
        if (abs(sensorIn - sensorInCandidateValue) >= sensorInDisplayDelta) {
          sensorInCandidateValue = sensorIn;
          sensorInCandidateSinceMs = now;
        } else if (now - sensorInCandidateSinceMs >= sensorInDisplayConfirmMs) {
          sensorInDisplayValue = sensorIn;
          sensorInCandidateActive = false;
          updateTriggered = true;
        }
      }
    } else {
      sensorInCandidateActive = false;
    }
  }

  if (!updateTriggered && !forceRedraw) {
    return;
  }

  drawSensorInOnOled(updateTriggered, pushDisplay);
}

void setup()
{
    setupTasks();
  servoControllerSetup();
}

void loop()
{
  // Serial.println ("Loop  line 3 ");
  ArduinoOTA.handle();
  client.loop();  // CRITICAL: Must call to maintain MQTT connection and prevent blocking
  updateSensorInOnOled();

  // ***************************************************************************
  if (gateType == "S") {
    staticPressure = (analogRead(ANALOG_PIN_IN));
    currentMillis = millis();
    if ((((fabs(staticPressure - lastSample)) >= staticDelta)) || ((currentMillis - lastMsgTime >= maxInterval))) {
      delay(200);
      staticPressure = (analogRead(ANALOG_PIN_IN));
      if ((((fabs(staticPressure - lastSample)) >= staticDelta)) || ((currentMillis - lastMsgTime >= maxInterval))) {
        lastMsgTime = currentMillis;
        inchesH2O = ((staticPressure - 775) / 221);
        inchesH2O = round(inchesH2O * 10) / 10.0;
        if (inchesH2O < .5) {
          inchesH2O = 0;
        }
        delta = staticPressure - lastSample;
        lastSample = staticPressure;
        inchesH2O_str = String(inchesH2O, 1);
        trace = inchesH2O_str;
        displayStat();
        /*
          Serial.print(staticPressure);
          Serial.print("  ");
          Serial.print(delta);
          Serial.print("  ");
          Serial.print(inchesH2O_str);
          Serial.print("  ");
        */
        dtostrf(inchesH2O, 4, 1, buffer);
        // Publish the message
        client.publish("home/sensors/static_pressure", buffer);
        //Serial.print("MQTT Message Sent: ");
        Serial.println(buffer);
      }
    }
    delay(interval);
  }

  // ***************************************************************************
  if (gateType == "M") {
    sensorIn = (analogRead(ANALOG_PIN_IN));
    sensorIn = 4095 - sensorIn;

    if (sensorIn > (2000)) {
      delay(200);
      if (gateOpenState != true) {
        digitalWrite(gateOn, HIGH);
        manualGateOpen();
      } else {
        trace = "ON";
        if (startTime != 0) {
          gateState = STATE_OPEN;
          publishGateState();
          Serial.println(BGtopic);
        }
        displayStat();
        startTime = 0;
      }
    }

    if (sensorIn < trigger) {
      if (gateOpenState == true) {
        digitalWrite(gateOn, LOW);
        manualGateClose();
      }
    }
  }

  // ***************************************************************************


  if (gateType == "P") {
   // Serial.println (" P Loop line # 92");
    ArduinoOTA.handle();
    sensorIn = (analogRead(ANALOG_PIN_IN));
    //Serial.print (" P Loop Line #  78 sensor in = ");
    //Serial.println (sensorIn);


    if (sensorIn < trigger) {
      if (gateOpenState == true) {
        digitalWrite(gateOn, LOW);
        toolOff();
      }
    }
    if (sensorIn > (trigger + triggerDelta)) {
      delay(200);
      sensorIn = (analogRead(ANALOG_PIN_IN));
      if ((sensorIn > (trigger + triggerDelta))) {
        startTime = 0;
        if (gateOpenState != true) {
          digitalWrite(gateOn, HIGH);
          digitalWrite(limitSwitchPin, LOW);
          toolOn();
        } else {
          trace = "ON";
          if (startTime != 0) {
            gateState = STATE_OPEN;
            publishGateState();
            //Serial.println(BGtopic);
          }
          displayStat();
          startTime = 0;
        }
      }
    }
    /*
      if (sensorIn < trigger) {
        if (gateOpenState == true)
        {
          digitalWrite(gateOn, LOW);
          toolOff();
        }
    */

    // if (displayFlag == false)
    //{
    //trace = "";
    //displayStat();
    //displayFlag = true;
    //}
    //Serial.println(sensorIn);
    //delay(holdTime);
  }


  if (gateType == "X") {
    servoControllerLoop();
  }

  // ***************************************************************************
  if ((gateType == "A") || (gateType == "B") || (gateType == "C") || (gateType == "D")) {
//   Serial.println ("Loop  line 132 ");
    static unsigned long belowTriggerStart = 0;
    const unsigned long lowSignalCloseDelayMs = 1000;

    checkSwitchState();
    sensorIn = (analogRead(ANALOG_PIN_IN));
    if (sensorIn > (trigger + triggerDelta)) {
      belowTriggerStart = 0;
      delay(200);
      sensorIn = (analogRead(ANALOG_PIN_IN));
      if ((sensorIn > (trigger + triggerDelta))) {
        startTime = 0;
        dbNew = "L140";
        if (gateOpenState != true) {
          dbNew = "L142";
          openGate();
        }

        //  If tool is turned on during countdown to close
        if (gateOpenState == true) {
          // Serial.println (sensorIn);
          dbNew = "L149";
          gateOpenTime = millis();
          sensor = sensorIn;
          digitalWrite(reedRelayPin, HIGH);
          // digitalWrite(reedRelayPinAlt, HIGH);
          gateOpenState = true;
          trace = "OPEN";
          displayStat();
          startTime = 0;
          digitalWrite(enablePin, HIGH);
          gateState = STATE_OPEN;
          publishGateState();
        }
      }
    }

    if (sensorIn < trigger) {
      if (belowTriggerStart == 0) {
        belowTriggerStart = millis();
      }

      dbNew = "L166";
      const bool gateIndicatesOpen = (gateOpenState == true) || (gateState == STATE_OPEN) || (gateState == STATE_OPENING);
      // If software state drifts, rely on limit switch to detect "not closed" and still close.
      const bool gatePhysicallyNotClosed = (gateCloseState == false);
      if ((gateIndicatesOpen || gatePhysicallyNotClosed) && (millis() - belowTriggerStart >= lowSignalCloseDelayMs)) {
        closeGate();
      }

      delay(holdTime);
    } else {
      belowTriggerStart = 0;
    }
    ArduinoOTA.handle();
  }
}


// put function definitions here:
int myFunction(int x, int y)
{
  return x + y;
}
