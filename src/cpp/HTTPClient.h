#pragma once

#include <ESP8266WiFi.h>
#include <StreamString.h>
#include "HTTPServer.h"

#include <http_parser.h>

#define HTTP_PARSER_CB(CB) \
  static int cb_##CB(http_parser* p) { return reinterpret_cast<HTTPClient *>(p->data)->debug_##CB(); } \
  int debug_##CB() { HTTP_DEBUG("[%d] HTTPClient::" #CB "\n", id); return CB(); } \
  int CB();

#define HTTP_PARSER_CB_DATA(CB) \
  static int cb_##CB(http_parser* p, const char *at, size_t length) \
  { char buf[length + 1]; memcpy(buf, at, length); buf[length] = 0x00; \
    return reinterpret_cast<HTTPClient *>(p->data)->debug_##CB(reinterpret_cast<const char *>(buf)); } \
  int debug_##CB(const char * data) { HTTP_DEBUG("[%d] HTTPClient::" #CB " [%s] \n", id, data); return CB(data); } \
  int CB(const char * data);


class HTTPClient
{
public:
    HTTPClient(WiFiClient client);
    ~HTTPClient();

    void              handle();
    static HTTPClient*  first() { return m_firstClient; }

private:
    // http_parser callback functions
    HTTP_PARSER_CB(onMessageBegin)
    HTTP_PARSER_CB(onHeadersComplete)
    HTTP_PARSER_CB(onMessageComplete)
    HTTP_PARSER_CB_DATA(onUrl)
    HTTP_PARSER_CB_DATA(onHeaderField)
    HTTP_PARSER_CB_DATA(onHeaderValue)
    HTTP_PARSER_CB_DATA(onBody)

    static void       add(HTTPClient *self);
    static void       remove(HTTPClient *self);

    WiFiClient        m_client;
    http_parser       m_parser;
    http_parser_settings  m_settings;
    size_t            m_received;

    static HTTPClient*    m_firstClient;
    HTTPClient*           m_nextClient;
    int                   id;
};
