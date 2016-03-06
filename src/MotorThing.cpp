#include <Arduino.h>
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ArduinoJson.h>

#include "TMCL.h"
#include "TMCLRequestHandler.h"

#define RS485_RO 13
#define RS485_DI 14
#define RS485_DE 12
#define RS485_SPEED 9600

#define WIFI_SSID "motor-thing"
#define WIFI_PASSWORD "motor$123"

#define MDNS_HOST "motor-thing"
#define HTTP_PORT 80

SoftwareSerial tmclSerial(RS485_RO, RS485_DI);
TMCLInterface tmclInterface(&tmclSerial);

ESP8266WebServer http(HTTP_PORT);

TMCLRequestHandler tmclRequestHandler("/tmcl", &tmclInterface);

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

  http.addHandler(&tmclRequestHandler);
  http.begin();
  Serial.printf("HTTP Server listening on port %d. \n", HTTP_PORT);

  MDNS.addService("http", "tcp", HTTP_PORT);

  Serial.print("Setup done.\n\n");

  tmclInterface.setDebug(&Serial);
}

void loop() {
  http.handleClient();
}
