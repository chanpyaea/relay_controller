#include <Arduino.h>

#include "config.h"
#include "mqtt_manager.h"
#include "water_level.h"

namespace
{
bool floatStates[4] = {false, false, false, false};
const char* waterLevelState = "LOW";
uint8_t waterLevelPercentage = 0;
unsigned long lastCheck = 0;

void readWaterLevel(bool publishChange)
{
    const char* previousState = waterLevelState;
    const uint8_t previousPercentage = waterLevelPercentage;

    floatStates[0] = digitalRead(FLOAT1) == LOW;
    floatStates[1] = digitalRead(FLOAT2) == LOW;
    floatStates[2] = digitalRead(FLOAT3) == LOW;
    floatStates[3] = digitalRead(FLOAT4) == LOW;

    if(floatStates[0] && floatStates[1])
    {
        waterLevelState = "FULL";
        waterLevelPercentage = 100;
    }
    else if(!floatStates[0] && floatStates[1])
    {
        waterLevelState = "MIDDLE";
        waterLevelPercentage = 50;
    }
    else
    {
        waterLevelState = "LOW";
        waterLevelPercentage = 0;
    }

    if(publishChange &&
       (strcmp(previousState, waterLevelState) != 0 ||
        previousPercentage != waterLevelPercentage))
    {
        Serial.printf("Water level: %s (%u%%)\n", waterLevelState, waterLevelPercentage);
        mqttPublishWaterLevel(waterLevelState, waterLevelPercentage);
    }
}
}

void waterLevelInit()
{
    pinMode(FLOAT1, INPUT);
    pinMode(FLOAT2, INPUT);
    pinMode(FLOAT3, INPUT);
    pinMode(FLOAT4, INPUT);
    readWaterLevel(false);
}

void waterLevelLoop()
{
    if(millis() - lastCheck < WATER_LEVEL_CHECK_INTERVAL_MS)
    {
        return;
    }

    lastCheck = millis();
    readWaterLevel(true);
}

const char* getWaterLevelState()
{
    return waterLevelState;
}

uint8_t getWaterLevelPercentage()
{
    return waterLevelPercentage;
}

bool getFloatState(uint8_t floatNumber)
{
    return floatNumber >= 1 && floatNumber <= 4
        ? floatStates[floatNumber - 1]
        : false;
}
