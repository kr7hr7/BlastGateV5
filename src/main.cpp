#include <Arduino.h>
#include "globals.h"
#include <ArduinoOTA.h>

// put function declarations here:
int myFunction(int, int);

void setup()
{
  settings();
  delay(100);
  Serial.begin(115200);
  // Serial.println("Booting");

  Wire.begin();
  display.begin(SSD1306_SWITCHCAPVCC, 0X3C);
  trace = "Start";
  displayStat();

  delay(2000);
  trace = "WiFi";
  displayStat();
  WiFiConnect();
  mac = WiFi.macAddress();
  wifiDB = WiFi.RSSI();

  delay(2000);
  trace = "Board Config";
  displayStat();
  // Initialize EEPROM with defined size
  if (!EEPROM.begin(EEPROM_SIZE))
  {
    Serial.println("Failed to initialize EEPROM");
    return;
  }

  boardconfiguration();
  prepNames();
  dirPin = 4;
  trace = "Board Setup";
  displayStat();

  pinMode(ANALOG_PIN_IN, INPUT);
  pinMode(reedRelayPin, OUTPUT);
  pinMode(greenLEDpin, OUTPUT);
  pinMode(redLEDpin, OUTPUT);
  pinMode(enablePin, OUTPUT);
  pinMode(limitSwitchPin, INPUT_PULLUP);
  pinMode(gateOn, OUTPUT);
  pinMode(gateOff, OUTPUT);
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  pinMode(linkPin, INPUT_PULLUP);
  digitalWrite(greenLEDpin, HIGH);
  digitalWrite(redLEDpin, HIGH);
  digitalWrite(reedRelayPin, LOW);
  digitalWrite(enablePin, HIGH);
  digitalWrite(linkPin, HIGH);
  digitalWrite(gateOn, LOW);
  digitalWrite(gateOff, LOW);

  trace = "Booting";
  displayStat();

  closeDelayTime = 1000 * gateDelaySeconds;
  client.setKeepAlive(keepAlive);
  client.setServer(mqtt_server, 1883);

  MQTTconnect();
  trace = "MQTT";
  displayStat();

  runTime = 0;
  ipa = WiFi.localIP().toString() + "....Boot";
  ipa = WiFi.localIP().toString();

  Serial.print("  MAC Address= ");
  Serial.println(WiFi.macAddress());

  xTaskCreatePinnedToCore(
      pingBroker,
      "Ping Mosquitto Broker",
      5000,
      NULL,
      1,
      NULL,
      CONFIG_ARDUINO_RUNNING_CORE);

  xTaskCreatePinnedToCore(
      keepWiFiAlive,
      "Keep WiFi Alive",
      5000,
      NULL,
      3,
      NULL,
      CONFIG_ARDUINO_RUNNING_CORE);

  xTaskCreatePinnedToCore(
      keepMqttAlive,
      "Keep MQTT Alive",
      5000,
      NULL,
      4,
      NULL,
      CONFIG_ARDUINO_RUNNING_CORE);

  // Serial.println("Setup line 151");

  Serial.println("");
  Serial.print("Gate Type           = ");
  Serial.println(gateType);
  Serial.println("");

  Serial.print("BoardIdByte         = ");
  Serial.println(boardIdByte);

  Serial.print("Board ID            = ");
  Serial.println(boardID);

  Serial.print("Board Version       = ");
  Serial.println(boardVer);

  Serial.print("Board Configuration = ");
  Serial.println(boardConfig);

  // Serial.print("flashed??           = ");
  // Serial.println(flash);

  Serial.print("ANALOG_PIN_IN       = ");
  Serial.println(ANALOG_PIN_IN);

  Serial.print("limitSwitchPin      = ");
  Serial.println(limitSwitchPin);

  Serial.print("reedRelayPin        = ");
  Serial.println(reedRelayPin);

  Serial.print("greenLEDpin         = ");
  Serial.println(greenLEDpin);

  Serial.print("redLEDpin           = ");
  Serial.println(redLEDpin);

  Serial.print("gateOn              = ");
  Serial.println(gateOn);

  Serial.print("gateOff             = ");
  Serial.println(gateOff);

  Serial.print("stepPin             = ");
  Serial.println(stepPin);

  Serial.print("dirPin              = ");
  Serial.println(dirPin);

  Serial.print("enablePin           = ");
  Serial.println(enablePin);
  Serial.println("");
  Serial.println(WiFi.localIP());
  Serial.println("");
  Serial.print(" Setup line 197 Gate Type           = ");
  Serial.println(gateType);
  Serial.println("");

  IPAddress ip = WiFi.localIP();

  int currentIP = ip[2]; //  If at home third octet of the ip address will be either 4 or 5
  if (currentIP <= 5)
  { //  Set the steps to that of a 4" gate
    fullRunSteps = 18000;
  }
  // if (currentIP > 5) {    //  If the controller is connected to the shop LAN

  writeToBootLog();
  Serial.println("Setup Line 157");
  //}

  Serial.println("");
  Serial.println("Setup Line 185");
  OTA();
  ArduinoOTA.handle();

  /*
    -------------------------------------
    Initialize
    -------------------------------------*/
  Serial.println("");
  Serial.print(" Setup line 205 Gate Type           = ");
  Serial.println(gateType);
  Serial.println("");

  digitalWrite(reedRelayPin, LOW);

  Serial.println("");
  Serial.print(" Setup line 212 Gate Type           = ");
  Serial.println(gateType);
  Serial.println("");

  if ((gateType != "A") && (gateType != "B") && (gateType != "C") && (gateType != "D") && (gateType != "S") && (gateType != "P") && (gateType != "M"))
  {

    Serial.println("");
    Serial.print("Gate Type           = ");
    Serial.println(gateType);
    Serial.println("");
    Serial.print("ERROR  Check gateType  ");
    Serial.println(gateType);
    eCode = 3;
    errorState();
  }

  const char *mqttTopic = BGtopic;
  client.publish(availabilityTopic, "online");

  // Serial.println ("Setup line 160 ");

  //*****************************************
  if (gateType == "S")
  {
    lastMsgTime = 0;
    lastSample = 0;
    return;
  }

  Serial.println("Setup  line line 216");

  //*****************************************
  if (gateType == "P")
  {
    lastMsgTime = 0;
    lastSample = 0;
    sensorIn = (analogRead(ANALOG_PIN_IN));

    if (sensorIn < trigger)
    {
      toolOff();
    }
    else
    {
      trace = "ON";
      toolOn();
    }
    return;
  }
  Serial.println("Setup  line line 232");

  //*****************************************
  if (gateType == "M")
  {
    sensorIn = (analogRead(ANALOG_PIN_IN));
    sensorIn = 4095 - sensorIn;
    // Serial.println (sensorIn);
    if (sensorIn > (trigger + triggerDelta))
    {
      // openGate();
      trace = "On";
      displayStat();
    }
    if (sensorIn < trigger)
    {
      // homePosition();
      trace = "Off";
      gateOpenTime = 0;
      displayStat();
      gateState = "closed";
      client.publish(BGtopic, gateState, false);
    }
    return;
  }

  Serial.println("Setup  line line 255 ");

  //*****************************************
  //        All Other Controllers
  Serial.println("Setup  line line 269");

  sensorIn = (analogRead(ANALOG_PIN_IN));

  Serial.println("Setup  line line 245");
  if (sensorIn > (trigger + triggerDelta))
  {
    Serial.println("Setup  line line 247");
    openGate();

    Serial.println("Setup  line line 250");
    trace = "OPEN";
    displayStat();
  }
  if (sensorIn < trigger)
  {

    Serial.println("Setup line 256");

    homePosition();
    trace = "CLOSED";
    gateOpenTime = 0;
    displayStat();
    gateState = "closed";
    client.publish(BGtopic, gateState, false);
  }
  Serial.print("Setup line 263");
  db = "Setup 275 ";
  client.publish(mID, db, false);
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
          gateState = "open";
          client.publish(BGtopic, gateState);
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
            gateState = "open";
            client.publish(BGtopic, gateState);
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
          gateState = "open";
          client.publish(BGtopic, gateState, false);
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