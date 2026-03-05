#include "commTools.h"
#include "globals.h"

void WiFiConnect() {
  Serial.println("\n=== WiFi Connection Debug ===");
  Serial.print("Attempting SSID: ");
  Serial.println(ssid);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  // Wait up to 10 seconds for connection
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 40) {
    delay(250);
    Serial.print(".");
    attempts++;
  }
  
  Serial.println();
  Serial.print("WiFi Status: ");
  Serial.println(WiFi.status()); // 3 = WL_CONNECTED
  
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Primary network failed, trying backup...");
    Serial.print("Attempting SSID: ");
    Serial.println(ssidAlt);
    
    ssid = ssidAlt;
    password = passwordAlt;
    mqtt_server = mqtt_serverAlt;
    
    WiFi.begin(ssid, password);
    attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 40) {
      delay(250);
      Serial.print(".");
      attempts++;
    }
    Serial.println();
    Serial.print("WiFi Status: ");
    Serial.println(WiFi.status());
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("WiFi Connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("ERROR: Failed to connect to WiFi - check SSID/password in settings.cpp");
    Serial.print("Last Status Code: ");
    Serial.println(WiFi.status());
  }
}
