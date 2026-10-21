#include <WiFi.h>
#include "config.h"

unsigned long wifiReconnectTimer = 0;

void wifiInit()
{
    WiFi.mode(WIFI_STA);

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    Serial.println("=================================");
    Serial.println("Connecting WiFi...");

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);

        Serial.print(".");
    }

    Serial.println("");
    Serial.println("=================================");
    Serial.println("WiFi Connected Successfully");
    Serial.println("=================================");

    Serial.print("Connected SSID : ");
    Serial.println(WiFi.SSID());

    Serial.print("ESP32 IP Address : ");
    Serial.println(WiFi.localIP());

    Serial.print("Signal RSSI : ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");

    Serial.print("ESP32 MAC : ");
    Serial.println(WiFi.macAddress());

    Serial.println("=================================");
}

void wifiLoop()
{
    if(WiFi.status() != WL_CONNECTED)
    {
        if(millis() - wifiReconnectTimer > 5000)
        {
            wifiReconnectTimer = millis();

            Serial.println("Reconnecting WiFi...");

            WiFi.disconnect();

            WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
        }
    }
}