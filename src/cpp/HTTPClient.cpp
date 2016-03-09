#include "HTTPClient.h"

HTTPClient* HTTPClient::m_firstClient = 0;

HTTPClient::HTTPClient(WiFiClient client)
  : m_client(client), m_received(0), m_nextClient(0)
{
  http_parser_init(&m_parser, HTTP_REQUEST);
  m_parser.data = this; // used by callbacks

  m_settings.on_message_begin = &cb_onMessageBegin;
  m_settings.on_headers_complete = &cb_onHeadersComplete;
  m_settings.on_message_complete = &cb_onMessageComplete;
  m_settings.on_url = &cb_onUrl;
  m_settings.on_header_field = &cb_onHeaderField;
  m_settings.on_header_value = &cb_onHeaderValue;
  m_settings.on_body = &cb_onBody;

  HTTPClient::add(this);
}

HTTPClient::~HTTPClient()
{
  HTTPClient::remove(this);
}

void HTTPClient::handle()
{
  if (m_nextClient) {
    m_nextClient->handle();
  }

  uint8_t buf[HTTPSERVER_BUFFER];
  int received = 0;
  int parsed = 0;
  do {
    received = m_client.read(buf, HTTPSERVER_BUFFER);
    m_received += received;
    if (m_received > 0) {
      HTTP_DEBUG("[%d] Received %d bytes... \n", id, received);
      parsed = http_parser_execute(&m_parser, &m_settings, reinterpret_cast<const char*>(buf), received);
      HTTP_DEBUG("[%d] Parsed %d bytes... \n", id, parsed);
      if (parsed != received) {
        HTTP_DEBUG("[%d] Parsed != Received! \n", id);
      }

      if (received == 0) {
        // EOF
        delete this;
      }
    }
  } while(received > 0);
}

void HTTPClient::add(HTTPClient *self)
{
  HTTPClient* tmp = m_firstClient;

  if (tmp == 0) {
    self->id = 0;
  } else {
    self->id = tmp->id + 1;
  }

  m_firstClient = self;
  self->m_nextClient = tmp;
  HTTP_DEBUG("[%d] HTTPClient::add \n", self->id);
}

void HTTPClient::remove(HTTPClient *self)
{
  HTTP_DEBUG("[%d] HTTPClient::remove \n", self->id);
  if (m_firstClient == self) {
    m_firstClient = self->m_nextClient;
    self->m_nextClient = 0;
    return;
  }

  for (HTTPClient* prev = m_firstClient; prev->m_nextClient; prev = prev->m_nextClient ) {
    if (prev->m_nextClient == self) {
      prev->m_nextClient = self->m_nextClient;
      self->m_nextClient = 0;
      return;
    }
  }
}

int HTTPClient::onMessageBegin()
{

}

int HTTPClient::onHeadersComplete()
{

}

int HTTPClient::onMessageComplete()
{
  m_client.write("HTTP/1.1 200 OK\r\n"
                 "Content-Type: text/ascii\r\n"
                 "\r\n"
                 "OK");
}

int HTTPClient::onUrl(const char * data)
{

}

int HTTPClient::onHeaderField(const char * data)
{

}

int HTTPClient::onHeaderValue(const char * data)
{

}

int HTTPClient::onBody(const char * data)
{

}
