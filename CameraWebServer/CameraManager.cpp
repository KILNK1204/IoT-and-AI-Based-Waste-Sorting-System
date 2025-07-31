#include "CameraManager.h"
#include <Arduino.h>
#include <cstring>

// Must define camera model before including camera_pins.h
#define CAMERA_MODEL_AI_THINKER
#include "camera_pins.h"

CameraManager::CameraManager() {
    // Initialize all camera configuration parameters
    _config.ledc_channel = LEDC_CHANNEL_0;
    _config.ledc_timer = LEDC_TIMER_0;
    _config.pin_d0 = Y2_GPIO_NUM;
    _config.pin_d1 = Y3_GPIO_NUM;
    _config.pin_d2 = Y4_GPIO_NUM;
    _config.pin_d3 = Y5_GPIO_NUM;
    _config.pin_d4 = Y6_GPIO_NUM;
    _config.pin_d5 = Y7_GPIO_NUM;
    _config.pin_d6 = Y8_GPIO_NUM;
    _config.pin_d7 = Y9_GPIO_NUM;
    _config.pin_xclk = XCLK_GPIO_NUM;
    _config.pin_pclk = PCLK_GPIO_NUM;
    _config.pin_vsync = VSYNC_GPIO_NUM;
    _config.pin_href = HREF_GPIO_NUM;
    _config.pin_sccb_sda = SIOD_GPIO_NUM;
    _config.pin_sccb_scl = SIOC_GPIO_NUM;
    _config.pin_pwdn = PWDN_GPIO_NUM;
    _config.pin_reset = RESET_GPIO_NUM;
    _config.xclk_freq_hz = 20000000;
    _config.frame_size = FRAMESIZE_VGA;
    _config.pixel_format = PIXFORMAT_JPEG;
    _config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
    _config.fb_location = CAMERA_FB_IN_PSRAM;
    _config.jpeg_quality = 14;
    _config.fb_count = 1;
}

void CameraManager::end() {
    if (_initialized) {
        esp_camera_deinit();
        
        // Only reset critical fields instead of entire config
        _config.xclk_freq_hz = 0;  // Mark as uninitialized
        _initialized = false;
        
        Serial.println("Camera deinitialized");
    }
}

bool CameraManager::begin() {
    if (_initialized) return true;
    
    // Reinitialize config if needed
    if (_config.xclk_freq_hz == 0) {
        CameraManager temp;  // Create temporary instance to get default config
        _config = temp._config;  // Copy the default configuration
    }

    // Rest of begin() remains the same...
    static bool gpio_isr_installed = false;
    if (!gpio_isr_installed) {
        esp_err_t err = gpio_install_isr_service(0);
        if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
            Serial.printf("GPIO ISR install failed: 0x%x\n", err);
            return false;
        }
        gpio_isr_installed = true;
    }
    
    // Reset camera power
    if (_config.pin_pwdn >= 0) {
        pinMode(_config.pin_pwdn, OUTPUT);
        digitalWrite(_config.pin_pwdn, HIGH);
        delay(100);
        digitalWrite(_config.pin_pwdn, LOW);
        delay(100);
    }
    
    // Adjust configuration if PSRAM is available
    if (psramFound()) {
        _config.jpeg_quality = 10;
        _config.fb_count = 2;
        _config.grab_mode = CAMERA_GRAB_LATEST;
    }

    esp_err_t err = esp_camera_init(&_config);
    if (err == ESP_OK) {
        _initialized = true;
        Serial.println("Camera initialized successfully");
        
        sensor_t *s = esp_camera_sensor_get();
        if (s->id.PID == OV3660_PID) {
            s->set_vflip(s, 1);
            s->set_brightness(s, 1);
            s->set_saturation(s, -2);
        }
        return true;
    } else {
        Serial.printf("Camera initialization failed with error 0x%x\n", err);
        return false;
    }
}

bool CameraManager::captureToBuffer(uint8_t** buf, size_t* len) {
    if (!_initialized) {
        Serial.println("Camera not initialized for capture");
        return false;
    }
    
    camera_fb_t* fb = esp_camera_fb_get();
    if (!fb) {
        Serial.println("Camera capture failed");
        return false;
    }
    
    *len = fb->len;
    *buf = (uint8_t*)malloc(fb->len);
    if (*buf == NULL) {
        Serial.println("Memory allocation failed for image buffer");
        esp_camera_fb_return(fb);
        return false;
    }
    
    memcpy(*buf, fb->buf, fb->len);
    esp_camera_fb_return(fb);
    
    Serial.printf("Captured image (%u bytes)\n", *len);
    return true;
}

bool CameraManager::isActive() const {
    return _initialized;
}