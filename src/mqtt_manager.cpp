#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#include "config.h"
#include "relay_manager.h"

WiFiClient espClient;

PubSubClient mqttClient(espClient);

unsigned long mqttReconnectTimer = 0;

// Forward declarations
void mqttPublishState(uint8_t relay, bool state);
void mqttPublishDiscovery();

void mqttCallback(char* topic, byte* payload, unsigned int length)
{
    String message;

    for(unsigned int i = 0; i < length; i++)
    {
        message += (char)payload[i];
    }

    Serial.print("MQTT Message: ");
    Serial.println(message);

    if(String(topic) == "home/relay/1/set")
    {
        setRelay(1, message == "ON");
        mqttPublishState(1, message == "ON");
    }

    if(String(topic) == "home/relay/2/set")
    {
        setRelay(2, message == "ON");
        mqttPublishState(2, message == "ON");
    }

    if(String(topic) == "home/relay/3/set")
    {
        setRelay(3, message == "ON");
        mqttPublishState(3, message == "ON");
    }
}

void mqttReconnect()
{
    if(mqttClient.connect(MQTT_CLIENT_ID, MQTT_USERNAME, MQTT_PASSWORD))
    {
        Serial.println("MQTT Connected");

        // Publish availability and discovery
        mqttClient.publish(HA_AVAILABILITY_TOPIC, "online", true);
        mqttPublishDiscovery();

        // Subscribe to relay commands
        mqttClient.subscribe("home/relay/1/set");
        mqttClient.subscribe("home/relay/2/set");
        mqttClient.subscribe("home/relay/3/set");

        // Publish current relay states
        for(uint8_t i = 1; i <= 3; i++)
        {
            mqttPublishState(i, getRelayState(i));
        }
    }
}

void mqttInit()
{
    mqttClient.setServer(MQTT_BROKER, MQTT_PORT);

    mqttClient.setCallback(mqttCallback);
}

void mqttPublishState(uint8_t relay, bool state)
{
    if(!mqttClient.connected())
    {
        return;
    }

    String stateTopic = "home/relay/" + String(relay) + "/state";
    String statePayload = state ? "ON" : "OFF";

    mqttClient.publish(stateTopic.c_str(), statePayload.c_str(), true);

    Serial.print("Published state: ");
    Serial.print(stateTopic);
    Serial.print(" = ");
    Serial.println(statePayload);
}

void mqttPublishDiscovery()
{
    if(!mqttClient.connected())
    {
        return;
    }

    // Publish discovery for each relay
    for(uint8_t i = 1; i <= 3; i++)
    {
        JsonDocument doc;

        doc["name"] = "Relay " + String(i);
        doc["unique_id"] = String(HA_DEVICE_ID) + "_relay_" + String(i);
        doc["cmd_t"] = "home/relay/" + String(i) + "/set";
        doc["stat_t"] = "home/relay/" + String(i) + "/state";
        doc["avty_t"] = HA_AVAILABILITY_TOPIC;
        doc["pl_on"] = "ON";
        doc["pl_off"] = "OFF";
        doc["pl_avail"] = "online";
        doc["pl_not_avail"] = "offline";

        doc["device"]["identifiers"][0] = HA_DEVICE_ID;
        doc["device"]["name"] = HA_DEVICE_NAME;
        doc["device"]["manufacturer"] = "DIY";
        doc["device"]["model"] = "ESP32 Relay Module";

        String discoveryTopic = String(HA_DISCOVERY_PREFIX) + "/switch/" + String(HA_DEVICE_ID) + "_relay_" + String(i) + "/config";

        String payload;
        serializeJson(doc, payload);

        mqttClient.publish(discoveryTopic.c_str(), payload.c_str(), true);

        Serial.print("Published discovery: ");
        Serial.println(discoveryTopic);
    }
}

void mqttLoop()
{
    if(WiFi.status() != WL_CONNECTED)
    {
        return;
    }

    if(!mqttClient.connected())
    {
        if(millis() - mqttReconnectTimer > 5000)
        {
            mqttReconnectTimer = millis();

            Serial.println("Reconnecting MQTT...");

            mqttReconnect();
        }
    }

    mqttClient.loop();
}