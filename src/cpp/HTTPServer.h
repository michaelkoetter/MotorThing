#pragma once

#include <ESP8266WiFi.h>

#define HTTPSERVER_BUFFER 2048

//#ifdef DEBUG
#define HTTP_DEBUG(fmt, ...) Serial.printf(fmt, ##__VA_ARGS__)
//#endif

class HTTPServer
{
public:
  HTTPServer(int port = 80);
  ~HTTPServer();

  void              begin();
  void              handle();
private:
  WiFiServer        m_server;
};
