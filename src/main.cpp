#include <Arduino.h>
#include "globals.h"
#include <ArduinoOTA.h>
#include "setupTasks.h"
#include "ControllerTasks.h"

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

static void drawSensorInOnOled(bool clearField, bool pushDisplay)
{
  if (!oledReady)
  {
    return;
  }

  const int fieldX = 98;
  const int fieldY = 50;
  const int fieldW = 30;
  const int charW = 6; // default font width at text size 1 (5px + 1px spacing)

  if (clearField)
  {
    // Clear only the sensor field area before redraw to avoid stale digits.
    display.fillRect(fieldX, fieldY, fieldW, 8, BLACK);
  }

  String sensorText = String(sensorInDisplayValue);
  int textX = fieldX + fieldW - (sensorText.length() * charW);
  if (textX < fieldX)
  {
    textX = fieldX;
  }

  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(textX, fieldY);
  display.print(sensorText);
  if (pushDisplay)
  {
    display.display();
  }
}

void redrawSensorInOnOled(bool pushDisplay)
{
  if (!sensorInDisplayInitialized)
  {
    sensorInDisplayValue = sensorIn;
    sensorInDisplayInitialized = true;
  }

  // Full-screen redraw paths already clear the OLED, so no field clear is needed here.
  drawSensorInOnOled(false, pushDisplay);
}

void updateSensorInOnOled(bool forceRedraw, bool pushDisplay)
{
  bool updateTriggered = false;
  const unsigned long now = millis();

  if (!sensorInDisplayInitialized)
  {
    sensorInDisplayValue = sensorIn;
    sensorInDisplayInitialized = true;
    sensorInCandidateActive = false;
    updateTriggered = true;
  }
  else
  {
    const int deltaFromDisplayed = abs(sensorIn - sensorInDisplayValue);
    if (deltaFromDisplayed >= sensorInDisplayDelta)
    {
      if (!sensorInCandidateActive)
      {
        sensorInCandidateActive = true;
        sensorInCandidateValue = sensorIn;
        sensorInCandidateSinceMs = now;
      }
      else
      {
        // If the candidate drifts significantly, restart confirmation timing.
        if (abs(sensorIn - sensorInCandidateValue) >= sensorInDisplayDelta)
        {
          sensorInCandidateValue = sensorIn;
          sensorInCandidateSinceMs = now;
        }
        else if (now - sensorInCandidateSinceMs >= sensorInDisplayConfirmMs)
        {
          sensorInDisplayValue = sensorIn;
          sensorInCandidateActive = false;
          updateTriggered = true;
        }
      }
    }
    else
    {
      sensorInCandidateActive = false;
    }
  }

  if (!updateTriggered && !forceRedraw)
  {
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
  client.loop(); // CRITICAL: Must call to maintain MQTT connection and prevent blocking
  updateSensorInOnOled();

  // ***************************************************************************
  if (gateType == "S")
  {
     S_StaticPressureTasks();
  }

  // ***************************************************************************
  if (gateType == "M")
  {
    manualGateTasks();
  }

  // ***************************************************************************

  if (gateType == "P")
  {
    // Serial.println (" P Loop line # 92");
    ArduinoOTA.handle();
    sensorIn = (analogRead(ANALOG_PIN_IN));
    // Serial.print (" P Loop Line #  78 sensor in = ");
    // Serial.println (sensorIn);

    if (sensorIn < trigger)
    {
      if (gateOpenState == true)
      {
        digitalWrite(gateOn, LOW);
        toolOff();
      }
    }
    if (sensorIn > (trigger + triggerDelta))
    {
      delay(200);
      sensorIn = (analogRead(ANALOG_PIN_IN));
      if ((sensorIn > (trigger + triggerDelta)))
      {
        startTime = 0;
        if (gateOpenState != true)
        {
          digitalWrite(gateOn, HIGH);
          digitalWrite(limitSwitchPin, LOW);
          toolOn();
        }
        else
        {
          trace = "ON";
          if (startTime != 0)
          {
            setGateState(STATE_OPEN);
            // Serial.println(BGtopic);
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
    // trace = "";
    // displayStat();
    // displayFlag = true;
    //}
    // Serial.println(sensorIn);
    // delay(holdTime);
  }

  if (gateType == "X")
  {
    // Serial.println ("Main Line # 236");
    checkSwitchState();
    servoControllerLoop();
  }

  // ***************************************************************************
  if ((gateType == "A") || (gateType == "B") || (gateType == "C") || (gateType == "D"))
  {
    //   Serial.println ("Loop  line 132 ");
    static unsigned long belowTriggerStart = 0;
    static unsigned long notClosedStart = 0;
    static bool closeLatchedForLowSignal = false;
    const unsigned long lowSignalCloseDelayMs = 1000;
    const unsigned long notClosedDebounceMs = (unsigned long)((closeSwitchDebounceMs < 25) ? 25 : closeSwitchDebounceMs);

    checkSwitchState();
    sensorIn = (analogRead(ANALOG_PIN_IN));
    if (sensorIn > (trigger + triggerDelta))
    {
      belowTriggerStart = 0;
      notClosedStart = 0;
      closeLatchedForLowSignal = false;
      delay(200);
      sensorIn = (analogRead(ANALOG_PIN_IN));
      if ((sensorIn > (trigger + triggerDelta)))
      {
        startTime = 0;
        dbNew = "L140";
        if (gateOpenState != true)
        {
          dbNew = "L142";
          openGate();
        }

        //  If tool is turned on during countdown to close
        if (gateOpenState == true)
        {
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
          setGateState(STATE_OPEN);
        }
      }
    }

    if (sensorIn < trigger)
    {
      if (belowTriggerStart == 0)
      {
        belowTriggerStart = millis();
        notClosedStart = 0;
        closeLatchedForLowSignal = false;
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

      dbNew = "L166";
      const bool gateIndicatesOpen = (gateOpenState == true) || (gateState == STATE_OPEN) || (gateState == STATE_OPENING);
      const bool closeInProgress = (startTime != 0) || (gateState == STATE_CLOSING);
      // If software state drifts, rely on a debounced limit switch "not closed" signal.
      const bool gatePhysicallyNotClosed = (notClosedStart != 0) && ((millis() - notClosedStart) >= notClosedDebounceMs);
      const bool eligibleToStartClose = (gateIndicatesOpen || gatePhysicallyNotClosed) &&
                                        (millis() - belowTriggerStart >= lowSignalCloseDelayMs) &&
                                        (closeLatchedForLowSignal == false);
      if (closeInProgress || eligibleToStartClose)
      {
        if (eligibleToStartClose)
        {
          closeLatchedForLowSignal = true;
        }
        closeGate();
      }

      // Once a gate is confirmed closed during this low-signal period, suppress any retrigger.
      if ((gateState == STATE_CLOSED) && (gateCloseState == true))
      {
        closeLatchedForLowSignal = true;
      }

      delay(holdTime);
    }
    else
    {
      belowTriggerStart = 0;
      notClosedStart = 0;
      closeLatchedForLowSignal = false;
    }
    ArduinoOTA.handle();
  }
}

// put function definitions here:
int myFunction(int x, int y)
{
  return x + y;
}
