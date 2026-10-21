#ifndef CONFIG_H
#define CONFIG_H

// WIFI
#define WIFI_SSID       "DET"
#define WIFI_PASSWORD   "1313373290"

// MQTT
#define MQTT_BROKER     "192.168.100.30"
#define MQTT_PORT       1883
#define MQTT_CLIENT_ID  "ESP32_RELAY"
#define MQTT_USERNAME   "mqtt-user"
#define MQTT_PASSWORD   "asdffdsa"  // Change this to your HA MQTT password
// HOME ASSISTANT MQTT DISCOVERY
#define HA_DISCOVERY_PREFIX  "homeassistant"
#define HA_DEVICE_NAME       "ESP32 Relay Controller"
#define HA_DEVICE_ID         "esp32_relay_01"
#define HA_AVAILABILITY_TOPIC "home/availability"
// RELAY GPIO
#define RELAY1_PIN 5
#define RELAY2_PIN 18
#define RELAY3_PIN 19

#endif