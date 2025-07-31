#ifndef WIFI_CLIENT_H
#define WIFI_CLIENT_H

#include <WiFi.h>

class WifiClient {
public:
    WifiClient(const char* ssid, const char* password);
    void connect();
    void disconnect();
    bool isConnected();
    IPAddress getIP();

private:
    const char* _ssid;
    const char* _password;
};

#endif