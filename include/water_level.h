#ifndef WATER_LEVEL_H
#define WATER_LEVEL_H

#include <Arduino.h>

void waterLevelInit();
void waterLevelLoop();
const char* getWaterLevelState();
uint8_t getWaterLevelPercentage();
bool getFloatState(uint8_t floatNumber);

#endif
