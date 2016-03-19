#ifndef _HTTP_RESPONSE_H
#define _HTTP_RESPONSE_H

#include <WiFiClient.h>
#include <Print.h>

#include "HTTPConnection.h"

class HTTPResponse : public Print
{

friend HTTPConnection;

public:
  void            setStatus(unsigned int status) { m_status = status; }
  void            setContentLength(size_t contentLength) { m_contentLength = contentLength; }
  void            setHeader(const char * field, const char * value);

  // Print methods
  virtual size_t  write(uint8_t);
  virtual size_t  write(const uint8_t *buffer, size_t size);

  long            contentLength() { return m_contentLength; }
private:
  HTTPResponse(WiFiClient& client, unsigned short httpMajor = 1,
     unsigned short httpMinor = 1)
     : m_client(client), m_httpMajor(httpMajor), m_httpMinor(httpMinor),
       m_status(200), m_contentLength(-1), m_headersSent(false)
     {}

  void            ensureHeadersSent();
  const char *    statusText(unsigned int status);

  WiFiClient      m_client;
  unsigned short  m_httpMajor, m_httpMinor;
  unsigned int    m_status;
  long            m_contentLength ;
  bool            m_headersSent;
};

#endif //_HTTP_RESPONSE_H
