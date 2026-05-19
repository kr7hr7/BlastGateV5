#include "settings.h"
#include "globals.h"

void settings() {
  boardID               = "42";
  machineID             = "00";
  gateDelaySeconds      = 05;
  closeSwitchDebounceMs = 250;


  bDoubleTriggerMs      = 750;

  openA                 = 125;
  openB                 = 125;
  closedA               = 35;
  closedB               = 40;
 
  eepromUpdate          = false;



  mqtt_server            = "192.168.7.168";
  mqtt_serverAlt         = "192.168.4.99";
  maxInterval            = 15000;  // this is the max between static pressure sample uploads - default 900000 = 15 minutes
  interval               = 500;       //  This variable sets the frequency of static pressure data being sampled
  staticDelta            = 25;    //   The amount change required to trigger a mqtt sample
  ssid                   = "SDFWA";
  password               = "woodworking";
  ssidAlt                = "kr7hr8";
  passwordAlt            = "8584354826";
  trigger                = 400;  //A/D threshold from current sensor pin to determine tool-on condition
  triggerDelta           = 200;
  maxMissedSteps         = 2000; // the }max number of steps that can be missed on homing before stopping the stepper
  openDelayTime          = 75;   // open-only step pulse delay (lower = faster)
  delayTime              = 85;   // close/homing step pulse delay: lower=faster higher=slower
  //rotation               = true;
  
  GOOGLE_SCRIPT_Boot   = "https://script.google.com/macros/s/AKfycbzuAAYRi9SCfKLZUx0ZxZI88JoB08p8I9PuNVDV2ve5m2nUU8vzUEZ7BRLSp53GSLSXSg/exec?";

}