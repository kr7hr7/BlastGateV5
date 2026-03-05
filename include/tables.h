#pragma once

#include <Arduino.h>

// forward declaration for the board lookup helper used by boardConfig.
// It reads the global boardIdByte and updates the related pin globals.
void applyBoardConfiguration();
