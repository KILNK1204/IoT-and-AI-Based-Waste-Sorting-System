#include <ESP8266WiFi.h>
#include "WifiClient.h"



WifiClient::WifiClient(char *ssid, char *pass)
{
    _ssid = ssid;
    _pass = pass;
}

void WifiClient::connect()
{
    WiFiClient wifiClient;
    Serial.println("Connecting to WPA SSID [" + String(_ssid) + "]...");

    WiFi.begin(_ssid, _pass);

    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(500);
    }

    IPAddress ip = WiFi.localIP();
    Serial.print(" done, IP: ");
    Serial.println(ip);
}

void WifiClient::disconnect()
{
    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println("Disconnecting from WiFi...");
        WiFi.disconnect();
        while (WiFi.status() == WL_CONNECTED)
        {
            delay(2000); // Wait until disconnected
        }
        Serial.println("Disconnected.");
    }
    else
    {
        Serial.println("WiFi is not currently connected.");
    }
}

