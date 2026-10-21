// This file provides the relay_manager.h header for relay_manager.cpp
#ifndef RELAY_MANAGER_H
#define RELAY_MANAGER_H

#include <Arduino.h>

void relayInit();
void setRelay(uint8_t relay, bool state);
bool getRelayState(uint8_t relay);

#endif // RELAY_MANAGER_H
