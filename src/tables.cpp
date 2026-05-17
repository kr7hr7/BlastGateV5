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
  int switchPinA;
  int servoPinA;
  int switchPinB;
  int servoPinB;

  BoardConfigEntry(const char *boardType,
                   int minId,
                   int maxId,
                   byte version,
                   int analogPin,
                   int limitSwitchPin,
                   int reedRelayPin,
                   int greenLEDpin,
                   int redLEDpin,
                   int gateOn,
                   int gateOff,
                   int stepPin,
                   int dirPin,
                   int enablePin,
                   int switchPinA,
                   int servoPinA,
                   int switchPinB,
                   int servoPinB)
      : boardType(boardType),
        minId(minId),
        maxId(maxId),
        version(version),
        analogPin(analogPin),
        limitSwitchPin(limitSwitchPin),
        reedRelayPin(reedRelayPin),
        greenLEDpin(greenLEDpin),
        redLEDpin(redLEDpin),
        gateOn(gateOn),
        gateOff(gateOff),
        stepPin(stepPin),
        dirPin(dirPin),
        enablePin(enablePin),
        switchPinA(switchPinA),
        servoPinA(servoPinA),
        switchPinB(switchPinB),
        servoPinB(servoPinB) {}
};

static const BoardConfigEntry boardTable[] = {
  //              MID   Ver  A0  LS  RR  GLED RLED GON GOFF STEP DIR ENBL SWA SVA SWB SVB
  {"v2 Blue",     0, 0,  3,  A0,  5, 23, 5,   5,   16,  27,  5,   4,  5, 5, 5, 5, 5},
  {"v2 Blue",     1, 1,  3,  A0,  5, 23, 5,   5,   16,  27,  5,   4,  5, 5, 5, 5, 5},
  {"v2 Blue",     2, 2,  3,  A0,  5, 23, 5,   5,   16,  27,  5,   4,  5, 5, 5, 5, 5},
  {"v2 Blue",     3, 3,  3,  A0,  5, 23, 5,   5,   16,  27,  5,   4,  5, 5, 5, 5, 5},
  {"v2 Blue",     4, 4,  3,  A0,  5, 23, 5,   5,   16,  27,  5,   4,  5, 5, 5, 5, 5},
  {"v3 Red",      5, 5,  3,  A0,  5, 23, 5,   5,   16,  27,  5,   4,  5, 5, 5, 5, 5},
  {"v3 Red",      6, 6,  3,  A0,  5, 23, 5,   5,   16,  27,  5,   4,  5, 5, 5, 5, 5},
  {"v4 Blue",    10, 10, 4,  36, 19, 23, 5,   5,    5,  27, 16,   4, 17, 5, 5, 5, 5},
  {"v4 Blue",    11, 11, 4,  36, 19, 23, 5,   5,    5,  27, 16,   4, 17, 5, 5, 5, 5},
  {"v4 Blue",    12, 12, 4,  36, 19, 23, 5,   5,    5,  27, 16,   4, 17, 5, 5, 5, 5},
  {"v4 Blue",    13, 13, 4,  36, 19, 23, 5,   5,    5,  27, 16,   4, 17, 5, 5, 5, 5},
  {"v5 Black",   14, 14, 5,  36, 19, 23, 5,   5,    5,  27, 16,   4, 17, 5, 5, 5, 5},
  {"v5 Black",   15, 15, 5,  36, 19, 23, 5,   5,    5,  27, 16,   4, 17, 5, 5, 5, 5},
  {"v5 Black",   16, 16, 5,  36, 19, 23, 5,   5,    5,  27, 16,   4, 17, 5, 5, 5, 5},
  {"v5 Black",   17, 17, 5,  36, 19, 23, 5,   5,    5,  27, 16,   4, 17, 5, 5, 5, 5},
  {"v5 Black",   18, 18, 5,  36, 19, 23, 5,   5,    5,  27, 16,   4, 17, 5, 5, 5, 5},
  {"v5 Black",   19, 19, 5,  36, 19, 23, 5,   5,    5,  27, 16,   4, 17, 5, 5, 5, 5},
  {"v5 Black",   20, 20, 5,  36, 19, 23, 5,   5,    5,  27, 16,   4, 17, 5, 5, 5, 5},
  {"v5 Black",   21, 21, 5,  36, 19, 23, 5,   5,    5,  27, 16,   4, 17, 5, 5, 5, 5},
  {"v5 Black",   22, 22, 5,  36, 19, 23, 5,   5,    5,  27, 16,   4, 17, 5, 5, 5, 5},
  {"v5 Black",   23, 23, 5,  36, 19, 23, 5,   5,    5,  27, 16,   4, 17, 5, 5, 5, 5},
  {"v5 Black",   24, 24, 5,  35, 19, 23, 5,   5,    5,  27, 16,   4, 17, 5, 5, 5, 5},
  {"v5 Black",   25, 25, 5,  36, 19, 23, 5,   5,    5,  27, 16,   4, 17, 5, 5, 5, 5},
  {"v5 Black",   26, 26, 5,  36, 19, 23, 5,   5,    5,  27, 16,   4, 17, 5, 5, 5, 5},
  {"v5 Black",   27, 27, 5,  36, 19, 23, 5,   5,    5,  27, 16,   4, 17, 5, 5, 5, 5},
  {"v5 Black",   28, 28, 5,  36, 19, 23, 5,   5,    5,  27, 16,   4, 17, 5, 5, 5, 5},
  {"v10 Blue",   32, 32,10,  32, 19, 18, 5,   5,    5,  27, 16,   4, 17, 5, 5, 5, 5},
  {"v10 Blue",   33, 33,10,  32, 19, 18, 5,   5,    5,  27, 16,   4, 17, 5, 5, 5, 5},
  {"v10 Blue",   34, 34,10,  33, 19, 18, 5,   5,    5,  27, 16,   4, 17, 5, 5, 5, 5},
  {"v10 Blue",   35, 35,10,  32, 19, 18, 5,   5,    5,  27, 16,   4, 17, 5, 5, 5, 5},
  {"v10 Blue",   36, 36,10,  32, 19, 18, 5,   5,    5,  27, 16,   4, 17, 5, 5, 5, 5},
  {"v10 Blue",   37, 37,10,  32, 19, 18, 5,   5,    5,  27, 16,   4, 17, 5, 5, 5, 5},
  {"v10 Blue",   38, 38,10,  32, 19, 18, 5,   5,    5,  27, 16,   4, 17, 5, 5, 5, 5},
  {"v10 Blue",   39, 39,10,  32, 19, 18, 5,   5,    5,  27, 16,   4, 17, 5, 5, 5, 5},
  {"v10 Blue",   40, 40,10,  32, 19, 18, 5,   5,    5,  27, 16,   4, 17, 5, 5, 5, 5},
  {"v10 Blue",   41, 41,10,  32, 19, 18, 5,   5,    5,  27, 16,   4, 17, 5, 5, 5, 5}, 
  {"v10 Blue",   42, 42,10,  32, 19, 18, 5,   5,    5,  27, 16,   4, 17, 5, 5, 5, 5},
  {"v11 Purple", 43, 43,11,  32, 18, 19, 33, 33,   33,  27, 16,   4, 17, 5, 5, 5, 5},
  {"v10 Blue",   44, 44,10,  32, 19, 18, 5,   5,    5,  27, 16,   4, 17, 5, 5, 5, 5},
  {"v10 Blue",   45, 45,10,  32, 19, 18, 5,   5,    5,  27, 16,   4, 17, 5, 5, 5, 5},
  {"v10 Blue",   46, 46,10,  32, 19, 18, 5,   5,    5,  27, 16,   4, 17, 5, 5, 5, 5},
  {"v10 Blue",   47, 47,10,  32, 19, 18, 5,   5,    5,  27, 16,   4, 17, 5, 5, 5, 5},
  {"v13 Green",  48, 48,13,  32,  5, 19, 5,   5,    5,  27, 16,   4, 17,18, 4, 32, 17},
  {"v13 Green",  49, 49,13,  32, 18, 19, 5,   5,    5,  27, 16,   4, 17, 5, 5, 5, 5},
  {"v10 Blue",   50, 50,10,  32, 19, 18, 5,   5,    5,  27, 16,   4, 17, 5, 5, 5, 5},
  {"v14 Blue",   60, 60,14,  39, 18, 19, 27, 27,   27,  27, 16,   4, 17, 25, 27, 32, 33},
  {"v14 Blue",   61, 61,14,  39, 18, 19, 27, 27,   27,  27, 16,   4, 17, 25, 27, 32, 33},
  {"v14 Blue",   62, 62,14,  39, 18, 19, 27, 27,   27,  27, 16,   4, 17, 25, 27, 32, 33},
  {"v14 Blue",   63, 63,14,  39, 18, 19, 27, 27,   27,  27, 16,   4, 17, 25, 27, 32, 33},
  {"v14 Blue",   64, 64,14,  39, 18, 19, 27, 27,   27,  27, 16,   4, 17, 25, 27, 32, 33},


    //              MID   Ver  A0  LS  RR  GLED RLED GON GOFF STEP DIR ENBL SWA SVA SWB SVB
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
      switchPinA = entry.switchPinA;
      servoPinA = entry.servoPinA;
      switchPinB = entry.switchPinB;
      servoPinB = entry.servoPinB;
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

struct MachineGateEntry {
  int toolInt;
  const char *gateName;
  const char *gateTypeString;
  bool rotation;
};

// One row per machine/blast-gate mapping.
// To add a machine: insert a new line anywhere in the table below:
//   {toolInt, "Machine Name", "X", rotation},
// where toolInt is the numeric ID and "X" is the gate mode (A/B/C/M/P/S/W/X/Y/Z).
// Unrecognized IDs default to "NA"/"Z" — no placeholder row needed.
// Keep rows sorted by toolInt ascending for readability.
static const MachineGateEntry machineGateTable[] = {
  {0,  "New Gate",              "B", true},
  {1,  "Laguna_Resaw",          "A", false},
  {2,  "Laguna_3000",           "A", false},
  {3,  "Laguna SE",             "A", false},
  {4,  "CNC HDM",               "C", false},
  {5,  "CNC XXL",               "C", false},
  {6,  "Jet Combo Sander",      "B", false},
  {10, "Supermax_2",            "A", false},
  {11, "Supermax_1",            "A", false},
  {12, "16 Jointer",            "A", false},
  {14, "Dust Collector",        "M", false},
  {15, "Edge Sander",           "B", false},
  {16, "12 inch Jointer",       "A", false},
  {17, "Jet 14 Bandsaw",        "B", false},
  {24, "Robust #2",             "L", false},
  {25, "Robust #1",             "L", false},
  {26, "Miter Saw",             "P", false},
  {29, "20 inch Planer",        "A", false},
  {30, "Router table #1",       "B", false},
  {31, "Router table #2",       "B", false},
  {33, "Enlon Spindle Sander",  "B", false},
  {34, "Powermatic Spindle",    "B", false},
  {35, "SawStop #1",            "A", false},
  {36, "Removed SawStop #2",    "Y", false},
  {37, "Pegas",                 "W", false},
  {38, "Combo Sander #2",       "B", false},
  {39, "SawStop#2",             "A", false},
  {43, "Jet 12-21 Lathe",       "L", false},
  {45, "15 inch planer",        "A", false},
  {50, "Static Pressure Sensor","S", false},
  {51, "Miter Saw Cleanup  ",   "C", false},
  {52, "Exhaust Back Pressure", "S", false},
  {60, "Powermatic Lathe",      "L", false},
};


void tools()   {
  toolInt = machineID.toInt();

  // Defaults if machine ID has no row in machineGateTable.
  toolString = "NA";
  gateTypeString = "Z";
  gateType = gateTypeString;
  rotation = false;

  for (const auto &entry : machineGateTable) {
    if (entry.toolInt == toolInt) {
      toolString = entry.gateName;
      gateTypeString = entry.gateTypeString;
      gateType = gateTypeString;
      rotation = entry.rotation;
      break;
    }
  }

}
