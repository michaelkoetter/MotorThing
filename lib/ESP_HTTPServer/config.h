#ifndef _HTTPSERVER_CONFIG_H
#define _HTTPSERVER_CONFIG_H

// Maximium header size allowed. See http_parser.h for details
// 2k should be sufficient.
#ifndef HTTP_MAX_HEADER_SIZE
#define HTTP_MAX_HEADER_SIZE (2 * 1024)
#endif

// The maximum number of concurrent clients.
// The absolute maximum with ESP8266 Arduino libs seems to be 5
#ifndef HTTPSERVER_MAX_CONNECTIONS
#define HTTPSERVER_MAX_CONNECTIONS 4
#endif


// HTTP handling low memory threshold.
// If there is less free heap space than this threshold, we won't handle
// any HTTP communication
#ifndef HTTPSERVER_HANDLE_MEMORY_THRESHOLD
#define HTTPSERVER_HANDLE_MEMORY_THRESHOLD (HTTPSERVER_MAX_CONNECTIONS * HTTP_MAX_HEADER_SIZE)
#endif

// TCP Accept low memory threshold.
// If there is less free heap space than this threshold, we won't accept new
// connections.
// FIXME this doesn't work well with the ESP8266/Arduino WiFiClient,
//    eventually we might need to use low-level API.
#ifndef HTTPSERVER_ACCEPT_MEMORY_THRESHOLD
#define HTTPSERVER_ACCEPT_MEMORY_THRESHOLD (2 * HTTPSERVER_HANDLE_MEMORY_THRESHOLD)
#endif

// The size of our read buffer.
#ifndef HTTPSERVER_BUFFER
#define HTTPSERVER_BUFFER 1460
#endif

#ifndef HTTPSERVER_MAX_URL_LENGTH
#define HTTPSERVER_MAX_URL_LENGTH 1024
#endif

// Maximum number of header lines
#ifndef HTTPSERVER_MAX_HEADER_LINES
#define HTTPSERVER_MAX_HEADER_LINES 100
#endif

#ifdef HTTPSERVER_DEBUG
#define HTTP_DEBUG(fmt, ...) Serial.printf(fmt, ##__VA_ARGS__);
#else
#define HTTP_DEBUG(fmt, ...) ;
#endif

#endif //_HTTPSERVER_CONFIG_H
