#include <Arduino.h>
#include "globals.h"
#include <ArduinoOTA.h>
#include "setupTasks.h"

// put function declarations here:
int myFunction(int, int);

// Non-blocking OLED refresh for sensor input once per second.
static unsigned long lastSensorValueUpdateMs = 0;
static unsigned long lastSensorFieldPaintMs = 0;
static int sensorInDisplayValue = 0;

static void updateSensorInOnOled() {
  const int fieldX = 98;
  const int fieldY = 50;
  const int fieldW = 30;
  const int charW = 6; // default font width at text size 1 (5px + 1px spacing)

  const unsigned long now = millis();
  // Update the displayed value once per second.
  if (now - lastSensorValueUpdateMs >= 1000UL) {
    lastSensorValueUpdateMs = now;
    sensorInDisplayValue = sensorIn;
  }

  // Repaint field faster so other screen draws do not leave it overwritten.
  if (now - lastSensorFieldPaintMs < 200UL) {
    return;
  }

  lastSensorFieldPaintMs = now;
  String sensorText = String(sensorInDisplayValue);
  int textX = fieldX + fieldW - (sensorText.length() * charW);
  if (textX < fieldX) {
    textX = fieldX;
  }

  // Clear only the sensor field area before redraw to avoid stale digits.
  display.fillRect(fieldX, fieldY, fieldW, 8, BLACK);
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(textX, fieldY);
  display.print(sensorText);
  display.display();
}

void setup()
{
    setupTasks();
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

  // ***************************************************************************
  if ((gateType == "A") || (gateType == "B") || (gateType == "C") || (gateType == "D")) {
//   Serial.println ("Loop  line 132 ");
    static unsigned long belowTriggerStart = 0;
    const unsigned long lowSignalCloseDelayMs = 1000;

    checkSwitchState();
    sensorIn = (analogRead(ANALOG_PIN_IN));
    Serial.println (sensorIn);
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
