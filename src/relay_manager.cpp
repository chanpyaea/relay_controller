#include "relay_manager.h"
#include "config.h"
#include "mqtt_manager.h"

bool relayStates[3] = {false, false, false};

void relayInit()
{
    pinMode(RELAY1_PIN, OUTPUT);
    pinMode(RELAY2_PIN, OUTPUT);
    pinMode(RELAY3_PIN, OUTPUT);

    digitalWrite(RELAY1_PIN, LOW);
    digitalWrite(RELAY2_PIN, LOW);
    digitalWrite(RELAY3_PIN, LOW);
}

void setRelay(uint8_t relay, bool state)
{
    relayStates[relay - 1] = state;

    switch(relay)
    {
        case 1:
            digitalWrite(RELAY1_PIN, state);
            break;

        case 2:
            digitalWrite(RELAY2_PIN, state);
            break;

        case 3:
            digitalWrite(RELAY3_PIN, state);
            break;
    }

    // Publish state change to MQTT
    mqttPublishState(relay, state);
}

bool getRelayState(uint8_t relay)
{
    return relayStates[relay - 1];
}