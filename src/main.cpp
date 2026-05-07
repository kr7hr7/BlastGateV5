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
  printInputStates();
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
  
  }
// ***************************************************************************
  if (gateType == "X")
  {
    // Gate type X is servo-only; avoid checkSwitchState side-effects that can
    // drive shared GPIOs and interfere with servo PWM on some board maps.
    servoControllerLoop();
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
          setGateState(STATE_OPEN);
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

  if (gateType == "L")
  {
    gateTypeL_Tasks();
  }
}

// put function definitions here:
int myFunction(int x, int y)
{
  return x + y;
}
