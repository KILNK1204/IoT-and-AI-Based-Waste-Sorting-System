#include "WifiClient.h"

WifiClient::WifiClient(const char* ssid, const char* password) 
    : _ssid(ssid), _password(password) {}

void WifiClient::connect() {
    Serial.println("Connecting to WiFi...");
    WiFi.begin(_ssid, _password);
    WiFi.setSleep(false);
    
    while (!isConnected()) {
        delay(500);
        Serial.print(".");
    }
    
    Serial.println("\nConnected!");
    Serial.print("IP Address: ");
    Serial.println(getIP());
}

void WifiClient::disconnect() {
    WiFi.disconnect(true);
    Serial.println("WiFi disconnected");
}

bool WifiClient::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

IPAddress WifiClient::getIP() {
    return WiFi.localIP();
}