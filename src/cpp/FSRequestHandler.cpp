#include "FSRequestHandler.h"
#include "fnv.h"

FSRequestHandler::FSRequestHandler(FS& fs, const char* uri, const char* dir,
  bool shortNamesOnly, bool gzipOnly,  const char* indexFile)
  : m_fs(fs), m_uri(uri), m_dir(dir), m_shortNamesOnly(shortNamesOnly),
    m_gzipOnly(gzipOnly), m_indexFile(indexFile), m_debug(0)
{
  if (!m_uri.endsWith("/"))
    m_uri += "/";

  if (!m_dir.endsWith("/"))
    m_dir += "/";
}

FSRequestHandler::~FSRequestHandler()
{

}

bool FSRequestHandler::canHandle(HTTPMethod requestMethod, String requestUri)
{
  return requestUri.startsWith(m_uri) && requestMethod == HTTP_GET;
}

bool FSRequestHandler::handle(ESP8266WebServer& server, HTTPMethod requestMethod, String requestUri)
{
  if (m_debug) m_debug->printf("GET %s \n", requestUri.c_str());

  String fileName = requestUri.substring(m_uri.length());
  if (requestUri.endsWith("/"))
    fileName += m_indexFile;

  String contentType = getContentType(fileName);

  if (m_shortNamesOnly) {
    fileName = shortFileName(fileName);
  }

  if (!sendFile(server, fileName, contentType)) {
    if (m_shortNamesOnly)
      return false;

    // try short name
    fileName = shortFileName(fileName);
    if (!sendFile(server, fileName, contentType)) {
      return false;
    }
  }
}

bool FSRequestHandler::sendFile(ESP8266WebServer& server, String& fileName, String& contentType)
{
  fileName = m_dir + fileName;

  if (m_gzipOnly && !fileName.endsWith(".gz")) {
    fileName += ".gz";
  }

  if (m_debug) {
    m_debug->printf("Send File: [%s] exists=%s \n", fileName.c_str(),
      m_fs.exists(fileName) ? "true" : "false");
  }

  if (!m_fs.exists(fileName)) {
    if (m_gzipOnly)
      return false;

    // try .gz
    fileName += ".gz";

    if (m_debug) {
      m_debug->printf("Send File: [%s] exists=%s \n", fileName.c_str(),
        m_fs.exists(fileName) ? "true" : "false");
    }

    if (!m_fs.exists(fileName))
      return false;
  }

  // file exists
  File file = m_fs.open(fileName, "r");
  if (!file)
    return false;

  server.streamFile(file, contentType);
  return true;
}

/**
 * Generate a short filename using 32 bit FNV1a hash function.
 * The hexadecimal hash is returned (lowercase, 8 chars)
 */
String FSRequestHandler::shortFileName(String &fileName)
{
  uint32_t hash = FNV::fnv1a(fileName.c_str(), 0);
  char _shortName[9] {0};
  snprintf(_shortName, 9, "%08x", hash);
  String shortName = String(_shortName);
  if (m_debug)
    m_debug->printf("Hash for [%s] = [%s] \n", fileName.c_str(), shortName.c_str());
  return shortName;
}
