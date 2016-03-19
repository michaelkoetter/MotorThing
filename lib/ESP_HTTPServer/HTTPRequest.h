#ifndef _HTTP_REQUEST_H
#define _HTTP_REQUEST_H

#include <Print.h>

#include "config.h"
#include "http_parser.h"

class HTTPConnection;

class HTTPRequestValue
{
public:
  HTTPRequestValue() : _value(0), _size(0) {}
  ~HTTPRequestValue() { if (_value) free(_value); }

  inline int onData(const char * data, size_t length) {
    bool is_empty = _size == 0;
    _size += length;
    _value = (char*) realloc(_value, _size + 1);
    if (is_empty) {
      strncpy(_value, data, length);
    } else {
      strncat(_value, data, length);
    }
    _value[_size] = 0x00; // zero-terminate
    return 0;
  }

  void set(const char * str) {
    clear(); onData(str, strlen(str));
  }

  const char *  buf() { return _value; }
  const char *  buf(size_t offset) { return offset < _size ? _value + offset : 0; }

  void clear() { truncate(0); }

  void truncate(size_t at) {
    if (at < _size) {
      _size = at;
      if (_value) _value[at] = 0x00;
    }
  }

  size_t  size() { return _size; }
  operator bool() const { return _size > 0; }

private:
  char* _value;
  size_t _size;
};

struct HTTPHeader
{
  HTTPRequestValue field;
  HTTPRequestValue value;
  bool store = false;
};

class HTTPRequest
{

friend HTTPConnection;

public:
  void            storeHeader(const char * name, bool store = true);

  const char *    path() { return m_path; }
  http_method     method() { return m_method; }

private:
  HTTPRequest();
  ~HTTPRequest();

  int             onUrl(http_method method, const char * data, size_t length);
  int             onHeaderField(const char * data, size_t length);
  int             onHeaderValue(const char * data, size_t length);
  int             onHeadersComplete();
  int             findHeader(const char * startsWith, size_t length);

  void            log(Print& p);

  HTTPConnection* m_connection;
  http_method     m_method;

  HTTPRequestValue  m_url;
  http_parser_url   m_urlP;

  // pointer into m_url.buf()
  const char *      m_path;
  const char *      m_query;

  HTTPRequestValue  m_headerField;
  HTTPHeader      m_header[HTTPSERVER_MAX_HEADER_LINES];

  int             m_currentHeader;
  int             m_headerLength;
  bool            m_headerValue;

  unsigned long   m_age;
  unsigned long   m_id;

  static unsigned long __id;
};

#endif //_HTTP_REQUEST_H
