#include "ControllerTasks.h"
#include <math.h>

void S_StaticPressureTasks() {
  char payload[64];

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
        inchesH2O_str = String(staticPressure,0);
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
        snprintf(payload, sizeof(payload), "%d", analogRead(ANALOG_PIN_IN));
        // Publish the message
        client.publish("home/sensors/static_pressure", payload);
        //Serial.print("MQTT Message Sent: ");
        Serial.println(payload);
      }
    }
    delay(interval);
  } 
