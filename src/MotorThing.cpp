#include <Arduino.h>
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ArduinoJson.h>

#include "TMCL.h"

#define RS485_RO 13
#define RS485_DI 14
#define RS485_DE 12
#define RS485_SPEED 9600

#define WIFI_SSID "motor-thing"
#define WIFI_PASSWORD "motor$123"

#define MDNS_HOST "motor-thing"
#define HTTP_PORT 80

SoftwareSerial tmclSerial(RS485_RO, RS485_DI);
TMCLInterface tmcl(&tmclSerial);

ESP8266WebServer http(HTTP_PORT);

// one extra byte to terminate version string
char tmclBuffer[TMCL_TELEGRAM_SIZE + 1] {0};
TMCLTelegram telegram(tmclBuffer);

TMCLInstruction instruction(&telegram);
TMCLReply reply(&telegram);

void sendJson(JsonObject& json) {
  String content;
  json.printTo(content);
  http.send(200, "application/json", content);
}

/**
 * Retrive the TMCL module version.
 * Returns a JSON object like this:
 *
 * {
 *   "module_version": <version string>
 * }
 *
 * If there is a timeout, HTTP status 504 (Gateway Timeout) will be sent.
 */
void handleVersion() {
  instruction.reset();
  instruction.instruction(136);
  if (tmcl.sendAndReceive(telegram)) {
    StaticJsonBuffer<JSON_OBJECT_SIZE(1)> jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
    root["version"] = tmclBuffer + 1;
    sendJson(root);
  } else {
    http.send(504, "text/plain", "TMCL timeout");
  }
}

/**
 * Handles TMCL JSON instruction (POST):
 *
 * {
 *   "instruction": <instruction>,
 *   "type": <type>,
 *   "motor": <motor>,
 *   "value": <value>
 * }
 *
 * "type", "motor" and "value" are optional and default to 0.
 * Returns the TMCL reply as JSON:
 *
 * {
 *   "status": <status>,
 *   "instruction": <instruction>,
 *   "value": <value>
 * }
 *
 * If there is a timeout, HTTP status 504 (Gateway Timeout) will be sent.
 * If there is a checksum error, HTTP status 502 (Bad Gateway) will be sent.
 * If the request is malformed, HTTP status 400 is sent.
 */
void handleInstruction() {
  if (http.method() != HTTP_POST) {
    return http.send(405, "text/plain", "Use POST method to send TMCL instructions");
  }

  if (!http.hasArg("plain")) {
    return http.send(400, "text/plain", "No POST data");
  }

  String _request = http.arg("plain");

  // StaticJsonBuffer will make a copy of _request, thus
  // we need some extra memory (200 bytes should be plenty)
  StaticJsonBuffer<JSON_OBJECT_SIZE(4) + 200> requestBuffer;
  JsonObject& request = requestBuffer.parseObject(_request);

  if (!request.success()) {
    return http.send(400, "text/plain", "JSON parse error");
  }

  if (!request.containsKey("instruction")) {
    return http.send(400, "text/plain", "'instruction' is required");
  }

  if (!request["instruction"].is<int>()) {
    return http.send(400, "text/plain", "'instruction' must be an integer");
  }

  instruction.reset();
  instruction.instruction(request["instruction"].as<char>());

  if (request.containsKey("type")) {
    if (!request["type"].is<int>()) {
      return http.send(400, "text/plain", "'type' must be an integer");
    }
    instruction.type(request["type"].as<char>());
  }

  if (request.containsKey("motor")) {
    if (!request["motor"].is<int>()) {
      return http.send(400, "text/plain", "'motor' must be an integer");
    }
    instruction.motor(request["motor"].as<char>());
  }

  if (request.containsKey("value")) {
    if (!request["value"].is<int>()) {
      return http.send(400, "text/plain", "'value' must be an integer");
    }
    instruction.value(request["value"].as<int>());
  }

  if (tmcl.sendAndReceive(telegram)) {
    if (telegram.checksumOK()) {
      StaticJsonBuffer<JSON_OBJECT_SIZE(3)> replyBuffer;
      JsonObject& root = replyBuffer.createObject();
      root["status"] = reply.status();
      root["instruction"] = reply.instruction();
      root["value"] = reply.value();
      sendJson(root);
    } else {
      http.send(502, "text/plain", "TMCL reply checksum error");
    }
  } else {
    http.send(504, "text/plain", "TMCL reply timeout");
  }
}

void setup() {
  Serial.begin(115200);
  Serial.print("\n\nTMCL-WiFi Bridge\n\nSetup...\n");

  Serial.printf("RS-485 RO: %d DI: %d DE: %d speed: %d \n",
    RS485_RO, RS485_DI, RS485_DE, RS485_SPEED);

  tmclSerial.setTransmitEnablePin(RS485_DE);
  tmclSerial.begin(RS485_SPEED);

  Serial.printf("WiFi AP ssid: %s password: %s \n",
    WIFI_SSID, WIFI_PASSWORD);

  WiFi.mode(WIFI_AP);
  WiFi.softAP(WIFI_SSID, WIFI_PASSWORD);

  if (MDNS.begin(MDNS_HOST, WiFi.softAPIP())) {
    Serial.printf("MDNS host: %s.local IP: %s \n", MDNS_HOST, WiFi.softAPIP().toString().c_str());
  }

  http.on("/tmcl", HTTP_GET, handleVersion);
  http.on("/tmcl", HTTP_POST, handleInstruction);
  http.begin();
  Serial.printf("HTTP Server listening on port %d. \n", HTTP_PORT);

  MDNS.addService("http", "tcp", HTTP_PORT);

  Serial.print("Setup done.\n\n");

  tmcl.setDebug(&Serial);
}

void loop() {
  http.handleClient();
}
