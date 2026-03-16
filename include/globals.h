#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include <EEPROM.h>
#include <ArduinoOTA.h>
#include <HTTPClient.h>

// gate state enumeration used for state machine logic
enum GateState {
    STATE_UNKNOWN = 0,
    STATE_CLOSED,
    STATE_CLOSING,
    STATE_OPEN,
    STATE_OPENING,
    STATE_ERROR_1,
    STATE_ERROR_2,
    STATE_ERROR_3
};

// helper for converting enum value to MQTT/string representation
inline const char *gateStateToString(GateState s) {
    switch (s) {
        case STATE_CLOSED: return "closed";
        case STATE_CLOSING: return "closing";
        case STATE_OPEN: return "open";
        case STATE_OPENING: return "opening";
        case STATE_ERROR_1: return "error_1";
        case STATE_ERROR_2: return "error_2";
        case STATE_ERROR_3: return "error_3";
        default: return "unknown";
    }
}

void errorState();
void prepNames();
void MQTTconnect();
void reportStaticPressue();
void keepMqttAlive(void* parameters);
void checkSwitchState();
void pingBroker(void* parameters);
void keepWiFiAlive(void* parameters);
void printStat();
void displayStat();
void updateSensorInOnOled(bool forceRedraw = false, bool pushDisplay = true);
void redrawSensorInOnOled(bool pushDisplay = true);
void servoControllerSetup();
void servoControllerLoop();
bool publishGateState(bool force = false);
bool setGateState(GateState newState, bool forcePublish = false);
void writeToBootLog();
//void write_to_google_sheet();
void reconnect();
void resetGate(void* parameters);
void tools();
void closeGate();
void openGate();
void homePosition();
void setupError();
void boardconfiguration();
void openGate();
void toolOn();
void toolOff();
void manualGateOpen();
void manualGateClose();
void eepromWrite();
// removed gatePinConfig in favor of table-based configuration
void gateTypeConfig();

// descriptive string pulled from board lookup table
extern const char* boardTypeName;
void OTA();
void WiFiConnect();
void settings();
void logCycle();
// void setupTasks(); // replaced by settings()



#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels
#define OLED_RESET -1     // Use shared reset; avoids conflict with servo pin assignments
#define SCREEN_ADDRESS 0x3C
#define I2C_SDA_PIN 21
#define I2C_SCL_PIN 22
#define WiFiTimeOutMS 5000
#define EEPROM_SIZE 20 

//                                     Pin Assignements
extern int       ANALOG_PIN_IN;
extern byte      flash;                 // 200 = flag that the board data has been stored
extern byte      boardIdByte;           // Integer to store in EEPROM
extern byte      boardConfig;           // Character to store in EEPROM
// 1= mac style stepper
// 2= Manual Gate
// 3= CNC & Cleanup Stepper
// 4= Presure Sensor Monitor
// 5= No gate control. Data logging only
extern byte      boardVer;
extern byte      analogPinInByte;
extern byte      limitSwitchPin;       
extern byte      reedRelayPin;        
extern byte      greenLEDpin;         
extern byte      redLEDpin;         
extern byte      gateOn;               // Controls H-Bridge
extern byte      gateOff;  // DO NOT use GPIO4/5 (boot straps); default moved to 27
extern byte      linkPin ;  // use pin 33 for ver 1-3   Use pin 13 for v4 plus
extern byte      stepPin ;  // 16 for combo boards   04 for earlier versions
extern byte      dirPin  ;  // avoid GPIO4/5; default now 14
extern byte      enablePin;  // 17 for combo boards  16 for early versions
extern int       switchPinA;
extern int       servoPinA;
extern int       switchPinB;
extern int       servoPinB;

//------- MQTTconnect variables-------------------
extern int       toolInt;
extern char      checkInID[30];  //  mID + " Checking In!" ;
extern int       mIDlen;
extern char      machineIDChar[3];
extern char      mID[4];  // The 3 digit machine Id   i.e. "M21"
extern String    mIDstring;
extern char      gateName[51];
extern const char* BGstatus;
extern String    gateNameString;
extern String    gateTypeString;
extern int       keepAlive;
extern char      availabilityTopic[50];
extern char      BGtopic[40];
extern String    rssiTopic;
extern String    machineIDString;
extern String    setupID;
extern GateState  gateState;
extern bool      rotation;  // To accomodate Rear or Front mount versions.  Mac(rear mounted) =false  Front mount(near the gate)=true
extern String    urlFinal;
extern bool      eepromUpdate;   // flag used to trigger EEPROM update 
extern char      cycleTopic[15];
extern const char*   ver;    // firmware version string

// Working Variables
extern int           sensorIn;
extern int           sensor;
extern int           stepPosition;
extern String        trace;
extern int           countDown;
extern String        toolString;
extern String        ipa;
extern String        mac;
extern int           wifiDB;
extern char          charWifiDB[10];
extern String        verString;
extern String        machineIDstring;
extern int           fullRunSteps;
extern byte          eCode;
extern const char*   db;
extern const char*   dbNew;

// Parameters
extern const int     waitTime;
extern const int     backSteps;
extern const int     stepsPerRevolution;
extern const int     holdTime;
extern const int     displayTime;
extern const int     timeout;

extern unsigned long closeDelayTime;
extern unsigned long currentTime;
extern unsigned long gateOpenTime;
extern unsigned long startTime;
extern unsigned long closeTime;
extern unsigned long onTime;
extern unsigned long offTime;

extern unsigned long x;
extern unsigned      runTime;
extern unsigned long currentMillis;
extern unsigned long lastMsgTime;
extern unsigned int  interval;
extern unsigned int  maxInterval;
extern unsigned int  staticDelta;
extern int           lastSample;
extern float         staticPressure;
extern float         inchesH2O;
extern String        inchesH2O_str;
extern char          buffer[10];
extern int           delta;

// Status Flags
extern volatile bool manualState;
extern volatile bool limitSwitchState;
extern volatile bool gateCloseState;
extern volatile bool gateOpenState;
extern volatile bool moveState;
extern volatile bool otaOn;
extern bool          oledReady;
extern int           manualStatus;
extern int           flag;

// Data
extern float         toolRunTime;
extern long          toolTimer;
extern int           toolCycles;
extern int           loopCount;

extern String        boardID;
extern int           intboardID;
extern String        machineID;
extern int           gateDelaySeconds;
extern int           bDoubleTriggerMs;
extern int           openA;
extern int           openB;
extern int           closedA;
extern int           closedB;
extern String        gateType;

extern const char*   mqtt_server;
extern const char*   mqtt_serverAlt;
extern const char*   ssid;
extern const char*   password;
extern const char*   ssidAlt;
extern const char*   passwordAlt;

extern int           trigger;
extern int           triggerDelta;
extern int           maxMissedSteps;
extern int           delayTime;

extern String        GOOGLE_SCRIPT_ID;
extern String        Google_Script_Master;
extern String        GOOGLE_SCRIPT_Boot;

extern WiFiClient espClient;
extern PubSubClient client;
extern Adafruit_SSD1306 display;















