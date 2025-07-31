# IoT-and-AI-Based-Waste-Sorting-System
This project is an AI-powered smart waste sorting system developed as a term project at the University of Toronto. It integrates cloud computing, computer vision, and IoT technologies to automatically classify and sort waste into Recyclable, Organic, and Non-Recyclable categories.

## Features

- Real-time image capture using ESP32-CAM
- Cloud-based image classification (MobileNetV2)
- Automatic and manual sorting modes
- Web interface for monitoring and control
- MQTT-based communication with servo-controlled actuators

## Hardware Components

- ESP32-CAM module
- HC-SR04 ultrasonic distance sensor
- FS90R servo motors (pipe and lid control)
- Raspberry Pi (MQTT broker)
- Arduino-compatible microcontrollers

## Cloud Components

- Google Cloud Run for classification API
- Google Cloud Storage for image logging
- MobileNetV2 trash classification model (Recyclable, Organic, Non-Recyclable)

- ## Watch our system in action (Requires UofT account):  
📹 [Demo Video](https://play.library.utoronto.ca/watch/f9eee6b63e604408a42305ec975bc442)

- ## Authors

- **Zhaoheng Li** 
- **Songyuan Sang**  
- **Qianyue Xue** 
