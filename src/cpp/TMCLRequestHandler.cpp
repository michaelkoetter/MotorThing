#include "TMCLRequestHandler.h"

TMCLRequestHandler::TMCLRequestHandler(const char* uri, TMCLInterface* tmclInterface)
  : m_uri(uri), m_tmclInterface(tmclInterface)
{
  memset(m_tmclBuf, 0, TMCL_TELEGRAM_SIZE + 1);
  m_tmclTelegram = new TMCLTelegram(m_tmclBuf, TMCL_TELEGRAM_SIZE);
  m_tmclDownload = new TMCLDownload(m_tmclInterface, m_tmclTelegram);
}

TMCLRequestHandler::~TMCLRequestHandler()
{
  delete m_tmclDownload;
  delete m_tmclTelegram;
}

bool TMCLRequestHandler::canHandle(HTTPMethod requestMethod, String requestUri)
{
  return requestUri.equals(m_uri);
}

bool TMCLRequestHandler::canUpload(String requestUri)
{
  return canHandle(HTTP_PUT, requestUri);
}

bool TMCLRequestHandler::handle(ESP8266WebServer& server, HTTPMethod requestMethod, String requestUri)
{
  switch (requestMethod) {
    case HTTP_GET:
      handleGet(server);
      break;
    case HTTP_POST:
      handlePost(server);
      break;
    case HTTP_PUT:
      handlePut(server);
      break;
    case HTTP_OPTIONS:
      // Support CORS requests
      // make sure to call server.collectHeaders(--) to capture the Access-Control-Request-Headers header!
      server.sendHeader("Access-Control-Allow-Headers", server.header("Access-Control-Request-Headers"));
      server.sendHeader("Access-Control-Allow-Methods", "GET, POST, PUT, OPTIONS");
      server.send(200, "text/plain", "Go ahead...");
      break;
    default:
      server.send(405, "text/plain", "Method Not Allowed");
      break;
  }

  return true;
}

void TMCLRequestHandler::upload(ESP8266WebServer& server, String requestUri, HTTPUpload& upload)
{
  if (server.method() == HTTP_PUT) {
    handleUpload(server, upload);
  }
}

/**
 * Retrieve the TMCL module version.
 * Returns a JSON object like this:
 *
 * {
 *   "module_version": <version string>
 * }
 *
 * If there is a timeout, HTTP status 504 (Gateway Timeout) will be sent.
 */
void TMCLRequestHandler::handleGet(ESP8266WebServer& server)
{
  TMCLInstruction instruction(m_tmclTelegram);
  instruction.reset();
  instruction.instruction(136);

  int _address = 1;
  if (server.hasArg("address")) {
    _address = server.arg("address").toInt();
    if (_address <= 0 || _address > 255) {
      return sendStatus(server, 0, "'address' must be an integer between 1 and 255", 400);
    }
  }

  instruction.moduleAddress((char) _address);

  if (tmclSendAndReceive()) {
    StaticJsonBuffer<JSON_OBJECT_SIZE(2)> jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
    root["address"] = _address; // echo address
    root["version"] = m_tmclBuf + 1;
    sendJson(server, root);
  } else {
    sendStatus(server, _address, "TMCL reply timeout", 504);
  }
}

/**
 * Handles TMCL JSON instruction (POST):
 *
 * {
 *   "instruction": <instruction>,
 *   "type": <type>,
 *   "motor": <motor>,
 *   "value": <value>
 * }
 *
 * "type", "motor" and "value" are optional and default to 0.
 * Returns the TMCL reply as JSON:
 *
 * {
 *   "status": <status>,
 *   "instruction": <instruction>,
 *   "value": <value>
 * }
 *
 * If there is a timeout, HTTP status 504 (Gateway Timeout) will be sent.
 * If there is a checksum error, HTTP status 502 (Bad Gateway) will be sent.
 * If the request is malformed, HTTP status 400 is sent.
 */
void TMCLRequestHandler::handlePost(ESP8266WebServer& server)
{
  if (!server.hasArg("plain")) {
    return sendStatus(server, 0, "No POST data", 400);
  }

  String _request = server.arg("plain");

  // StaticJsonBuffer will make a copy of _request, thus
  // we need some extra memory (200 bytes should be plenty)
  StaticJsonBuffer<JSON_OBJECT_SIZE(4) + 200> requestBuffer;
  JsonObject& request = requestBuffer.parseObject(_request);

  if (!request.success()) {
    return sendStatus(server, 0, "JSON parse error", 400);
  }

  int _address = 1;
  if (request.containsKey("address")) {
    if (!request["address"].is<int>()) {
      return sendStatus(server, 0, "'address' must be an integer", 400);
    }

    _address = request["address"].as<int>();
    if (_address <= 0 || _address > 255) {
      return sendStatus(server, 0, "'address' must be an integer between 1 and 255", 400);
    }

  }

  if (!request.containsKey("instruction")) {
    return sendStatus(server, _address, "'instruction' is required", 400);
  }

  if (!request["instruction"].is<int>()) {
    return sendStatus(server, _address, "'instruction' must be an integer", 400);
  }

  TMCLInstruction instruction(m_tmclTelegram);
  instruction.reset();
  instruction.instruction(request["instruction"].as<char>());


  instruction.moduleAddress((char) _address);

  if (request.containsKey("type")) {
    if (!request["type"].is<int>()) {
      return sendStatus(server, _address, "'type' must be an integer", 400);
    }
    instruction.type(request["type"].as<char>());
  }

  if (request.containsKey("motor")) {
    if (!request["motor"].is<int>()) {
      return sendStatus(server, _address, "'motor' must be an integer", 400);
    }
    instruction.motor(request["motor"].as<char>());
  }

  if (request.containsKey("value")) {
    if (!request["value"].is<int>()) {
      return sendStatus(server, _address, "'value' must be an integer", 400);
    }
    instruction.value(request["value"].as<int>());
  }

  if (tmclSendAndReceive()) {
    if (m_tmclTelegram->checksumOK()) {
      TMCLReply reply(m_tmclTelegram);

      StaticJsonBuffer<JSON_OBJECT_SIZE(4)> replyBuffer;
      JsonObject& root = replyBuffer.createObject();
      root["address"] = reply.moduleAddress();
      root["status"] = reply.status();
      root["instruction"] = reply.instruction();
      root["value"] = reply.value();
      sendJson(server, root);
    } else {
      sendStatus(server, _address, "TMCL reply checksum error", 502);
    }
  } else {
    sendStatus(server, _address, "TMCL reply timeout", 504);
  }
}

void TMCLRequestHandler::handlePut(ESP8266WebServer& server)
{
  if (!m_tmclDownload->error()) {
    sendStatus(server, m_tmclDownload->address(), "TMCL download OK");
  } else {
    sendStatus(server, m_tmclDownload->address(), "TMCL download error", 500);
  }
}

/**
 * Uploads a compiled TMCL program (.bin file) to the module.
 */
void TMCLRequestHandler::handleUpload(ESP8266WebServer& server, HTTPUpload& upload)
{
  if (upload.status == UPLOAD_FILE_START) {
    int _address = 1;
    if (server.hasArg("address")) {
      _address = server.arg("address").toInt();
    }

    Serial.printf("Upload start: %s \n", upload.filename.c_str());
    m_tmclDownload->begin((char) _address);
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    Serial.printf("Upload write: %s (%d bytes) \n", upload.filename.c_str(), upload.currentSize);
    m_tmclDownload->download(upload.buf, upload.currentSize);
  } else if (upload.status == UPLOAD_FILE_END || upload.status == UPLOAD_FILE_ABORTED) {
    Serial.printf("Upload end: %s \n", upload.filename.c_str());
    m_tmclDownload->end();
    Serial.printf("TMCL download status: %s\n",
      m_tmclDownload->error() ? "ERROR" : "OK");
  }
  yield();
}

void TMCLRequestHandler::sendStatus(ESP8266WebServer& server, unsigned char address, const char * message, int status)
{
  StaticJsonBuffer<JSON_OBJECT_SIZE(2)> replyBuffer;
  JsonObject& root = replyBuffer.createObject();
  root["address"] = address;
  if (status >= 400) {
    root["error"] = message;
  } else {
    root["status"] = message;
  }

  sendJson(server, root, status);
}

void TMCLRequestHandler::sendJson(ESP8266WebServer& server, JsonObject& json, int status)
{
  String content;
  json.printTo(content);
  server.send(status, "application/json", content);
}
