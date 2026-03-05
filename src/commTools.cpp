#include "commTools.h"
#include "globals.h"

void WiFiConnect() {
    WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  delay(4000);
  //Serial.print("IP address: ");
  if (WiFi.status() != WL_CONNECTED) {
    ssid = ssidAlt;
    password = passwordAlt;
    mqtt_server = mqtt_serverAlt;
    unsigned long startAttempTime = millis();
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    Serial.print("Attempting to connect WiFi  ");
    delay(2000);
  }
  Serial.println(WiFi.localIP());
}
