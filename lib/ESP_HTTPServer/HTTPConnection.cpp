#include "HTTPConnection.h"
#include "HTTPResponse.h"
#include "RequestHandlers.h"

unsigned long HTTPConnection::__id = 0;

HTTPConnection::HTTPConnection()
  : m_request(0), m_status(Closed),
    m_id(__id++)
{
  HTTP_DEBUG("<%lu> HTTPConnection::HTTPConnection() \n", m_id)

  m_settings.on_message_begin = &cb_onMessageBegin;
  m_settings.on_headers_complete = &cb_onHeadersComplete;
  m_settings.on_message_complete = &cb_onMessageComplete;
  m_settings.on_url = &cb_onUrl;
  m_settings.on_header_field = &cb_onHeaderField;
  m_settings.on_header_value = &cb_onHeaderValue;
  m_settings.on_body = &cb_onBody;

  init();
}

void HTTPConnection::init() {
  HTTP_DEBUG("<%lu> HTTPConnection::init status=Closed \n", m_id)
  http_parser_init(&m_parser, HTTP_REQUEST);
  m_parser.data = this; // used by callbacks
  m_status = Closed;
}

HTTPConnection::~HTTPConnection()
{
  HTTP_DEBUG("<%lu> HTTPConnection::~HTTPConnection() \n", m_id)
  if (m_request) delete m_request;
}

void HTTPConnection::handle(WiFiClient& client, uint8_t* buf, size_t bufSize)
{
  m_currentClient = client;

  if (!client || !client.connected()) {
    if (m_status != Closed) {
      HTTP_DEBUG("<%lu> HTTPConnection::handle status=Closed \n", m_id)
      http_parser_execute(&m_parser, &m_settings, reinterpret_cast<const char*>(buf), 0);
      m_status = Closed;
    }
    return;
  }

  // else: we have a valid & connected client

  if (m_status == WaitClose) {
    // we are waiting for the connection to close
    client.stop();
    client = WiFiClient();
    return;
  }

  if (m_status == Closed) {
    // this is a new connection
    HTTP_DEBUG("<%lu> HTTPConnection::handle status=Idle \n", m_id)
    m_status = Idle;
  }

  if (client.available()) {
    HTTP_DEBUG("<%lu> HTTPConnection::handle status=WaitRead \n", m_id)
    m_status = WaitRead;
    int received = 0;
    int parsed = 0;
    received = client.read(buf, bufSize);
    HTTP_DEBUG("<%lu> HTTPConnection::handle Received %d bytes... \n", m_id, received)
    parsed = http_parser_execute(&m_parser, &m_settings, reinterpret_cast<const char*>(buf), received);
    HTTP_DEBUG("<%lu> HTTPConnection::handle Parsed %d bytes... \n", m_id, parsed)
    if (parsed != received) {
      HTTP_DEBUG("<%lu> HTTPConnection::handle Parsed != Received \n", m_id)

      Serial.printf("<%lu> HTTP parser error: %s - %s \n", m_id,
        http_errno_name((http_errno) m_parser.http_errno),
        http_errno_description((http_errno) m_parser.http_errno));

      // FIXME implement better error handling
      // for now we just close the connection...
      close();
    }
  }

  m_currentClient = WiFiClient();
}

void HTTPConnection::close() {
  HTTP_DEBUG("<%lu> HTTPConnection::close status=WaitClose \n", m_id)
  m_status = WaitClose;
}

int HTTPConnection::onMessageBegin()
{
  HTTP_DEBUG("<%lu> HTTPConnection::onMessageBegin free heap=%d \n", m_id, ESP.getFreeHeap())

  if (m_request != 0) {
    Serial.printf("<%lu> Dangling HTTP request, aborting.", m_id);
    return -1; // panic!
  }

  m_request = new HTTPRequest();
  return 0;
}

int HTTPConnection::onHeadersComplete()
{
  return m_request->onHeadersComplete();
}

int HTTPConnection::onMessageComplete()
{
  HTTP_DEBUG("<%lu> HTTPConnection::onMessageComplete free heap=%d \n", m_id, ESP.getFreeHeap())

  HTTPResponse response(m_currentClient, m_parser.http_major, m_parser.http_minor);
  NotFoundRequestHandler notFound;
  notFound.handle(m_request, &response);

  m_request->log(Serial);

  bool needToClose = false;

  if (response.contentLength() < 0) {
    HTTP_DEBUG("<%lu> HTTPConnection::conMessageComplete RequestHandler didn't set Content-Length, "
      "need to close connection. \n", m_id);
    needToClose = true;
  }

  if (!http_should_keep_alive(&m_parser)) {
    HTTP_DEBUG("<%lu> HTTPConnection::onMessageComplete need to close connection \n", m_id)
    needToClose = true;
  }

  if (needToClose) {
    close();
  } else {
    HTTP_DEBUG("<%lu> HTTPConnection::onMessageComplete status=Idle \n", m_id)
    m_status = Idle;
  }

  delete m_request; m_request = 0;
  return 0;
}

int HTTPConnection::onUrl(const char* data, size_t length)
{
  return m_request->onUrl((http_method) m_parser.method, data, length);
}

int HTTPConnection::onHeaderField(const char* data, size_t length)
{
  return m_request->onHeaderField(data, length);
}

int HTTPConnection::onHeaderValue(const char* data, size_t length)
{
  return m_request->onHeaderValue(data, length);
}

int HTTPConnection::onBody(const char * data, size_t length)
{
  return 0;
}
