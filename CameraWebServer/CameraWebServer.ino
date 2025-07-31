#include "WifiClient.h"
#include "MqttClient.h"
#include "CameraManager.h"
#include <WebServer.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

// Configuration
const char* WIFI_SSID = "Sang";
const char* WIFI_PASS = "17719897975";
const char* MQTT_SERVER = "192.168.219.81";
const uint16_t MQTT_PORT = 1883;
const char* MQTT_TOPIC = "esp32cam/control";
const char* CLOUD_ENDPOINT = "https://trashclassification-849464200647.us-central1.run.app/";

unsigned long lastCaptureTime = 0;
bool cameraActive = false;

// Globals
WifiClient wifiClient(WIFI_SSID, WIFI_PASS);
MqttClient mqttClient(MQTT_SERVER, MQTT_PORT);
CameraManager camera;
WebServer server(80);
uint8_t* jpgBuf = nullptr;
size_t jpgBufLen = 0;

// Function to upload image to cloud
bool uploadToCloud(uint8_t* imageBuf, size_t bufLen) {
    WiFiClientSecure *client = new WiFiClientSecure;
    HTTPClient https;
    
    // Configure TLS (might need adjustment for your certificate)
    client->setInsecure(); // Bypass SSL verification - consider proper cert for production
    
    if (https.begin(*client, CLOUD_ENDPOINT)) {
        // Create the multipart form data
        String boundary = "----WebKitFormBoundary" + String(micros());
        String header = "--" + boundary + "\r\nContent-Disposition: form-data; name=\"file\"; filename=\"esp32cam.jpg\"\r\nContent-Type: image/jpeg\r\n\r\n";
        String footer = "\r\n--" + boundary + "--\r\n";
        
        // Calculate total length and prepare headers
        size_t totalLength = header.length() + bufLen + footer.length();
        https.addHeader("Content-Type", "multipart/form-data; boundary=" + boundary);
        
        // Combine all parts into one buffer
        uint8_t* combinedData = (uint8_t*)malloc(totalLength);
        if (!combinedData) {
            Serial.println("Failed to allocate memory for combined data");
            https.end();
            delete client;
            return false;
        }
        
        // Copy all parts into the combined buffer
        memcpy(combinedData, header.c_str(), header.length());
        memcpy(combinedData + header.length(), imageBuf, bufLen);
        memcpy(combinedData + header.length() + bufLen, footer.c_str(), footer.length());
        
        // Send the POST request
        int httpCode = https.POST(combinedData, totalLength);
        free(combinedData); // Free the allocated memory
        
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
            String payload = https.getString();
            Serial.println("Cloud response:");
            Serial.println(payload);
            https.end();
            delete client;
            return true;
        }
        
        Serial.printf("HTTP error: %s\n", https.errorToString(httpCode).c_str());
        https.end();
    }
    delete client;
    return false;
}

// MQTT callback
void mqttCallback(char* topic, byte* payload, unsigned int length) {
    String message;
    for (unsigned int i = 0; i < length; i++) {
        message += (char)payload[i];
    }

    if (message == "START") {
        if (!cameraActive && camera.begin()) {
            cameraActive = true;
            bool success = false;

            // Discard first 3 images
            for (int i = 0; i < 6; i++) {
                uint8_t* tempBuf;
                size_t tempLen;
                bool result = camera.captureToBuffer(&tempBuf, &tempLen);

                if (i == 5 && result) {
                    // Only store the 4th image
                    jpgBuf = tempBuf;
                    jpgBufLen = tempLen;
                    success = true;
                    
                    // Upload to cloud
                    if (uploadToCloud(jpgBuf, jpgBufLen)) {
                        mqttClient.publish(MQTT_TOPIC, "Image uploaded to cloud");
                    } else {
                        mqttClient.publish(MQTT_TOPIC, "Cloud upload failed");
                    }
                } else if (tempBuf) {
                    // Free unused buffer (if CameraManager allocates dynamically)
                    free(tempBuf);
                }
                delay(100); // slight delay between frames
            }
        }
    }
    else if (message == "STOP") {
        if (cameraActive) {
            camera.end();
            cameraActive = false;
            delay(500); // Add small delay for hardware to reset
            mqttClient.publish(MQTT_TOPIC, "Camera stopped");
        }
    }
}

// HTTP: Return JPEG image
void handleImage() {
    if (jpgBuf != nullptr) {
        server.sendHeader("Access-Control-Allow-Origin", "*");
        server.send_P(200, "image/jpeg", (const char*)jpgBuf, jpgBufLen);
    } else {
        server.sendHeader("Access-Control-Allow-Origin", "*");
        server.send(500, "text/plain", "No frame available");
    }
}

// HTTP: Return status
void handleStatus() {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "text/plain", camera.isActive() ? "ACTIVE" : "STANDBY");
}

// Optional: Root handler (just a notice)
void handleRoot() {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "text/plain", "This device serves only API endpoints. Use external webpage.");
}

void setup() {
    Serial.begin(115200);

    // Connect to WiFi and MQTT
    wifiClient.connect();
    mqttClient.setCallback(mqttCallback);
    mqttClient.connect("ESP32CAM");
    mqttClient.subscribe(MQTT_TOPIC);

    // HTTP API routes
    server.on("/", HTTP_GET, handleRoot);
    server.on("/image", HTTP_GET, handleImage);
    server.on("/status", HTTP_GET, handleStatus);
    server.begin();

    Serial.println("System ready (camera inactive)");
}

void loop() {
    mqttClient.loop();
    server.handleClient();

    // Reconnect if needed
    if (!wifiClient.isConnected()) {
        wifiClient.connect();
        mqttClient.connect("ESP32CAM");
    }
}