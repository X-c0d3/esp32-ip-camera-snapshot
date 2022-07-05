/*

  # Author : Watchara Pongsri
  # [github/X-c0d3] https://github.com/X-c0d3/
  # Web Site: https://wwww.rockdevper.com

*/

#ifndef LINE_NOTIFY_H
#define LINE_NOTIFY_H

#include <Arduino.h>
#include <WiFiClientSecure.h>

#include "configs.h"
#define USER_AGENT "ESP32"
int httpCode = 404;
bool resStatus = false;
bool Line_Notify_Picture(String message, WiFiClient* stream, int len) {
    WiFiClientSecure _clientSecure;
    _clientSecure.setInsecure();
    if (!_clientSecure.connect("notify-api.line.me", 443)) {
        Serial.println("connection LINE failed");
        return false;
    }

    String boundary = "----LineNotify--";
    String body = "--" + boundary + "\r\n";
    body += "Content-Disposition: form-data; name=\"message\"\r\n\r\n" + message;
    body += " \r\n";

    body += "--" + boundary + "\r\n";
    body += "Content-Disposition: form-data; name=\"imageFile\"; filename=\"image.jpg\"\r\n";
    body += "Content-Type: image/jpeg\r\n\r\n";

    String body_end = "--" + boundary + "--\r\n";
    size_t body_length = body.length() + len + body_end.length();

    String header = "POST /api/notify HTTP/1.1\r\n";
    header += "Host: notify-api.line.me\r\n";
    header += "Authorization: Bearer " + String(LINE_TOKEN) + "\r\n";
    header += "User-Agent: " + String(USER_AGENT) + "\r\n";
    header += "Connection: close\r\n";
    header += "Cache-Control: no-cache\r\n";
    header += "Content-Length: " + String(body_length) + "\r\n";
    header += "Content-Type: multipart/form-data; boundary=" + boundary + "\r\n\r\n";
    _clientSecure.print(header + body);

    uint8_t buff[128] = {0};
    while ((len > 0 || len == -1)) {
        size_t size = stream->available();
        if (size) {
            // read up to 128 byte
            int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));
            _clientSecure.write(buff, c);
            if (len > 0) {
                len -= c;
            }
        }
        delay(1);
    }

    _clientSecure.print("\r\n" + body_end);
    while (_clientSecure.connected() && !_clientSecure.available()) delay(10);
    if (_clientSecure.connected() && _clientSecure.available()) {
        String resp = _clientSecure.readStringUntil('\n');
        httpCode = resp.substring(resp.indexOf(" ") + 1, resp.indexOf(" ", resp.indexOf(" ") + 1)).toInt();
        resStatus = (httpCode == 200);
        Serial.println(resp);
    }
    delay(10);

    _clientSecure.stop();

    return resStatus;
}

void Line_Notify(String message) {
    if (!ENABLE_LINE_NOTIFY)
        return;

    Serial.println("Send Line-Notify");
    WiFiClientSecure client;
    client.setInsecure();
    if (!client.connect("notify-api.line.me", 443)) {
        Serial.println("connection failed");
        return;
    }

    String req = "";
    req += "POST /api/notify HTTP/1.1\r\n";
    req += "Host: notify-api.line.me\r\n";
    req += "Authorization: Bearer " + String(LINE_TOKEN) + "\r\n";
    req += "Cache-Control: no-cache\r\n";
    req += "User-Agent: ESP32\r\n";
    req += "Content-Type: application/x-www-form-urlencoded\r\n";
    req += "Content-Length: " + String(String("message=" + message).length()) + "\r\n";
    req += "\r\n";
    req += "message=" + message;

    if (ENABLE_DEBUG_MODE)
        Serial.println(req);

    client.print(req);
    delay(20);

    Serial.println("-------------");
    while (client.connected()) {
        String line = client.readStringUntil('\n');
        if (line == "\r") {
            break;
        }
        if (ENABLE_DEBUG_MODE)
            Serial.println(line);
    }
    Serial.println("-------------");
}

#endif