#include <ESP8266WebServer.h>
#include "WifiClient.h"
#include "MqttClient.h"
#include "HCSR04.h"

// WiFi Configuration
const char* ssid = "Sang";
const char* password = "17719897975";

// MQTT Configuration
const char* mqtt_broker = "192.168.219.81";
const int mqtt_port = 1883;
const char* control_topic = "esp32cam/control";

// HC-SR04 Configuration
const int trigPin = D5;
const int echoPin = D6;
const float activation_distance = 15.0; // cm threshold

// Mode Control
bool autoMode = true;
bool objectDetected = false;

// Client objects
WifiClient wifiClient((char*)ssid, (char*)password);
HCSR04 distanceSensor(trigPin, echoPin);
String subscribe_topics[] = {""}; // No subscriptions for controller
MqttClient mqttClient(mqtt_broker, mqtt_port, subscribe_topics, 0);

// Web server
ESP8266WebServer server(80);

// ========== HTTP HANDLERS ==========

void handleRoot() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/plain", "ESP8266 Cam Controller Ready");
}

void handleSetMode() {
  String mode = server.arg("mode");

  if (mode == "manual") {
    if (autoMode) {
      mqttClient.publish_message(control_topic, "STOP");
      Serial.println("Sent STOP due to manual mode switch");
    }
    autoMode = false;
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "text/plain", "Switched to MANUAL mode");
  } else if (mode == "auto") {
    autoMode = true;
    objectDetected = false; // reset detection state
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "text/plain", "Switched to AUTO mode");
  } else {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(400, "text/plain", "Invalid mode");
  }
}

void setupWebServer() {
  server.on("/", HTTP_GET, handleRoot);
  server.on("/setMode", HTTP_GET, handleSetMode);
  server.begin();
  Serial.println("HTTP server started");
}

// ========== SETUP ==========

void setup() {
  Serial.begin(115200);
  
  // Connect WiFi
  wifiClient.connect();

  // Connect MQTT
  mqttClient.connect("ESP8266Controller");

  // Web server
  setupWebServer();

  Serial.println("System ready (default: AUTO mode)");
}


void loop() {
  mqttClient.check_connection("ESP8266Controller");
  server.handleClient();

  if (!autoMode) {
    delay(1000); // Save power / reduce CPU usage in manual mode
    return;
  }

  float distance = distanceSensor.calculate_distance();
    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.println(" cm");

  if (distance < activation_distance && !objectDetected) {
    Serial.println("CAM START in delay");
    delay(4000);
    mqttClient.publish_message(control_topic, "START");
    objectDetected = true;
    Serial.println("Published START command");
  }

  if (distance >= activation_distance && objectDetected) {
    mqttClient.publish_message(control_topic, "STOP");
    objectDetected = false;
    Serial.println("Published STOP command");
  }

  delay(500);
}
