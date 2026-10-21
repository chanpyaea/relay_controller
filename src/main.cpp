#include <Arduino.h>

#include "wifi_manager.h"
#include "mqtt_manager.h"
#include "relay_manager.h"
#include "web_server.h"

void setup()
{
    Serial.begin(115200);

    relayInit();

    wifiInit();

    mqttInit();

    webServerInit();

    Serial.println("System Ready");
}

void loop()
{
    wifiLoop();

    mqttLoop();
}