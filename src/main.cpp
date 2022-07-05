#include <Arduino.h>
#include <ArduinoJson.h>
#include <EasyButton.h>
#include <FS.h>
#include <HardwareSerial.h>
#include <SocketIOclient.h>
#include <TridentTD_LineNotify.h>
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

int signalPin = 12;

void sendLineNotify() {
    String fileName = "/snapshot.jpg";
    digitalWrite(LED_BUILTIN, LOW);
    HTTPClient http;

    Serial.println("[HTTP] begin...\n");
    Serial.println(fileName);

    String url = "http://" + String(IPCAM_IP) + ":" + String(IPCAM_PORT) + "/snapshot.cgi?user=" + String(IPCAM_USERNAME) + "&pwd=" + String(IPCAM_PASSWORD);
    Serial.println(url);
    http.begin(url);

    int httpCode = http.GET();
    if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTP] GET... code: %d\n", httpCode);
        Serial.printf("[FILE] open file for writing %d\n", fileName.c_str());

        File file = SPIFFS.open(fileName, "w");

        // file found at server
        if (httpCode == HTTP_CODE_OK) {
            // get lenght of document (is -1 when Server sends no Content-Length header)
            int len = http.getSize();

            // create buffer for read
            uint8_t buff[128] = {0};

            // get tcp stream
            WiFiClient* stream = http.getStreamPtr();

            while (http.connected() && (len > 0 || len == -1)) {
                // get available data size
                size_t size = stream->available();
                if (size) {
                    // read up to 128 byte
                    int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));

                    // write it to Serial
                    // file.write(buff, c);
                    if (len > 0) {
                        len -= c;
                    }
                }
                delay(1);
            }

            // LINE.setToken(LINE_TOKEN);
            // LINE.notifyPicture("Camera snapshot", {http.getStream().connected()}, len);

            Serial.println("Send image 2");
            LINE.notifyPicture("Camera snapshot", SPIFFS, "/snapshot.jpg");
        }
    }
    http.end();

    digitalWrite(LED_BUILTIN, HIGH);
}

// List files in SPIFFS (For debugging)
// void listFiles(void) {
//     Serial.println();
//     Serial.println("SPIFFS files found:");

//     Dir dir = SPIFFS.openDir("/");  // Root directory
//     String line = "=====================================";

//     Serial.println(line);
//     Serial.println("  File name               Size");
//     Serial.println(line);

//     while (dir.next()) {
//         String fileName = dir.fileName();
//         Serial.print(fileName);
//         int spaces = 25 - fileName.length();  // Tabulate nicely
//         while (spaces--) Serial.print(" ");
//         File f = dir.openFile("r");
//         Serial.print(f.size());
//         Serial.println(" bytes");
//     }

//     Serial.println(line);
//     Serial.println();
//     delay(1000);
// }

void onPressed() {
    Serial.println("capture");
    sendLineNotify();
}

void setup() {
    Serial.begin(115200);
    pinMode(signalPin, INPUT);
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);

    // Connect WIFI
    setup_Wifi();

    btnReset.begin();
    btnReset.onPressed(onPressed);

    // Initialize File System
    if (SPIFFS.begin()) {
        Serial.println("SPIFFS Initialize....ok");
    } else {
        Serial.println("SPIFFS Initialization...failed");
    }

    // Format File System
    if (SPIFFS.format()) {
        Serial.println("File System Formated");
    } else {
        Serial.println("File System Formatting Error");
    }

    sendLineNotify();
}

void loop() {
    btnReset.read();
}