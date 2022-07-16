#ifndef CONFIGS_H
#define CONFIGS_H

// Configuration
#define DEVICE_NAME "HomeSecurity"
#define WIFI_SSID "MY-WIFI"
#define WIFI_PASSWORD "123456789"
#define SOCKETIO_HOST "192.168.137.102"
#define SOCKETIO_PORT 4000
#define SOCKETIO_CHANNEL "ESP"

// Line config
#define LINE_TOKEN "XXXXXXXXXXXXXX"

// Setup IP Camera (Generic P2P IP Camera)
#define IPCAM_IP "192.168.137.108"
#define IPCAM_PORT 81
#define IPCAM_USERNAME "admin"
#define IPCAM_PASSWORD "1234567"

#define FIRMWARE_VERSION "1.0.5"
#define FIRMWARE_LASTUPDATE "2022-07-17"
#define FIRMWARE_SERVER "https://www.xxxxxx.com/firmware/firmware.json"

#define ENABLE_DEBUG_MODE true
#define ENABLE_SOCKETIO true
#define DEFAULT_BAUD_RATE 115200
#define DEFAULT_INTERVAL 2000
#define CHECK_FIRMWARE_INTERVAL 20000
#define ENABLE_FIRMWARE_AUTOUPDATE true
#define ENABLE_LINE_NOTIFY true

#define MAX_WORLD_COUNT 140
#define MIN_WORLD_COUNT 5

#define CURRENT_SENSOR_PIN A0

#endif