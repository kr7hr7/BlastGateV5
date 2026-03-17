#include "ControllerTasks.h"
#include <math.h>
#include "globals.h"
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

      dtostrf(inchesH2O, 4, 1, buffer);
      // Publish the message
      client.publish("home/sensors/static_pressure", buffer);
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