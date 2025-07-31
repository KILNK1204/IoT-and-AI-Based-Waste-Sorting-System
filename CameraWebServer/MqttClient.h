#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

#include <PubSubClient.h>
#include <WiFiClient.h>

class MqttClient {
public:
    MqttClient(const char* server, uint16_t port);
    void connect(const char* clientId);
    void publish(const char* topic, const char* message);
    void subscribe(const char* topic);
    void loop();
    void setCallback(void (*callback)(char*, byte*, unsigned int));

private:
    WiFiClient _wifiClient;
    PubSubClient _client;
};

#endif