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