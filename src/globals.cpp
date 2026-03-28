#include "globals.h"

// definitions for external globals declared in globals.h

// hardware pin defaults and constants
int ANALOG_PIN_IN = 32;
byte flash = 0;
byte boardIdByte = 0;
byte boardConfig = 0;
byte boardVer = 0;
byte analogPinInByte = 0;
byte limitSwitchPin = 19;
byte reedRelayPin = 23;
byte greenLEDpin = 25;
byte redLEDpin = 26;
byte gateOn = 16;
byte gateOff = 27;
byte linkPin = 13;
byte stepPin = 16;
byte dirPin = 4;
byte enablePin = 17;
int switchPinA = 5;
int servoPinA = 5;
int switchPinB = 5;
int servoPinB = 5;

// MQTT/connect variables
int toolInt = 0;
char checkInID[30] = "";
int mIDlen = 0;
char machineIDChar[3] = "";
char mID[4] = "";
String mIDstring;
char gateName[51] = "";
const char* BGstatus = nullptr;
String gateNameString;
String gateTypeString;
int keepAlive = 70;
char availabilityTopic[50] = "";
char BGtopic[40] = "";
String rssiTopic;
String machineIDString;
String setupID;
GateState gateState = STATE_UNKNOWN;
bool rotation = false;
String urlFinal;
bool eepromUpdate = false;   // flag used to trigger EEPROM update 
char cycleTopic[15] = "";
const char* ver = "03_27_2026";  // MM_DD_YYYY

// Working Variables
int sensorIn = 0;
int sensor = 0;
int stepPosition = 0;
String trace;
int countDown = 0;
String toolString;
String ipa;
String mac;
int wifiDB = 0;
char charWifiDB[10] = "";
String verString;
String machineIDstring;
int fullRunSteps = 0;
byte eCode = 0;
const char* db = nullptr;
const char* dbNew = nullptr;

// Parameters
const int waitTime = 250;
const int backSteps = 50;  //steps required to open switch after homing
const int stepsPerRevolution = 200;
const int holdTime = 500;  // time to stablize each cycle (microseconds)
const int displayTime = 0;
const int timeout = 120;

unsigned long closeDelayTime = 0;
unsigned long currentTime = 0;
unsigned long gateOpenTime = 0;
unsigned long startTime = 0;
unsigned long closeTime = 0;
unsigned long onTime = 0;
unsigned long offTime = 0;

unsigned long x = 0;
unsigned runTime = 0;
unsigned long currentMillis = 0;
unsigned long lastMsgTime = 0;
unsigned int interval = 0;
unsigned int maxInterval = 0;
unsigned int staticDelta = 0;
int lastSample = 0;
float staticPressure = 0.0;
float inchesH2O = 0.0;
String inchesH2O_str;
char buffer[10] = "";
int delta = 0;

// Status Flags
volatile bool manualState = false;
volatile bool limitSwitchState = false;
volatile bool gateCloseState = false;
volatile bool gateOpenState = false;
volatile bool moveState = false;
volatile bool otaOn = false;
bool oledReady = false;
int manualStatus = 0;
int flag = 0;

// Data
float toolRunTime = 0.0;
long toolTimer = 0;
int toolCycles = 0;
int loopCount = 0;

String boardID;
int intboardID = 0;
String machineID;
int gateDelaySeconds = 0;
int bDoubleTriggerMs = 1000;
int closeSwitchDebounceMs = 250;
int openA = 120;
int openB = 120;
int closedA = 40;
int closedB = 40;
String gateType;

// human-readable board type from configuration table
const char* boardTypeName = "";

const char* mqtt_server = nullptr;
const char* mqtt_serverAlt = nullptr;
const char* ssid = nullptr;
const char* password = nullptr;
const char* ssidAlt = nullptr;
const char* passwordAlt = nullptr;

int trigger = 0;
int triggerDelta = 0;
int maxMissedSteps = 0;
int delayTime = 0;

String GOOGLE_SCRIPT_ID;
String Google_Script_Master;
String GOOGLE_SCRIPT_Boot;

// Networking objects
WiFiClient espClient;
PubSubClient client(espClient);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
