#ifndef _HTTP_CONNECTION_H
#define _HTTP_CONNECTION_H

#include <WiFiClient.h>

#include "config.h"
#include "HTTPRequest.h"

#include "http_parser.h"

#ifdef HTTPSERVER_DEBUG
#define HTTP_PARSER_DEBUG_CB(CB, DATA, ...) \
    HTTP_DEBUG("<%d> HTTPConnection::" #CB " data=%d \n", \
      reinterpret_cast<HTTPConnection *>(p->data)->id(), DATA) \
    return reinterpret_cast<HTTPConnection *>(p->data)->CB(__VA_ARGS__);
#else
#define HTTP_PARSER_DEBUG_CB(CB, DATA, ...) \
    return reinterpret_cast<HTTPConnection *>(p->data)->CB(__VA_ARGS__);
#endif

#define HTTP_PARSER_CB(CB, ...) \
  static int cb_##CB(http_parser* p) { HTTP_PARSER_DEBUG_CB(CB, -1) } \
  int CB();

#define HTTP_PARSER_CB_DATA(CB, ...) \
  static int cb_##CB(http_parser* p, const char *at, size_t length) \
    { HTTP_PARSER_DEBUG_CB(CB, length, at, length) } \
  int CB(const char * data, size_t length);


typedef enum { WaitRead = 1, Idle = 3, WaitClose = 98, Closed = 99} HTTPConnectionStatus;

class HTTPConnection
{
public:
    HTTPConnection();
    ~HTTPConnection();

    void              handle(WiFiClient& client, uint8_t* buf, size_t bufSize);
    void              init();

    unsigned long         id() { return m_id; }
    HTTPConnectionStatus  status() { return m_status; }
    bool                  idle() { return m_status == Idle || m_status == WaitClose; }
    void                  close();

private:
    // http_parser callback functions
    HTTP_PARSER_CB(onMessageBegin)
    HTTP_PARSER_CB(onHeadersComplete)
    HTTP_PARSER_CB(onMessageComplete)
    HTTP_PARSER_CB_DATA(onUrl)
    HTTP_PARSER_CB_DATA(onHeaderField)
    HTTP_PARSER_CB_DATA(onHeaderValue)
    HTTP_PARSER_CB_DATA(onBody)

    http_parser           m_parser;
    http_parser_settings  m_settings;

    WiFiClient            m_currentClient;
    HTTPRequest*          m_request;

    HTTPConnectionStatus  m_status;

    unsigned long         m_id;

    static unsigned long __id;
};

#endif //_HTTP_CONNECTION_H
