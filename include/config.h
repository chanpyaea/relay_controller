#ifndef CONFIG_H
#define CONFIG_H

// DEVICE IDENTITY
// Change only this value for each physical controller: "01", "02", "03", etc.
// Every MQTT topic and Home Assistant identifier below is derived from it.
#define DEVICE_ID_SUFFIX       "02"
#define HA_DEVICE_ID           "esp32_relay_" DEVICE_ID_SUFFIX
#define HA_DEVICE_NAME         "ESP32 Relay Controller " DEVICE_ID_SUFFIX
#define MQTT_CLIENT_ID         "ESP32_RELAY_" DEVICE_ID_SUFFIX
#define MQTT_BASE_TOPIC        "home/" HA_DEVICE_ID

// WIFI
#define WIFI_SSID       "DET"
#define WIFI_PASSWORD   "1313373290"

// MQTT
#define MQTT_BROKER     "192.168.100.30"
#define MQTT_PORT       1883
#define MQTT_BUFFER_SIZE 768
#define MQTT_USERNAME   "mqtt-user"
#define MQTT_PASSWORD   "asdffdsa"  // Change this to your HA MQTT password
// HOME ASSISTANT MQTT DISCOVERY
#define HA_DISCOVERY_PREFIX  "homeassistant"
#define HA_AVAILABILITY_TOPIC MQTT_BASE_TOPIC "/availability"

// AC MAINS SENSING (pulse output from the H_PF circuit)
#define AC_SENSE_PIN          25
#define AC_CHECK_INTERVAL_MS  1000
#define AC_MIN_PULSES_PER_CHECK 1
#define AC_ON_CONFIRM_CHECKS  3
#define AC_OFF_CONFIRM_CHECKS 3
#define AC_STATE_TOPIC        MQTT_BASE_TOPIC "/electricity/state"
// RELAY GPIO
#define RELAY1_PIN 5
#define RELAY2_PIN 18
#define RELAY3_PIN 19

#endif
