#include <Arduino.h>
#include <ArduinoJson.h>
#include <EasyButton.h>
#include <HardwareSerial.h>
#include <SocketIOclient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <arduino-timer.h>
#include <string.h>

#include "configs.h"
#include "esp_system.h"
#include "firmware.h"
#include "utility.h"
#include "wifiMan.h"

EasyButton btnReset(0);

void sendLineNotify(String message) {
    digitalWrite(LED_BUILTIN, HIGH);
    HTTPClient http;
    http.begin("http://" + String(IPCAM_IP) + ":" + String(IPCAM_PORT) + "/snapshot.cgi?user=" + String(IPCAM_USERNAME) + "&pwd=" + String(IPCAM_PASSWORD));

    int httpCode = http.GET();
    if (httpCode > 0) {
        Serial.printf("[HTTP] GET... code: %d\n", httpCode);
        // file found at server
        if (httpCode == HTTP_CODE_OK) {
            int len = http.getSize();
            WiFiClient* stream = http.getStreamPtr();
            Serial.println("Sending Snapshot from Ip Camera");
            if (Line_Notify_Picture(message, stream, len)) {
                Serial.println("The Snapshot sending success !!");
            }
        }
    }
    http.end();

    digitalWrite(LED_BUILTIN, LOW);
}

void onPressed() {
    Serial.println("capture");
    sendLineNotify("Testing take snapshot from Ipcam");
}

void setup() {
    Serial.begin(115200);

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);

    // Connect WIFI
    setup_Wifi();

    btnReset.begin();
    btnReset.onPressed(onPressed);
}

void loop() {
    btnReset.read();
}