#pragma once

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FS.h>

/**
 * A RequestHandler that serves file from the embedded filesystem - with a twist.
 *
 * This handler can derive short file names using a 32 bit FNV1a hash to
 * overcome embedded filesystem filename length restrictions.
 * In addition, it can serve a gzipped copy of the requested file.
 *
 * Using the 'shortNamesOnly' and 'gzipOnly' flags, the handler can be
 * configured to not even attempt to serve "normal" files.
 *
 * To build the short filenames, something like Gulp in conjunction with
 * the 'fnv-plus' library could be used (https://github.com/tjwebb/fnv-plus),
 * but ultimately it's up to you.
 */
class FSRequestHandler : public RequestHandler
{
public:
  FSRequestHandler(FS& fs, const char* uri, const char* dir,
    bool shortNamesOnly = false, bool gzipOnly = false,
    const char* indexFile = "index.html");

  ~FSRequestHandler();

  bool            canHandle(HTTPMethod requestMethod, String requestUri);
  bool            handle(ESP8266WebServer& server, HTTPMethod requestMethod, String requestUri);
  void            setDebug(Stream* debug) { m_debug = debug; }

private:
  String          shortFileName(String& fileName);
  bool            sendFile(ESP8266WebServer& server, String& fileName, String& contentType);

  String          m_uri, m_dir, m_indexFile;
  FS&             m_fs;
  Stream*         m_debug;
  bool            m_shortNamesOnly, m_gzipOnly;

  static String getContentType(const String& path) {
      if (path.endsWith(".html")) return "text/html";
      else if (path.endsWith(".htm")) return "text/html";
      else if (path.endsWith(".tag")) return "text/html";
      else if (path.endsWith(".css")) return "text/css";
      else if (path.endsWith(".txt")) return "text/plain";
      else if (path.endsWith(".js")) return "application/javascript";
      else if (path.endsWith(".png")) return "image/png";
      else if (path.endsWith(".gif")) return "image/gif";
      else if (path.endsWith(".jpg")) return "image/jpeg";
      else if (path.endsWith(".ico")) return "image/x-icon";
      else if (path.endsWith(".svg")) return "image/svg+xml";
      else if (path.endsWith(".ttf")) return "application/x-font-ttf";
      else if (path.endsWith(".otf")) return "application/x-font-opentype";
      else if (path.endsWith(".woff")) return "application/font-woff";
      else if (path.endsWith(".woff2")) return "application/font-woff2";
      else if (path.endsWith(".eot")) return "application/vnd.ms-fontobject";
      else if (path.endsWith(".sfnt")) return "application/font-sfnt";
      else if (path.endsWith(".xml")) return "text/xml";
      else if (path.endsWith(".pdf")) return "application/pdf";
      else if (path.endsWith(".zip")) return "application/zip";
      else if(path.endsWith(".gz")) return "application/x-gzip";
      else if (path.endsWith(".appcache")) return "text/cache-manifest";
      return "application/octet-stream";
  }
};
