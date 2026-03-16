#include "settings.h"
#include "globals.h"

void settings() {

  boardID               = "48";
  machineID             = "25";
  gateDelaySeconds      = 15;
  bDoubleTriggerMs      = 750;
  openA                 = 120;
  openB                 = 120;
  closedA               = 35;
  closedB               = 40;
 
  eepromUpdate          = false;



  mqtt_server            = "192.168.7.168";
  mqtt_serverAlt         = "192.168.4.99";
  maxInterval            = 15000;  // this is the max between static pressure sample uploads - default 900000 = 15 minutes
  interval               = 500;       //  This variable sets the frequency of static pressure data being sampled
  staticDelta            = 100;    //   The amount change required to trigger a mqtt sample
  ssid                   = "SDFWA";
  password               = "woodworking";
  ssidAlt                = "kr7hr8";
  passwordAlt            = "8584354826";
  trigger                = 400;  //A/D threshold from current sensor pin to determine tool-on condition
  triggerDelta           = 200;
  maxMissedSteps         = 2000; // the max number of steps that can be missed on homing before stopping the stepper
  delayTime              = 85;   // hold time between steps lower=faster higher=slower. 200-400 for 4988 drivers,  75-125 for TMC2209 drivers
  

  GOOGLE_SCRIPT_Boot   = "https://script.google.com/macros/s/AKfycbzuAAYRi9SCfKLZUx0ZxZI88JoB08p8I9PuNVDV2ve5m2nUU8vzUEZ7BRLSp53GSLSXSg/exec?";

}