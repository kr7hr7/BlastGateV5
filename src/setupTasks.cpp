#include "globals.h"

void setupTasks()
{
  settings();
  delay(100);
  Serial.begin(115200);
  // Serial.println("Booting");

  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  Wire.setClock(100000);
  Wire.setTimeOut(20);

  bool oledOk = display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  if (!oledOk) {
    oledOk = display.begin(SSD1306_SWITCHCAPVCC, 0x3D);
  }
  if (!oledOk) {
    Serial.println("OLED init failed");
  } else {
    display.ssd1306_command(SSD1306_DISPLAYON);
    display.invertDisplay(false);
    display.dim(false);
    display.clearDisplay();
    display.display();
  }
  oledReady = oledOk;
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
  trace = "Setup";
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
  digitalWrite(gateOn, LOW);
  digitalWrite(gateOff, LOW);

  //trace = "Booting";
  //displayStat();

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
  
  Serial.print("Servo A Open        = ");
  Serial.println(openA);

  Serial.print("Servo A Closed      = ");
  Serial.println(closedA);

  Serial.print("Servo B Open        = ");
  Serial.println(openB);

  Serial.print("Servo B Closed      = ");
  Serial.println(closedB);
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
  // }

  Serial.println("");
  Serial.println("Setup Line 185");
  Serial.print("WiFi SSID: ");
  Serial.println(WiFi.SSID());
  Serial.print("WiFi IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("WiFi Gateway: ");
  Serial.println(WiFi.gatewayIP());
  Serial.print("WiFi Subnet: ");
  Serial.println(WiFi.subnetMask());
  Serial.print("WiFi MAC: ");
  Serial.println(WiFi.macAddress());
  Serial.print("OTA Name: ");
  Serial.println(gateName);
  OTA();
  ArduinoOTA.handle();
  Serial.println("OTA Ready - waiting for upload...");
  Serial.println("If OTA not working, try: pio run --target upload --upload-port " + WiFi.localIP().toString());

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

  if ((gateType != "A") && (gateType != "B") && (gateType != "C") && (gateType != "D") && (gateType != "S") && (gateType != "P") && (gateType != "X") && (gateType != "M"))
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
  if (gateType =="X") {
    Serial.println("Setup line 270 Gate Type = X");
  pinMode(switchPinA, INPUT_PULLUP);
  pinMode(switchPinB, INPUT_PULLUP);
  pinMode(servoPinA, OUTPUT);
  pinMode(servoPinB, OUTPUT); 
  return;
}

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
      setGateState(STATE_CLOSED);
    }
    return;
  }

  Serial.println("Setup  line line 255 ");

  //*****************************************
  //        All Other Controllers
  Serial.println("Setup  line line 269");

  sensorIn = (analogRead(ANALOG_PIN_IN));
  checkSwitchState();

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
    setGateState(STATE_CLOSED);
  }
  if ((sensorIn >= trigger) && (sensorIn <= (trigger + triggerDelta)) && (gateCloseState == false))
  {
    // Mid-band sensor noise on boot should not leave a physically open gate open.
    homePosition();
    trace = "CLOSED";
    gateOpenTime = 0;
    displayStat();
    setGateState(STATE_CLOSED);
  }

  Serial.print("Setup line 263");
  db = "Setup 275 ";
  client.publish(mID, db, false);
}