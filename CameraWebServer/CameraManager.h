#ifndef CAMERA_MANAGER_H
#define CAMERA_MANAGER_H

#include "esp_camera.h"

class CameraManager {
public:
    CameraManager();
    bool begin();
    void end();
    bool captureToBuffer(uint8_t** buf, size_t* len);
    bool isActive() const;

private:
    camera_config_t _config;
    bool _initialized = false;
};

#endif