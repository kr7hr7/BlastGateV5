#include <Arduino.h>
#include "globals.h"
#include <ArduinoOTA.h>
#include "setupTasks.h"

// put function declarations here:
int myFunction(int, int);

void setup()
{
    setupTasks();
}

void loop()
{
  // Serial.println ("Loop  line 3 ");
  ArduinoOTA.handle();

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
          client.publish(BGtopic, gateStateToString(gateState));
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
            client.publish(BGtopic, gateStateToString(gateState));
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
    checkSwitchState();
    sensorIn = (analogRead(ANALOG_PIN_IN));
    // Serial.println (sensorIn);
    if (sensorIn > (trigger + triggerDelta)) {
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
          client.publish(BGtopic, gateStateToString(gateState), false);
        }
      }
    }

    if (sensorIn < trigger) {
      dbNew = "L166";
      if (gateOpenState == true) {
        closeGate();
      }

      delay(holdTime);
    }
    ArduinoOTA.handle();
  }
}


// put function definitions here:
int myFunction(int x, int y)
{
  return x + y;
}
