#include "utilities.h"
#include "globals.h"
#include <ESPmDNS.h>

// Publish gate state only when it changes unless force=true.
bool publishGateState(bool force) {
  static GateState lastPublishedState = STATE_UNKNOWN;
  static bool hasPublished = false;

  if (!client.connected()) {
    return false;
  }

  if (force || !hasPublished || gateState != lastPublishedState) {
    bool ok = client.publish(BGtopic, gateStateToString(gateState), false);
    if (ok) {
      lastPublishedState = gateState;
      hasPublished = true;
    }
    return ok;
  }

  return true;
}

//----------------------------------------
void prepNames() {

  //           --   BG Topic  --
  String BGtopicString = "home-assistant/blastgates/M" + machineID + "/state";
  {
    size_t len = BGtopicString.length();
    if (len >= sizeof(BGtopic)) len = sizeof(BGtopic) - 1;
    for (size_t i = 0; i < len; i++) {
      BGtopic[i] = BGtopicString[i];
    }
    BGtopic[len] = '\0';  // Null-terminate the char array
  }
  //Serial.println(BGtopic);

  //           --   Availability Topic  --
  String availabilityTopicString = "home-assistant/blastgates/M" + machineID + "/availability";
  {
    size_t len = availabilityTopicString.length();
    if (len >= sizeof(availabilityTopic)) len = sizeof(availabilityTopic) - 1;
    for (size_t i = 0; i < len; i++) {
      availabilityTopic[i] = availabilityTopicString[i];
    }
    availabilityTopic[len] = '\0';  // Null-terminate the char array
  }
  //Serial.println(availabilityTopic);

  //        --  Setup Identification Board ID + M + Machine ID  -----

  machineIDstring = String(machineID);
  setupID = (boardID + "M" + machineID);
  //Serial.print("setupID= ");
  //Serial.print(setupID);

  //tools();
  String gateNameString;
  //int toolInt;

  Serial.print("  Tool Name= ");
  Serial.print(toolString);

  // Build OTA hostname candidate and sanitize to valid mDNS characters.
  // mDNS host labels should contain only letters, digits, or hyphen.
  String baseName = setupID + "_" + toolString;
  String safeName = "";
  for (size_t i = 0; i < baseName.length(); i++) {
    char c = baseName[i];
    bool isLower = (c >= 'a' && c <= 'z');
    bool isUpper = (c >= 'A' && c <= 'Z');
    bool isDigit = (c >= '0' && c <= '9');
    if (isLower || isUpper || isDigit || c == '-') {
      safeName += c;
    } else {
      safeName += '-';
    }
  }
  if (safeName.length() == 0) {
    safeName = "blastgate";
  }
  Serial.print("  OTA gateName = ");
  Serial.println(safeName);

  // append EEPROM flag if present
  if (EEPROM.read(3) == 200) {
    safeName += "-F";
  }
  gateNameString = safeName;
  // copy final string out-of-band
  gateNameString.toCharArray(gateName, sizeof(gateName));

  Serial.print("  gateNameString = ");
  Serial.println(gateNameString);


  verString = String(ver ? ver : "");

  //        --  Setup MQTT Logon Variables  -----

  //convert machineID to char variable for MQTT logon

  mIDlen = machineID.length();
  //machineID.toCharArray(machineIDChar, (mIDlen + 1));
  machineID.toCharArray(machineIDChar, 3);
  mIDstring = "M" + machineID;
  //mIDstring.toCharArray(mID, sizeof(mID));
  mIDstring.toCharArray(mID, 4);

  strcpy(cycleTopic, mID);
  strcat(cycleTopic, "CycleData");

  rssiTopic = String(mID) + "RSSI";


  const char* mqttName = mID;
  const char* mqttUser = machineIDChar;
  const char* mqttTopic = BGtopic;
}
//--------------------------------------------------------------------------
void reportStaticPressue() {

  Serial.print("reportStaticPressue:  ");
  //client.publish ("home-assistant/blastgates", "Checking In");
  client.publish(availabilityTopic, "online");
  publishGateState();
  Serial.println(sensorIn);
}

//--------------------------------------------------------------------------
void keepMqttAlive(void* parameters) {

  const char* mqttName = mID;
  const char* mqttUser = machineIDChar;

  for (;;) {

    //Serial.println("keepMqttAlive Line 72");
    client.loop();  // CRITICAL: Must call to maintain MQTT connection
    if (client.connected()) {
      //Serial.println(" keepMqttAlive  MQTT Connected");
    }
    if (!client.connected()) {
      Serial.println("MQTT Not Connected!  Attempting to connect to broker");
      client.connect(mqttName, mqttUser, "LetBGin!", availabilityTopic, 0, true, "offline");
      delay(2000);

      if (WiFi.status() == WL_CONNECTED) {
        Serial.println("MQTT Re-Connected");
        publishGateState(true);
        displayStat();
      }
    }
    vTaskDelay(60000 / portTICK_PERIOD_MS);  // wait 1 minutes before checking the status again
  }
}


//------------------------------------------------------------------
void MQTTconnect() {
  const char* mqttName = mID;
  const char* mqttUser = machineIDChar;
  client.connect(mqttName, mqttUser, "LetBGin!", availabilityTopic, 0, true, "offline");
  delay(2000);
  if (client.connected()) {
    Serial.println("MQTT Connected!");
    client.publish(availabilityTopic, "online");
    publishGateState(true);
  }

  if (!client.connected()) {
    Serial.println("MQTT  NOT  Connected!");
  }
}

//------------------------------------------------------------------------------
void checkSwitchState() {

  ArduinoOTA.handle();
  if (digitalRead(limitSwitchPin) == HIGH) {
    limitSwitchState = false;
    gateCloseState = false;
  }

  else {
    limitSwitchState = true;
    gateOpenState = false;
    gateCloseState = true;
    digitalWrite(greenLEDpin, LOW);
    digitalWrite(redLEDpin, HIGH);
  }
}
//--------------------------------------------------------------------------
void pingBroker(void* parameters) {

  for (;;) {
    client.publish(availabilityTopic, "online");
    publishGateState();
    client.publish("Line", db, false);
    wifiDB = WiFi.RSSI();
    String stringWifiDB = String (wifiDB);
    char charWifiDB[20];
    itoa(wifiDB, charWifiDB, 10);
    char charRssiTopic[20];
    rssiTopic.toCharArray(charRssiTopic, 20);
    client.publish(charRssiTopic, charWifiDB, false);
    Serial.print (charRssiTopic);
    Serial.print("  Signal Strength=  ");
    Serial.println(charWifiDB);
    vTaskDelay(60000 / portTICK_PERIOD_MS);  //wait one minute before checking the status again
  }
}
//--------------------------------------------------------------------------
void keepWiFiAlive(void* parameters) {

  for (;;) {
    //Serial.print("keepWiFiAlive");
    if (WiFi.status() == WL_CONNECTED) {
      // Serial.println("Connected");
    }
    if (WiFi.status() != WL_CONNECTED) {
      unsigned long startAttempTime = millis();
      WiFi.mode(WIFI_STA);
      WiFi.begin(ssid, password);
      Serial.println("Attempting to connect");
      delay(2000);

      if (WiFi.status() == WL_CONNECTED) {
        Serial.print("WiFi connected  IP Address= ");
        Serial.print (WiFi.localIP());
        Serial.print("  MAC Address= ");
        Serial.println(WiFi.macAddress());

        displayStat();
      }
    }
    client.publish(availabilityTopic, "online");
    vTaskDelay(120000 / portTICK_PERIOD_MS);  // wait 2 minutes before checking the status again
  }
}
//------------------------------------------------------------------------------
void printStat() {
  Serial.print(loopCount);
  Serial.print("   ");
  Serial.print(trace);
  Serial.print("   ");
  Serial.print(sensorIn);
  Serial.print("   ");
  //Serial.print(stepPosition);
  Serial.print("   ");
  Serial.print(WiFi.localIP());
  Serial.print("   ");
  Serial.print(millis());
  Serial.print("   ");
  Serial.print((currentTime));
  Serial.print("   ");
  Serial.print((startTime));
  Serial.print("   ");
  x = currentTime - startTime;
  Serial.print(closeTime);
  Serial.println("   ");
}
//------------------------------------------------------------------------------

void displayStat() {
  ArduinoOTA.handle();
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(3);
  display.setCursor(0, 0);
  display.print(trace);
  display.print("        ");
  display.setTextSize(1);
  display.setCursor(0, 33);
  display.print(setupID);
  display.setCursor(50, 33);
  display.print(ver);
  display.setCursor(120, 33);
  display.print(gateType);
  display.setCursor(0, 43);
  display.setTextWrap(false);
  String toolLine = toolString;
  if (toolLine.length() > 21) {
    toolLine = toolLine.substring(0, 21);
  }
  display.print(toolLine);
  // Keep right-bottom area free for sensorIn overlay in main loop.
  display.setCursor(0, 53);
  //display.print(gateDelaySeconds);
  //display.setCursor(0, 53);
  display.print(WiFi.localIP());
  display.setTextWrap(true);
  // Restore cached sensor overlay after full-screen clear without changing its value.
  redrawSensorInOnOled(false);
  display.display();
}

//----------------------------------------

void errorState() {
  //delay(3000);
  Serial.println("errorState line 230");
  if (eCode == 1) {
    gateState = STATE_ERROR_1;
  } else if (eCode == 2) {
    gateState = STATE_ERROR_2;
  } else if (eCode == 3) {
    gateState = STATE_ERROR_3;
  } else {
    gateState = STATE_UNKNOWN;
  }
  trace = "Error";
  publishGateState();
  OTA();
  Serial.println(" errorState Line232 ");
  //Serial.print(" LimitSwitch = ");
  //Serial.println (digitalRead(limitSwitchPin));
  digitalWrite(enablePin, HIGH);
  //Serial.print(" LimitSwitch = ");
  //Serial.println (digitalRead(limitSwitchPin));
  client.publish(availabilityTopic, "offline");
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(3);
  display.setCursor(20, 0);
  display.print("Error");
  display.setCursor(0, 33);
  display.print("Code");
  display.setCursor(100, 33);
  display.print(eCode);
  display.display();
  for (;;) {
    delay(500);
    dbNew = "U252";
    ArduinoOTA.handle();

  }                                                         /*
       Code 1 = excessive missed steps: gate may be blocked
       Code 2 = Board assignment does not match EEPROM
       Code 3 = Gate assignment error:  Check settings
       Code 5 = close/homing timeout waiting for limit switch
*/
  Serial.println("errorState Line 257");
}

//----------------------------------------

void setupError() {
  WiFiConnect();
  delay(3000);
  OTA();
  digitalWrite(enablePin, HIGH);
  client.publish(availabilityTopic, "offline");
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(3);
  display.setCursor(0, 5);
  display.print("Setup");
  display.setCursor(0, 35);
  display.print("Error");
  display.display();
  for (;;) {
    dbNew = "U280";
    delay(500);
    ArduinoOTA.handle();
    if (linkPin != linkPin)  {
      break;
    }
  }
}

//----------------------------------------
void writeToBootLog() {
  Serial.print("     write to boot log line 301 ");

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("write to boot log");
    //Serial.println("");
    Serial.println(" write to boot log line 277");
    static bool flag = false;
    //Serial.println("WiFi Connected!");
    urlFinal = GOOGLE_SCRIPT_Boot + "gate=M" + machineID + "&board=" + boardID + "&ipa=" + ipa + "&ver=" + verString + "&gateType=" + gateType + "&mac=" + mac + "&wifiDB=" + wifiDB;
    //Serial.println("");
    Serial.println(urlFinal);
    Serial.println("POST data to spreadsheet:  ");
    HTTPClient http;
    http.begin(urlFinal.c_str());
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    http.setTimeout(5000);  // 5 second timeout to prevent boot hanging
    int httpCode = http.GET();
    Serial.print("httpCode=  ");
    Serial.println (httpCode);

    Serial.println(" write to boot log line 326");

  }
  else {
    Serial.println ("WiFi not connected!");
  }
}


//------------------------------------------------------------------

void reconnect() {
  Serial.print("Attempting MQTT connection...");
  String clientId = "ESP32Client-";
  clientId += String(random(0xffff), HEX);
  // Attempt to connect
  if (client.connect(clientId.c_str())) {
    // Serial.println("connected");
  } else {
    Serial.print("failed, rc=");
    Serial.println(client.state());
    delay(5000);
  }
}


//------------------------------------------------------------------
void OTA() {
  if (otaOn == false) {
    Serial.println("OTA line 342");
    ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else  // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type);
    })
    .onEnd([]() {
      Serial.println("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });
    Serial.println("OTA line 368");
    //Serial.println (gateName);
    ArduinoOTA.setPort(3232);
    ArduinoOTA.setHostname(gateName);
    if (!MDNS.begin(gateName)) {
      Serial.println("mDNS start failed");
    } else {
      // Required for discover_ota.py which scans _arduino._tcp.local.
      MDNS.addService("arduino", "tcp", 3232);
    }
    ArduinoOTA.begin();
    otaOn = true;
  }
}
void logCycle() {
  Serial.print ("U384  toolRunTime= ");
  Serial.println (toolRunTime);
  Serial.println (cycleTopic);
  //  int randomInt = random(1, 500);
  // float temp = randomInt / 10000.0;
  char temp1 [15] ;
  //  char msg [10] ;
  dtostrf( toolRunTime, 12, 4, temp1);
  //  dtostrf( temp, 12, 4, temp1);

  client.publish(cycleTopic , temp1);
}
