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
  {"v2 Blue", 0, 0, 3, A0, 5, 23, 5, 5, 16, 27, 5, 14, 5, 5, 5, 5, 5},
  {"v2 Blue", 1, 1, 3, A0, 5, 23, 5, 5, 16, 27, 5, 14, 5, 5, 5, 5, 5},
  {"v2 Blue", 2, 2, 3, A0, 5, 23, 5, 5, 16, 27, 5, 14, 5, 5, 5, 5, 5},
  {"v2 Blue", 3, 3, 3, A0, 5, 23, 5, 5, 16, 27, 5, 14, 5, 5, 5, 5, 5},
  {"v2 Blue", 4, 4, 3, A0, 5, 23, 5, 5, 16, 27, 5, 14, 5, 5, 5, 5, 5},
  {"v3 Red", 5, 5, 3, A0, 5, 23, 5, 5, 16, 27, 5, 14, 5, 5, 5, 5, 5},
  {"v3 Red", 6, 6, 3, A0, 5, 23, 5, 5, 16, 27, 5, 14, 5, 5, 5, 5, 5},
  {"v4 Blue", 10, 10, 4, 36, 19, 23, 5, 5, 16, 27, 16, 14, 17, 5, 5, 5, 5},
  {"v4 Blue", 11, 11, 4, 36, 19, 23, 5, 5, 16, 27, 16, 14, 17, 5, 5, 5, 5},
  {"v4 Blue", 12, 12, 4, 36, 19, 23, 5, 5, 16, 27, 16, 14, 17, 5, 5, 5, 5},
  {"v4 Blue", 13, 13, 4, 36, 19, 23, 5, 5, 16, 27, 16, 14, 17, 5, 5, 5, 5},
  {"v5 Black", 14, 14, 5, 36, 19, 23, 5, 5, 16, 27, 16, 14, 17, 5, 5, 5, 5},
  {"v5 Black", 15, 15, 5, 36, 19, 23, 5, 5, 16, 27, 16, 14, 17, 5, 5, 5, 5},
  {"v5 Black", 16, 16, 5, 36, 19, 23, 5, 5, 16, 27, 16, 14, 17, 5, 5, 5, 5},
  {"v5 Black", 17, 17, 5, 36, 19, 23, 5, 5, 16, 27, 16, 14, 17, 5, 5, 5, 5},
  {"v5 Black", 18, 18, 5, 36, 19, 23, 5, 5, 16, 27, 16, 14, 17, 5, 5, 5, 5},
  {"v5 Black", 19, 19, 5, 36, 19, 23, 5, 5, 16, 27, 16, 14, 17, 5, 5, 5, 5},
  {"v5 Black", 20, 20, 5, 36, 19, 23, 5, 5, 16, 27, 16, 14, 17, 5, 5, 5, 5},
  {"v5 Black", 21, 21, 5, 36, 19, 23, 5, 5, 16, 27, 16, 14, 17, 5, 5, 5, 5},
  {"v5 Black", 22, 22, 5, 36, 19, 23, 5, 5, 16, 27, 16, 14, 17, 5, 5, 5, 5},
  {"v5 Black", 23, 23, 5, 36, 19, 23, 5, 5, 16, 27, 16, 14, 17, 5, 5, 5, 5},
  {"v5 Black", 24, 24, 5, 36, 19, 23, 5, 5, 16, 27, 16, 14, 17, 5, 5, 5, 5},
  {"v5 Black", 25, 25, 5, 36, 19, 23, 5, 5, 16, 27, 16, 14, 17, 5, 5, 5, 5},
  {"v5 Black", 26, 26, 5, 36, 19, 23, 5, 5, 16, 27, 16, 14, 17, 5, 5, 5, 5},
  {"v5 Black", 27, 27, 5, 36, 19, 23, 5, 5, 16, 27, 16, 14, 17, 5, 5, 5, 5},
  {"v5 Black", 28, 28, 5, 36, 19, 23, 5, 5, 16, 27, 16, 14, 17, 5, 5, 5, 5},
  {"v10 Blue", 32, 32, 10, 32, 19, 18, 5, 5, 5, 27, 16, 4, 17, 5, 5, 5, 5},
  {"v10 Blue", 33, 33, 10, 32, 19, 18, 5, 5, 5, 27, 16, 4, 17, 5, 5, 5, 5},
  {"v10 Blue", 34, 34, 10, 32, 19, 18, 5, 5, 5, 27, 16, 4, 17, 5, 5, 5, 5},
  {"v10 Blue", 35, 35, 10, 32, 19, 18, 5, 5, 5, 27, 16, 4, 17, 5, 5, 5, 5},
  {"v10 Blue", 36, 36, 10, 32, 19, 18, 5, 5, 5, 27, 16, 4, 17, 5, 5, 5, 5},
  {"v10 Blue", 37, 37, 10, 32, 19, 18, 5, 5, 5, 27, 16, 4, 17, 5, 5, 5, 5},
  {"v10 Blue", 38, 38, 10, 32, 19, 18, 5, 5, 5, 27, 16, 4, 17, 5, 5, 5, 5},
  {"v10 Blue", 39, 39, 10, 32, 19, 18, 5, 5, 5, 27, 16, 4, 17, 5, 5, 5, 5},
  {"v10 Blue", 40, 40, 10, 32, 19, 18, 5, 5, 5, 27, 16, 4, 17, 5, 5, 5, 5},
  {"v10 Blue", 41, 41, 10, 32, 19, 18, 5, 5, 5, 27, 16, 4, 17, 5, 5, 5, 5},
  {"v10 Blue", 42, 42, 10, 32, 19, 18, 5, 5, 5, 27, 16, 4, 17, 5, 5, 5, 5},
  {"v11 Purple", 43, 43, 11, 32, 18, 19, 33, 33, 33, 27, 16, 4, 17, 5, 5, 5, 5},
  {"v10 Blue", 44, 44, 10, 32, 19, 18, 5, 5, 5, 27, 16, 4, 17, 5, 5, 5, 5},
  {"v10 Blue", 45, 45, 10, 32, 19, 18, 5, 5, 5, 27, 16, 4, 17, 5, 5, 5, 5},
  {"v10 Blue", 46, 46, 10, 32, 19, 18, 5, 5, 5, 27, 16, 4, 17, 5, 5, 5, 5},
  {"v10 Blue", 47, 47, 10, 32, 19, 18, 5, 5, 5, 27, 16, 4, 17, 5, 5, 5, 5},
  {"v10 Blue", 48, 48, 10, 32, 19, 18, 5, 5, 5, 27, 16, 4, 17, 18, 4, 32, 17},
  {"v13 Green", 49, 49, 13, 32, 18, 19, 5, 5, 5, 27, 16, 4, 17, 5, 5, 5, 5},
  {"v10 Blue", 50, 50, 10, 32, 19, 18, 5, 5, 5, 27, 16, 14, 17, 5, 5, 5, 5},
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
};

// One row per machine/blast-gate mapping.
// Edit guide:
// - Keep rows sorted by toolInt ascending.
// - gateName is the human-readable machine/tool label.
// - gateTypeString is the controller mode code (A/B/C/.../Z).
// - Use "NA" with type "Z" for unused machine IDs.
// This table is intended to be edited frequently as machines are added/changed.
static const MachineGateEntry machineGateTable[] = {
  {0, "New Gate", "B"},
  {1, "Laguna_Resaw", "A"},
  {2, "Laguna_3000", "A"},
  {3, "Laguna SE", "A"},
  {4, "CNC HDM", "C"},
  {5, "CNC XXL", "C"},
  {6, "Jet Combo Sander", "B"},
  {7, "NA", "Z"},
  {8, "NA", "Z"},
  {9, "NA", "Z"},
  {10, "Supermax_2", "A"},
  {11, "Supermax_1", "A"},
  {12, "16 Jointer", "A"},
  {13, "NA", "Z"},
  {14, "Dust Collector", "M"},
  {15, "Edge Sander", "B"},
  {16, "12 inch Jointer", "A"},
  {17, "8 inch Jointer", "A"},
  {18, "NA", "Z"},
  {19, "NA", "Z"},
  {20, "NA", "Z"},
  {21, "NA", "Z"},
  {22, "NA", "Z"},
  {23, "NA", "Z"},
  {24, "NA", "Z"},
  {25, "Robust Lathe", "X"},
  {26, "Miter Saw", "P"},
  {27, "NA", "Z"},
  {28, "Dewalt Planer", "Z"},
  {29, "20 inch Planer", "A"},
  {30, "Router table #1", "B"},
  {31, "Router table #2", "B"},
  {32, "NA", "Z"},
  {33, "Enlon Spindle Sander", "B"},
  {34, "Powermatic Spindle", "B"},
  {35, "SawStop #1", "A"},
  {36, "SawStop #2", "Y"},
  {37, "Pegas", "W"},
  {38, "Combo Sander #2", "B"},
  {39, "SawStop#2", "A"},
  {40, "Jet 12-21 Lathe", "X"},
  {41, "15 inch planer", "T"},
  {42, "NA", "L"},
  {43, "NA", "M"},
  {44, "NA", "Z"},
  {45, "NA", "A"},
  {46, "NA", "Z"},
  {47, "NA", "Z"},
  {48, "NA", "Z"},
  {49, "NA", "Z"},
  {50, "Static Presure Sensor", "S"},
};


void tools()   {
  toolInt = machineID.toInt();

  // Defaults if machine ID has no row in machineGateTable.
  toolString = "NA";
  gateTypeString = "Z";
  gateType = gateTypeString;

  for (const auto &entry : machineGateTable) {
    if (entry.toolInt == toolInt) {
      toolString = entry.gateName;
      gateTypeString = entry.gateTypeString;
      gateType = gateTypeString;
      break;
    }
  }

}
