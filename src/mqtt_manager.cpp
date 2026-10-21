#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#include "config.h"
#include "relay_manager.h"
#include "ac_monitor.h"

WiFiClient espClient;

PubSubClient mqttClient(espClient);

unsigned long mqttReconnectTimer = 0;

// Forward declarations
void mqttPublishState(uint8_t relay, bool state);
void mqttPublishDiscovery();
void mqttPublishElectricityState(bool state);

void mqttCallback(char* topic, byte* payload, unsigned int length)
{
    String message;

    for(unsigned int i = 0; i < length; i++)
    {
        message += (char)payload[i];
    }

    Serial.print("MQTT Message: ");
    Serial.println(message);

    if(String(topic) == String(MQTT_BASE_TOPIC) + "/relay/1/set")
    {
        setRelay(1, message == "ON");
        mqttPublishState(1, message == "ON");
    }

    if(String(topic) == String(MQTT_BASE_TOPIC) + "/relay/2/set")
    {
        setRelay(2, message == "ON");
        mqttPublishState(2, message == "ON");
    }

    if(String(topic) == String(MQTT_BASE_TOPIC) + "/relay/3/set")
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
        for(uint8_t i = 1; i <= 3; i++)
        {
            const String commandTopic = String(MQTT_BASE_TOPIC) +
                "/relay/" + String(i) + "/set";
            mqttClient.subscribe(commandTopic.c_str());
        }

        // Publish current relay states
        for(uint8_t i = 1; i <= 3; i++)
        {
            mqttPublishState(i, getRelayState(i));
        }

        mqttPublishElectricityState(isElectricityOn());
    }
}

void mqttInit()
{
    mqttClient.setServer(MQTT_BROKER, MQTT_PORT);

    // Home Assistant discovery JSON plus its topic is larger than
    // PubSubClient's default 256-byte packet buffer.
    if(!mqttClient.setBufferSize(MQTT_BUFFER_SIZE))
    {
        Serial.println("ERROR: Could not allocate MQTT packet buffer");
    }

    mqttClient.setCallback(mqttCallback);
}

void mqttPublishState(uint8_t relay, bool state)
{
    if(!mqttClient.connected())
    {
        return;
    }

    String stateTopic = String(MQTT_BASE_TOPIC) +
        "/relay/" + String(relay) + "/state";
    String statePayload = state ? "ON" : "OFF";

    mqttClient.publish(stateTopic.c_str(), statePayload.c_str(), true);

    Serial.print("Published state: ");
    Serial.print(stateTopic);
    Serial.print(" = ");
    Serial.println(statePayload);
}

void mqttPublishElectricityState(bool state)
{
    if(!mqttClient.connected())
    {
        return;
    }

    const char* payload = state ? "ON" : "OFF";
    mqttClient.publish(AC_STATE_TOPIC, payload, true);
    Serial.printf("Published electricity state: %s = %s\n", AC_STATE_TOPIC, payload);
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
        doc["cmd_t"] = String(MQTT_BASE_TOPIC) +
            "/relay/" + String(i) + "/set";
        doc["stat_t"] = String(MQTT_BASE_TOPIC) +
            "/relay/" + String(i) + "/state";
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

        const bool published = mqttClient.publish(
            discoveryTopic.c_str(), payload.c_str(), true);
        Serial.printf("%s discovery: %s (%u bytes)\n",
                      published ? "Published" : "FAILED to publish",
                      discoveryTopic.c_str(),
                      static_cast<unsigned int>(payload.length()));
    }

    // This is a separate read-only entity on the existing device.  Its unique
    // ID and discovery topic do not overlap with any of the relay switches.
    JsonDocument acDoc;
    acDoc["name"] = "Electricity Supply";
    acDoc["unique_id"] = String(HA_DEVICE_ID) + "_ac_power";
    acDoc["stat_t"] = AC_STATE_TOPIC;
    acDoc["avty_t"] = HA_AVAILABILITY_TOPIC;
    acDoc["pl_on"] = "ON";
    acDoc["pl_off"] = "OFF";
    acDoc["pl_avail"] = "online";
    acDoc["pl_not_avail"] = "offline";
    acDoc["device_class"] = "power";
    acDoc["device"]["identifiers"][0] = HA_DEVICE_ID;
    acDoc["device"]["name"] = HA_DEVICE_NAME;
    acDoc["device"]["manufacturer"] = "DIY";
    acDoc["device"]["model"] = "ESP32 Relay Module";

    String acPayload;
    serializeJson(acDoc, acPayload);
    const String acDiscoveryTopic = String(HA_DISCOVERY_PREFIX) +
        "/binary_sensor/" + String(HA_DEVICE_ID) + "_ac_power/config";
    const bool acPublished = mqttClient.publish(
        acDiscoveryTopic.c_str(), acPayload.c_str(), true);
    Serial.printf("%s discovery: %s (%u bytes)\n",
                  acPublished ? "Published" : "FAILED to publish",
                  acDiscoveryTopic.c_str(),
                  static_cast<unsigned int>(acPayload.length()));
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
