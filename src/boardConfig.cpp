#include "boardConfig.h"
#include "globals.h"
#include "tables.h"  // lookup helper for pin settings

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

  // configure pins and version using table lookup
  applyBoardConfiguration();
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

