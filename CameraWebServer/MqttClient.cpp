#include "MqttClient.h"

MqttClient::MqttClient(const char* server, uint16_t port)
    : _client(_wifiClient) {
    _client.setServer(server, port);
}

void MqttClient::connect(const char* clientId) {
    Serial.print("Connecting to MQTT...");
    while (!_client.connect(clientId)) {
        Serial.print(".");
        delay(1000);
    }
    Serial.println(" connected!");
}

void MqttClient::publish(const char* topic, const char* message) {
    _client.publish(topic, message);
}

void MqttClient::subscribe(const char* topic) {
    _client.subscribe(topic);
}

void MqttClient::loop() {
    _client.loop();
}

void MqttClient::setCallback(void (*callback)(char*, byte*, unsigned int)) {
    _client.setCallback(callback);
}