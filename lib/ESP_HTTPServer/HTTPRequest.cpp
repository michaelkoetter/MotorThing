#include "HTTPRequest.h"
#include "HTTPConnection.h"

unsigned long HTTPRequest::__id = 0;


HTTPRequest::HTTPRequest()
  : m_method((http_method) -1),
    m_path(0), m_query(0),
    m_currentHeader(-1), m_headerLength(0), m_headerValue(true),
    m_age(millis()), m_id(__id++)
{
  HTTP_DEBUG("   <%lu> HTTPRequest::HTTPRequest\n", m_id)

  http_parser_url_init(&m_urlP);

  // we store some headers by defult
  storeHeader("Host");
  storeHeader("Content-Type");
  storeHeader("Content-Length");

  // CORS support
  storeHeader("Access-Control-Request-Method");
  storeHeader("Access-Control-Request-Headers");
}

HTTPRequest::~HTTPRequest()
{
  HTTP_DEBUG("   <%lu> HTTPRequest::~HTTPRequest \n", m_id)
}

void HTTPRequest::storeHeader(const char * name, bool store) {
  int i = 0;
  while (i < HTTPSERVER_MAX_HEADER_LINES && m_header[i].field.buf()) i++;
  if (i < HTTPSERVER_MAX_HEADER_LINES) {
    m_header[i].field.set(name);
    m_header[i].store = true;
  }
}

int HTTPRequest::findHeader(const char * startsWith, size_t length)
{
  int i = 0;
  while (i < HTTPSERVER_MAX_HEADER_LINES && m_header[i].field) {
    if (strncasecmp(m_header[i].field.buf(), startsWith, length) == 0) return i;
    i++;
  }
  return -1;
}

int HTTPRequest::onUrl(http_method method, const char * data, size_t length)
{
  m_method = method;
  return m_url.onData(data, length);
}

int HTTPRequest::onHeaderField(const char * data, size_t length)
{
  m_headerField.onData(data, length);
  m_currentHeader = findHeader(m_headerField.buf(), m_headerField.size());
  m_headerValue = false;
  return 0;
}

int HTTPRequest::onHeaderValue(const char * data, size_t length)
{
  if (m_currentHeader >= 0 && m_header[m_currentHeader].store) {
    m_header[m_currentHeader].value.onData(data, length);
  }
  m_headerValue = true;
  m_headerField.clear();
  return 0;
}

int HTTPRequest::onHeadersComplete()
{
  HTTP_DEBUG("   <%lu> HTTPRequest::onHeadersComplete %s %s \n", m_id,
    http_method_str(m_method), m_url.buf())

  if (http_parser_parse_url(m_url.buf(), m_url.size(),
    m_method == HTTP_CONNECT ? 1 : 0, &m_urlP) != 0) {
    HTTP_DEBUG("   <%lu> HTTPRequest::onHeadersComplete could not parse URL: %s \n", m_url.buf())
    return 1; // this will abort the parser
  }

  if (m_urlP.field_set & (1 << UF_QUERY)) {
    // TODO instead of just storing the query, we should parse it as
    // application/x-www-form-urlencoded
    size_t _query_offset = m_urlP.field_data[UF_QUERY].off;
    m_query = m_url.buf(_query_offset);
    m_url.truncate(_query_offset + m_urlP.field_data[UF_QUERY].len);
  }

  if (m_urlP.field_set & (1 << UF_PATH)) {
    size_t _path_offset = m_urlP.field_data[UF_PATH].off;
    m_path = m_url.buf(_path_offset);
    m_url.truncate(_path_offset + m_urlP.field_data[UF_PATH].len);
  }

  #ifdef HTTPSERVER_DEBUG
  HTTP_DEBUG("   <%lu>    <path>: %s \n", m_id, m_path)
  HTTP_DEBUG("   <%lu>    <query>: %s \n", m_id, m_query)

  int i = 0;
  while (i < HTTPSERVER_MAX_HEADER_LINES && m_header[i].field) {
    HTTP_DEBUG("   <%lu>    %s: %s \n", m_id,
      m_header[i].field.buf(), m_header[i].value.buf());
    i++;
  }
  #endif

  return 0;
}

void HTTPRequest::log(Print& p)
{
  unsigned long ticks = millis();
  p.printf("@%lu %s %s %-lums \n", ticks, http_method_str(m_method),
    m_url.buf(), ticks - m_age);
}
