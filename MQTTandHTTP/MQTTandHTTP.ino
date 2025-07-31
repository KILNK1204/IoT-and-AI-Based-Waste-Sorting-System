/*
MQTT: ip=rpi ip
mosquitto_pub -h 192.168.43.199 -t "test/topic" -m "{\"class\":\"Recyclable\",\"confidence\":0.916273,\"image_url\":\"gs://run-sources-ece-1528-group-us-central1/image-storage/1743356727_test.jpg\",\"success\":true}"
HTTP: ip = esp ip
curl -X POST http://192.168.43.17/post -d "message=Hello ESP!"
*/
#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// WiFi Credentials
const char* ssid = "ASGARD";
const char* password = "xueqianyue";

// MQTT Broker
const char* mqtt_server = "192.168.43.199";

// WiFi & MQTT Clients
WiFiClient espClient;
PubSubClient client(espClient);
AsyncWebServer server(80);

// MQTT Message Callback Function
void callback(char* topic, byte* payload, unsigned int length) {
    Serial.print("MQTT Message Received on Topic: ");
    Serial.println(topic);

    // Create a buffer for the payload
    char message[length + 1];
    memcpy(message, payload, length);
    message[length] = '\0';
    
    Serial.print("Raw Message: ");
    Serial.println(message);

    // Parse JSON
    StaticJsonDocument<512> doc; // Adjust size if needed
    DeserializationError error = deserializeJson(doc, message);

    if (error) {
        Serial.print("JSON Parsing Failed: ");
        Serial.println(error.c_str());
        return;
    }

    // Extract values
    const char* detected_class = doc["class"];
    float confidence = doc["confidence"];
    const char* image_url = doc["image_url"];
    bool success = doc["success"];

    if (detected_class) {
        Serial.println("----- Extracted Values -----");
        Serial.print("Class: "); Serial.println(detected_class);
        Serial.print("Confidence: "); Serial.println(confidence, 6);
        Serial.print("Image URL: "); Serial.println(image_url);
        Serial.print("Success: "); Serial.println(success ? "true" : "false");
    } else {
        Serial.println("Class not found in JSON");
    }

    Serial.println("------------------------");
}

// Reconnect to MQTT Broker
void reconnect() {
    while (!client.connected()) {
        Serial.print("Connecting to MQTT...");
        if (client.connect("ESP8266Client")) {
            Serial.println("Connected!");
            client.subscribe("test/topic");
        } else {
            Serial.print("Failed, rc=");
            Serial.print(client.state());
            Serial.println(" Retrying in 5s...");
            delay(5000);
        }
    }
}

void setup() {
    Serial.begin(115200);
    
    // Connect to WiFi
    Serial.print("Connecting to WiFi...");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }
    Serial.println("\nWiFi Connected!");
    Serial.print("ESP IP Address: ");
    Serial.println(WiFi.localIP());

    // Setup MQTT
    client.setServer(mqtt_server, 1883);
    client.setCallback(callback);

    // Setup HTTP Server
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "text/plain", "ESP D1 is listening!");
    });

    server.on("/post", HTTP_POST, [](AsyncWebServerRequest *request){
        if (request->hasParam("message", true)) {
            String message = request->getParam("message", true)->value();
            Serial.println("HTTP POST Received: " + message);
        }
        request->send(200, "text/plain", "Message received!");
    });

    server.begin();
}

void loop() {
    if (!client.connected()) {
        reconnect();
    }
    client.loop();
}

/*
#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>
#include <PubSubClient.h>

// WiFi Credentials
const char* ssid = "ASGARD";
const char* password = "xueqianyue";

// MQTT Broker
const char* mqtt_server = "192.168.43.199";


// WiFi & MQTT Clients
WiFiClient espClient;
PubSubClient client(espClient);
AsyncWebServer server(80);

// MQTT Message Callback Function
void callback(char* topic, byte* payload, unsigned int length) {
    Serial.print("MQTT Message Received on Topic: ");
    Serial.println(topic);

    Serial.print("Message: ");
    for (int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);  // Print each character
    }
    Serial.println("\n------------------------");
}

// Reconnect to MQTT Broker
void reconnect() {
    while (!client.connected()) {
        Serial.print("Connecting to MQTT...");
        if (client.connect("ESP8266Client")) {
            Serial.println("Connected!");
            client.subscribe("test/topic");  // Subscribe to topic
        } else {
            Serial.print("Failed, rc=");
            Serial.print(client.state());
            Serial.println(" Retrying in 5s...");
            delay(5000);
        }
    }
}

void setup() {
    Serial.begin(115200);
    
    // Connect to WiFi
    Serial.print("Connecting to WiFi...");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }
    Serial.println("\nWiFi Connected!");
    Serial.print("ESP IP Address: ");
    Serial.println(WiFi.localIP());

    // Setup MQTT
    client.setServer(mqtt_server, 1883);
    client.setCallback(callback);

    // Setup HTTP Server
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        Serial.println("HTTP GET Request Received");
        request->send(200, "text/plain", "ESP D1 is listening!");
    });

    server.on("/post", HTTP_POST, [](AsyncWebServerRequest *request){
        if (request->hasParam("message", true)) {
            String message = request->getParam("message", true)->value();
            Serial.println("HTTP POST Received: " + message);
        }
        request->send(200, "text/plain", "Message received!");
    });

    server.begin();
}

void loop() {
    if (!client.connected()) {
        reconnect();
    }
    client.loop();  // Handle MQTT messages
}
*/
