#include "globals.h"
#include "tables.h"  // board configuration lookup

// board configuration table moved here from boardConfig.cpp

// struct used to capture pin assignments and metadata for a single
// board.  one row per board id (minId==maxId) per request.
struct BoardConfigEntry {
  const char *boardType;
  int minId;
  int maxId;
  byte version;
  int analogPin;
  int limitSwitchPin;
  int reedRelayPin;
  int greenLEDpin;
  int redLEDpin;
  int gateOn;
  int gateOff;
  int stepPin;
  int dirPin;
  int enablePin;
};

static const BoardConfigEntry boardTable[] = {
    {"v2 Blue", 0, 0, 3, A0, 5, 23, 5, 5, 16, 27, 5, 14, 5},
    {"v2 Blue", 1, 1, 3, A0, 5, 23, 5, 5, 16, 27, 5, 14, 5},
    {"v2 Blue", 2, 2, 3, A0, 5, 23, 5, 5, 16, 27, 5, 14, 5},
    {"v2 Blue", 3, 3, 3, A0, 5, 23, 5, 5, 16, 27, 5, 14, 5},
    {"v2 Blue", 4, 4, 3, A0, 5, 23, 5, 5, 16, 27, 5, 14, 5},
    {"v3 Red", 5, 5, 3, A0, 5, 23, 5, 5, 16, 27, 5, 14, 5},
    {"v3 Red", 6, 6, 3, A0, 5, 23, 5, 5, 16, 27, 5, 14, 5},
    {"v4 Blue", 10, 10, 4, 36, 19, 23, 5, 5, 16, 27, 16, 14, 17},
    {"v4 Blue", 11, 11, 4, 36, 19, 23, 5, 5, 16, 27, 16, 14, 17},
    {"v4 Blue", 12, 12, 4, 36, 19, 23, 5, 5, 16, 27, 16, 14, 17},
    {"v4 Blue", 13, 13, 4, 36, 19, 23, 5, 5, 16, 27, 16, 14, 17},
    {"v5 Black", 14, 14, 5, 36, 19, 23, 5, 5, 16, 27, 16, 14, 17},
    {"v5 Black", 15, 15, 5, 36, 19, 23, 5, 5, 16, 27, 16, 14, 17},
    {"v5 Black", 16, 16, 5, 36, 19, 23, 5, 5, 16, 27, 16, 14, 17},
    {"v5 Black", 17, 17, 5, 36, 19, 23, 5, 5, 16, 27, 16, 14, 17},
    {"v5 Black", 18, 18, 5, 36, 19, 23, 5, 5, 16, 27, 16, 14, 17},
    {"v5 Black", 19, 19, 5, 36, 19, 23, 5, 5, 16, 27, 16, 14, 17},
    {"v5 Black", 20, 20, 5, 36, 19, 23, 5, 5, 16, 27, 16, 14, 17},
    {"v5 Black", 21, 21, 5, 36, 19, 23, 5, 5, 16, 27, 16, 14, 17},
    {"v5 Black", 22, 22, 5, 36, 19, 23, 5, 5, 16, 27, 16, 14, 17},
    {"v5 Black", 23, 23, 5, 36, 19, 23, 5, 5, 16, 27, 16, 14, 17},
    {"v5 Black", 24, 24, 5, 36, 19, 23, 5, 5, 16, 27, 16, 14, 17},
    {"v5 Black", 25, 25, 5, 36, 19, 23, 5, 5, 16, 27, 16, 14, 17},
    {"v5 Black", 26, 26, 5, 36, 19, 23, 5, 5, 16, 27, 16, 14, 17},
    {"v5 Black", 27, 27, 5, 36, 19, 23, 5, 5, 16, 27, 16, 14, 17},
    {"v5 Black", 28, 28, 5, 36, 19, 23, 5, 5, 16, 27, 16, 14, 17},
    {"v10 Blue", 32, 32, 10, 32, 19, 18, 5, 5, 5, 27, 16, 14, 17},
    {"v10 Blue", 33, 33, 10, 32, 19, 18, 5, 5, 5, 27, 16, 14, 17},
    {"v10 Blue", 34, 34, 10, 32, 19, 18, 5, 5, 5, 27, 16, 14, 17},
    {"v10 Blue", 35, 35, 10, 32, 19, 18, 5, 5, 5, 27, 16, 14, 17},
    {"v10 Blue", 36, 36, 10, 32, 19, 18, 5, 5, 5, 27, 16, 14, 17},
    {"v10 Blue", 37, 37, 10, 32, 19, 18, 5, 5, 5, 27, 16, 14, 17},
    {"v10 Blue", 38, 38, 10, 32, 19, 18, 5, 5, 5, 27, 16, 14, 17},
    {"v10 Blue", 39, 39, 10, 32, 19, 18, 5, 5, 5, 27, 16, 14, 17},
    {"v10 Blue", 40, 40, 10, 32, 19, 18, 5, 5, 5, 27, 16, 14, 17},
    {"v10 Blue", 41, 41, 10, 32, 19, 18, 5, 5, 5, 27, 16, 14, 17},
    {"v10 Blue", 42, 42, 10, 32, 19, 18, 5, 5, 5, 27, 16, 14, 17},
    {"v11 Purple", 43, 43, 11, 32, 18, 19, 33, 33, 33, 27, 16, 14, 17},
    {"v10 Blue", 44, 44, 10, 32, 19, 18, 5, 5, 5, 27, 16, 14, 17},
    {"v10 Blue", 45, 45, 10, 32, 19, 18, 5, 5, 5, 27, 16, 14, 17},
    {"v10 Blue", 46, 46, 10, 32, 19, 18, 5, 5, 5, 27, 16, 14, 17},
    {"v10 Blue", 47, 47, 10, 32, 19, 18, 5, 5, 5, 27, 16, 14, 17},
    {"v10 Blue", 48, 48, 10, 32, 19, 18, 5, 5, 5, 27, 16, 14, 17},
    {"v10 Blue", 49, 49, 10, 32, 19, 18, 5, 5, 5, 27, 16, 14, 17},
    {"v10 Blue", 50, 50, 10, 32, 19, 18, 5, 5, 5, 27, 16, 14, 17},
};

// look up pin assignments based on the boardIdByte read from EEPROM
void applyBoardConfiguration()
{
  bool found = false;
  for (const auto &entry : boardTable) {
    if ((boardIdByte >= entry.minId) && (boardIdByte <= entry.maxId)) {
      boardVer = entry.version;
      ANALOG_PIN_IN = entry.analogPin;
      limitSwitchPin = entry.limitSwitchPin;
      reedRelayPin = entry.reedRelayPin;
      greenLEDpin = entry.greenLEDpin;
      redLEDpin = entry.redLEDpin;
      gateOn = entry.gateOn;
      gateOff = entry.gateOff;
      stepPin = entry.stepPin;
      dirPin = entry.dirPin;
      enablePin = entry.enablePin;
      boardTypeName = entry.boardType;
      found = true;
      break;
    }
  }
  if (!found) {
    Serial.print("Unknown boardIdByte: ");
    Serial.println(boardIdByte);
    eCode = 4;
    errorState();
  }

  // special case from prior logic: board id 6 uses ADC32
  if (boardIdByte == 6) {
    ANALOG_PIN_IN = 32;
  }
}


void tools()   {
  String tools[51] = {







    "New Gate",
    "Laguna_Resaw",
    "Laguna_3000",
    "Laguna SE",
    "CNC HDM",
    "CNC XXL",
    "Jet Combo Sander",
    "NA",
    "NA",
    "NA",
    "Supermax_2",
    "Supermax_1",
    "16 Jointer",
    "NA",
    "Dust Collector",
    "Edge Sander",
    "12 inch Jointer",
    "8 inch Jointer",
    "NA",
    "NA",
    "NA",
    "NA",
    "NA",
    "NA",
    "NA",
    "Robust Lathe",
    "Miter Saw",
    "NA",
    "Dewalt Planer",
    "20 inch Planer",
    "Router table #1",
    "Router table #2",
    "NA",
    "Enlon Spindle Sander",
    "Powermatic Spindle",
    "SawStop #1",
    "SawStop #2",
    "Pegas",
    "Combo Sander #2",
    "SawStop#2",



    "Jet 12-21 Lathe",

    "15 inch planer",




   "Static Presure Sensor",


  };
  toolInt = machineID.toInt();
  toolString = tools[toolInt];

  String gTypes[51] = {


    "B",// 00 New Gate         
    "A",// 01 Laguna_Resaw     
    "A",// 02 Laguna_3000      
    "A",// 03 Laguna_SE        
    "C",// 04 CNC HDM          
    "C",// 05 CNC XXL          
    "B",// 06 Jet Combo Sander   
    "Z",//
    "Z",//
    "Z",//
    "A",// 10 Supermax_2       
    "A",// 11 Supermax_1       
    "A",// 12 16" Jointer
    "Z",//
    "M",// 14 Dust Collector   
    "B",// 15 Edge Sander      
    "A",// 16 12 inch Jointer  
    "A",// 17 8 inch Jointer   
    "Z",//
    "Z",//
    "Z",//
    "Z",//
    "Z",//
    "Z",//
    "Z",//  
    "M",// 25 Robust Lathe    
    "P",// 26 Miter Saw       
    "Z",//
    "Z",// 27 Dewalt Planer   
    "A",// 29 20 inch Planer  
    "B",// 30 Router table #1 
    "B",// 31 Router table #2 
    "Z",//    
    "B",// 33 Enlon Spindle Sander 
    "B",// 34 Powermatic Spindle   
    "A",// 35 SawStop #1      
    "Y",//      
    "W",//
    "B",// 38 Combo Sander #2
    "A",// 39 SawStop #2
    "X",//
    "T",//
    "L",//
    "M",// 43 Jet Lathe
    "Z",//
    "A",// 45 Powermatic 15" Planer
    "Z",//
    "Z",//
    "Z",//
    "Z",//           
    "S",// 50 Static Pressure Sensor 
  };
 
 
  gateType = gTypes[toolInt];

}
