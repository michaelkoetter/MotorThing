#pragma once

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>

#include "TMCL.h"

class TMCLRequestHandler : public RequestHandler
{
public:
  TMCLRequestHandler(const char* uri, TMCLInterface* tmclInterface);
  ~TMCLRequestHandler();

  bool            canHandle(HTTPMethod requestMethod, String requestUri);
  bool            canUpload(String requestUri);
  bool            handle(ESP8266WebServer& server, HTTPMethod requestMethod, String requestUri);
  void            upload(ESP8266WebServer& server, String requestUri, HTTPUpload& upload);

private:

  void            handleGet(ESP8266WebServer& server);
  void            handlePost(ESP8266WebServer& server);
  void            handlePut(ESP8266WebServer& server);
  void            handleUpload(ESP8266WebServer& server, HTTPUpload& upload);

  void            sendJson(ESP8266WebServer& server, JsonObject& json);

  bool            tmclSendAndReceive() { return m_tmclInterface->sendAndReceive(m_tmclTelegram); }

  String          m_uri;
  TMCLInterface   *m_tmclInterface;

  // buffer has one extra byte to terminate version string
  char            m_tmclBuf[TMCL_TELEGRAM_SIZE + 1];
  TMCLTelegram    *m_tmclTelegram;
  TMCLDownload    *m_tmclDownload;
};
