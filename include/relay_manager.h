#ifndef RELAY_MANAGER_H
#define RELAY_MANAGER_H

#include <Arduino.h>

void relayInit();

void setRelay(uint8_t relay, bool state);

bool getRelayState(uint8_t relay);

#endif