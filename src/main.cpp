/*

  # Author : Watchara Pongsri
  # [github/X-c0d3] https://github.com/X-c0d3/
  # Web Site: https://wwww.rockdevper.com

*/

#include <Arduino.h>
#include <ArduinoJson.h>
#include <EasyButton.h>
#include <SocketIOclient.h>
#include <arduino-timer.h>
#include <string.h>

#include "configs.h"
#include "esp_system.h"
#include "firmware.h"
#include "utility.h"
#include "wifiMan.h"

EasyButton btnReset(0);
SocketIOclient webSocket;

unsigned long interval = 300;         // the time we need to wait
unsigned long previousMillis = 0;     // millis() returns an unsigned long.
auto timer = timer_create_default();  // create a timer with default settings
Timer<> default_timer;                // save as above
hw_timer_t* watchdogTimer = NULL;

int senseDoorbell = 0;
int debounce = 1000;
unsigned long currentMillis = 0;
unsigned long prevRing = 0;

void event(uint8_t* payload, size_t length) {
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, payload);
    if (error)
        return;

    String action = doc[1]["action"];
    unsigned long currentMillis = millis();  // grab current time
    if (action != "null") {
        String state = doc[1]["payload"]["state"];
        String messageInfo = doc[1]["payload"]["messageInfo"];

        if ((unsigned long)(currentMillis - previousMillis) >= interval) {
            if (ENABLE_DEBUG_MODE) {
                Serial.printf("=====>: %s\n", payload);
            }

            // if (action == "ENERGY_RESET")
            //     pzem04t.resetEnergy();

            // Do something in here

            previousMillis = currentMillis;
        }
    }
}

void takeSnapshot(String message) {
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

// For testing
void onPressed() {
    Serial.println("capture");
    takeSnapshot("☃ มีผู้มาเยือน ☃ \r\n" + printLocalTime());
}

void IRAM_ATTR interruptReboot() {
    // Prevent reboot when firmware upgrading
    if (firmwareUpgradeProgress == 0) {
        ets_printf("reboot (Watch Dog)\n");
        esp_restart();
    }
}

void setupWatchDog() {
    Serial.print("Setting timer in setup");
    watchdogTimer = timerBegin(0, 80, true);
    // timer 0 divisor 80
    timerAlarmWrite(watchdogTimer, 10000000, false);  // 10 sec set time in uS must be fed within this time or reboot
    timerAttachInterrupt(watchdogTimer, &interruptReboot, true);
    timerAlarmEnable(watchdogTimer);  // enable interrupt
}

void hexdump(const void* mem, uint32_t len, uint8_t cols = 16) {
    const uint8_t* src = (const uint8_t*)mem;
    Serial.printf("\n[HEXDUMP] Address: 0x%08X len: 0x%X (%d)", (ptrdiff_t)src, len, len);
    for (uint32_t i = 0; i < len; i++) {
        if (i % cols == 0) {
            Serial.printf("\n[0x%08X] 0x%08X: ", (ptrdiff_t)src, i);
        }
        Serial.printf("%02X ", *src);
        src++;
    }
    Serial.printf("\n");
}

void socketIOEvent(socketIOmessageType_t type, uint8_t* payload, size_t length) {
    switch (type) {
        case sIOtype_DISCONNECT:
            Serial.printf("[IOc] Disconnected!\n");
            break;
        case sIOtype_CONNECT:
            Serial.printf("[IOc] Connected to url: %s\n", payload);

            // join default namespace (no auto join in Socket.IO V3)
            webSocket.send(sIOtype_CONNECT, "/");
            break;
        case sIOtype_EVENT:
            // Serial.printf("[IOc] get event: %s\n", payload);
            event(payload, length);
            break;
        case sIOtype_ACK:
            Serial.printf("[IOc] get ack: %u\n", length);
            hexdump(payload, length);
            break;
        case sIOtype_ERROR:
            Serial.printf("[IOc] get error: %u\n", length);
            hexdump(payload, length);
            break;
        case sIOtype_BINARY_EVENT:
            Serial.printf("[IOc] get binary: %u\n", length);
            hexdump(payload, length);
            break;
        case sIOtype_BINARY_ACK:
            Serial.printf("[IOc] get binary ack: %u\n", length);
            hexdump(payload, length);
            break;
    }
}

void setup() {
    Serial.begin(115200);

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);

    pinMode(CURRENT_SENSOR_PIN, INPUT);
    // Connect WIFI
    setup_Wifi();
    setupTimeZone();

    btnReset.begin();
    btnReset.onPressed(onPressed);

    if (WiFi.status() == WL_CONNECTED) {
        setupWatchDog();

        webSocket.begin(SOCKETIO_HOST, String(SOCKETIO_PORT).toInt());
        webSocket.onEvent(socketIOEvent);
        // use HTTP Basic Authorization this is optional remove if not needed
        // webSocket.setAuthorization("user", "Password");

        // webSocket.setExtraHeaders("Authorization: Bearer TOKEN_HERE");

        // try ever 5000 again if connection has failed
        webSocket.setReconnectInterval(5000);

        if (ENABLE_FIRMWARE_AUTOUPDATE)
            timer.every(CHECK_FIRMWARE_INTERVAL, updateFirmware);

        Line_Notify(String(DEVICE_NAME) + " - Started...");
    }
}

void loop() {
    if (ENABLE_SOCKETIO && (WiFi.status() == WL_CONNECTED)) {
        timerWrite(watchdogTimer, 0);  // reset timer (feed watchdog)

        webSocket.loop();

        currentMillis = millis();
        if (currentMillis - prevRing >= debounce) {
            senseDoorbell = analogRead(CURRENT_SENSOR_PIN);
            // detecting doolbell from current sensor
            if (senseDoorbell > 50) {
                Serial.println("DingDong : Value is " + String(senseDoorbell));
                takeSnapshot("☃ มีผู้มาเยือน ☃ \r\n" + printLocalTime());
                prevRing = currentMillis;
            }
        }
    }

    btnReset.read();
}