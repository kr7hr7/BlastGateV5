#include "boardConfig.h"
#include "globals.h"

void boardconfiguration() {
  tools();
  gateTypeConfig();
    Serial.print ("boardconfig line 4");  
        Serial.print ("   eeprom Update = ");
        Serial.println (eepromUpdate);
  if (eepromUpdate == true) {
    eepromWrite();
  }
  Serial.println("boardConfig line 7");
  if (EEPROM.read(3) == 200) {
    boardIdByte     = EEPROM.read(0);

    Serial.print ("      boardConfig line 44 boardIdByte = ");
    Serial.print (boardIdByte);
    Serial.print (" boardID (Int) = ");
    Serial.println (boardID.toInt());
    if (boardIdByte == boardID.toInt()) {
      Serial.println ("Board ID confirmed with EEPROM");
    }
    else {
      Serial.println ("                   boardConfig line #34  setup error");
      eCode = 2;
      errorState();
    }
  }
  gatePinConfig();
}

//********************************************************************************
void gateTypeConfig() {
  //Serial.println ("CONFIG!!!!!!!!!! Line 2");
  if (gateType == "A") {   // For 5" Gates
    rotation = false;
    fullRunSteps = 22500;
  }
  if (gateType == "B") {   // For 4" Gates
    rotation = false;
    fullRunSteps = 18000;
  }
  if (gateType == "C") {    // For use with the 5"gate Dust collector remains on during the countdown
    rotation = false;
    fullRunSteps = 22500;
  }
  if (gateType == "D") {   // For use with 4" Gate Cleanup Ports Dust Collector remains on during the countdown
    rotation = false;
    fullRunSteps = 18000;
  }
  if (gateType == "M") {   // "M" is for manual gates (No controller).  It captures data, starts the DC, and provides an indicator light.
    gateDelaySeconds = 0;
    fullRunSteps = 0;
  }
  if (gateType == "S") {   // "S" is to designate the controller to cature and report static pressure sensor data
  }
  if (gateType == "P") {    // For use with tools that do not have a blast gate.   Used to capture useage information.
    fullRunSteps = 0;
    gateDelaySeconds = 0;
  }
}

//********************************************************************************
void eepromWrite() {
  boardIdByte = boardID.toInt();
  EEPROM.write(0, boardIdByte);
  EEPROM.write(3, 200);
  EEPROM.commit();

  boardIdByte     = EEPROM.read(0);
  boardVer        = EEPROM.read(1);
  boardConfig     = EEPROM.read(2);
  flash           = EEPROM.read(3);

  Serial.println(" ");

  Serial.print("boardID: ");
  Serial.println(boardIdByte);

  Serial.print("Board Version = ");
  Serial.println(boardVer);

  Serial.print("boardConfig = ");
  Serial.println(boardConfig);

  Serial.print("Flashed = ");
  Serial.println(flash);
  Serial.println(" ");
  Serial.println ("CONFIG!!!!!!!!!! Line 56");
  Serial.println(" ");
}

//********************************************************************************
void gatePinConfig() {
  boardIdByte = boardID.toInt();
  if (boardIdByte <= 4) {                                 // v2 Blue
    boardVer = 3;
  }
  if (boardIdByte >= 7 && boardIdByte <= 5) {             // v3 Red
    boardVer = 3;
    if (boardIdByte == 6 ) {
      ANALOG_PIN_IN = 32;
    }
  }
  if (boardIdByte >= 10 && boardIdByte <= 13) {           // v4 Blue
    boardVer = 4;
  }
  if (boardIdByte >= 14 && boardIdByte <= 28) {           // v5 Black
    boardVer = 5;
  }

  if (boardIdByte >= 32 && boardIdByte <= 50) {           // v10 Blue
    boardVer = 10;
  }
  if (boardIdByte == 43 ) {           // v11 Purple
    boardVer = 11;
  }
 if (boardIdByte == 44 ) {           //  v10 Blue
    boardVer = 10;
  }
if (boardIdByte == 46 ) {           //  v10 Blue
    boardVer = 10;
  }


  
  if ((boardVer == 2) || (boardVer == 2))  {
    ANALOG_PIN_IN        = A0;
    limitSwitchPin       = 05;
    reedRelayPin         = 23;
    greenLEDpin          = 05;
    redLEDpin            = 05;
    gateOn               = 16;
    gateOff              = 27;    // moved off strapping pin
    stepPin              = 05;
    dirPin               = 14;    // safe pin instead of 4/5
    enablePin            = 05;
  }
  if (boardVer == 3)  {
    ANALOG_PIN_IN        = A0;
    limitSwitchPin       = 05;
    reedRelayPin         = 23;
    greenLEDpin          = 05;
    redLEDpin            = 05;
    gateOn               = 16;
    gateOff              = 27;
    stepPin              = 05;
    dirPin               = 14;
    enablePin            = 05;
  }
  if ((boardIdByte) == 06) {
    ANALOG_PIN_IN = 32;
  }
  
  if ((boardVer == 4) || (boardVer == 5))  {
    ANALOG_PIN_IN        = 36;
    limitSwitchPin       = 19;
    reedRelayPin         = 23;
    greenLEDpin          = 05;
    redLEDpin            = 05;
    gateOn               = 16;
    gateOff              = 27;
    stepPin              = 16;
    dirPin               = 14;
    enablePin            = 17;
  }

  if (boardVer == 10)  {
    ANALOG_PIN_IN        = 32;
    limitSwitchPin       = 19;
    reedRelayPin         = 18;
    greenLEDpin          = 05;
    redLEDpin            = 05;
    gateOn               = 05;
    gateOff              = 27;
    stepPin              = 16;
    dirPin               = 14;
    enablePin            = 17;
  }

  if (boardVer == 11)  {
    ANALOG_PIN_IN        = 32;
    limitSwitchPin       = 18;
    reedRelayPin         = 19;
    greenLEDpin          = 33;
    redLEDpin            = 33;
    gateOn               = 33;
    gateOff              = 27;
    stepPin              = 16;
    dirPin               = 14;
    enablePin            = 17;
  }
}
