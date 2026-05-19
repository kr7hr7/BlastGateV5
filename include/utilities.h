#pragma once
#include "globals.h"

// utility function declarations

// MQTT callback handler for incoming messages
void mqttMessageCallback(char* topic, byte* payload, unsigned int length);

// Display "Reconnect" message on OLED
void displayReconnectMessage();
