#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

#include <Arduino.h>

void mqttInit();

void mqttLoop();

void mqttPublishState(uint8_t relay, bool state);

void mqttPublishDiscovery();

void mqttPublishElectricityState(bool state);

void mqttPublishWaterLevel(const char* state, uint8_t percentage);

#endif
