#include <Arduino.h>
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <FS.h>
#include <ArduinoJson.h>

#include "HTTPServer.h"
#include "TMCL.h"

#define RS485_RO 13
#define RS485_DI 14
#define RS485_DE 12
#define RS485_SPEED 9600

#define WIFI_SSID "motor-thing"
#define WIFI_PASSWORD "motor$123"

#define MDNS_HOST "motor-thing"
#define HTTP_PORT 80

#define TMCL_INIT_FILE "/tmcl.bin"

SoftwareSerial tmclSerial(RS485_RO, RS485_DI);
TMCLInterface tmclInterface(&tmclSerial);

HTTPServer http(HTTP_PORT);

void setup() {
  Serial.begin(115200);
  Serial.print("\n\nTMCL-WiFi Bridge\n\nSetup...\n");

  Serial.printf("RS-485 RO: %d DI: %d DE: %d speed: %d \n",
    RS485_RO, RS485_DI, RS485_DE, RS485_SPEED);

  tmclSerial.setTransmitEnablePin(RS485_DE);
  tmclSerial.begin(RS485_SPEED);
  tmclInterface.setDebug(&Serial);

  if (SPIFFS.begin()) {
    Serial.print("SPIFFS mounted. \n");

    if (SPIFFS.exists(TMCL_INIT_FILE)) {
      Serial.printf("Found '%s', starting TMCL download... \n", TMCL_INIT_FILE);
      File tmclBin = SPIFFS.open(TMCL_INIT_FILE, "r");
      if (tmclBin) {
        char tmclBuffer[TMCL_TELEGRAM_SIZE];
        TMCLTelegram telegram(tmclBuffer, TMCL_TELEGRAM_SIZE);
        TMCLDownload download(&tmclInterface, &telegram);
        download.download(tmclBin);
        Serial.printf("TMCL download %s \n", download.error() ? "ERROR" : "OK");
      } else {
        Serial.printf("Error opening TMCL file '%s'! \n", TMCL_INIT_FILE);
      }
    }
  } else {
    // this is not required (yet)
    Serial.print("SPIFFS could not be mounted. \n");
  }

  Serial.printf("WiFi AP ssid: %s password: %s \n",
    WIFI_SSID, WIFI_PASSWORD);
  WiFi.mode(WIFI_AP);
  WiFi.softAP(WIFI_SSID, WIFI_PASSWORD);

  if (MDNS.begin(MDNS_HOST, WiFi.softAPIP())) {
    Serial.printf("MDNS host: %s.local IP: %s \n", MDNS_HOST, WiFi.softAPIP().toString().c_str());
  }

  http.begin();
  Serial.printf("HTTP Server listening on port %d. \n", HTTP_PORT);

  MDNS.addService("http", "tcp", HTTP_PORT);

  Serial.print("Setup done.\n\n");
}

void loop() {
  http.handle();
}
