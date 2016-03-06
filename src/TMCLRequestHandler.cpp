#include "TMCLRequestHandler.h"

TMCLRequestHandler::TMCLRequestHandler(const char* uri, TMCLInterface* tmclInterface)
  : m_uri(uri), m_tmclInterface(tmclInterface)
{
  memset(m_tmclBuf, 0, TMCL_TELEGRAM_SIZE + 1);
  m_tmclTelegram = new TMCLTelegram(m_tmclBuf);
}

TMCLRequestHandler::~TMCLRequestHandler()
{
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

  if (tmclSendAndReceive()) {
    StaticJsonBuffer<JSON_OBJECT_SIZE(1)> jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
    root["version"] = m_tmclBuf + 1;
    sendJson(server, root);
  } else {
    server.send(504, "text/plain", "TMCL timeout");
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
    return server.send(400, "text/plain", "No POST data");
  }

  String _request = server.arg("plain");

  // StaticJsonBuffer will make a copy of _request, thus
  // we need some extra memory (200 bytes should be plenty)
  StaticJsonBuffer<JSON_OBJECT_SIZE(4) + 200> requestBuffer;
  JsonObject& request = requestBuffer.parseObject(_request);

  if (!request.success()) {
    return server.send(400, "text/plain", "JSON parse error");
  }

  if (!request.containsKey("instruction")) {
    return server.send(400, "text/plain", "'instruction' is required");
  }

  if (!request["instruction"].is<int>()) {
    return server.send(400, "text/plain", "'instruction' must be an integer");
  }

  TMCLInstruction instruction(m_tmclTelegram);
  instruction.reset();
  instruction.instruction(request["instruction"].as<char>());

  if (request.containsKey("type")) {
    if (!request["type"].is<int>()) {
      return server.send(400, "text/plain", "'type' must be an integer");
    }
    instruction.type(request["type"].as<char>());
  }

  if (request.containsKey("motor")) {
    if (!request["motor"].is<int>()) {
      return server.send(400, "text/plain", "'motor' must be an integer");
    }
    instruction.motor(request["motor"].as<char>());
  }

  if (request.containsKey("value")) {
    if (!request["value"].is<int>()) {
      return server.send(400, "text/plain", "'value' must be an integer");
    }
    instruction.value(request["value"].as<int>());
  }

  if (tmclSendAndReceive()) {
    if (m_tmclTelegram->checksumOK()) {
      TMCLReply reply(m_tmclTelegram);

      StaticJsonBuffer<JSON_OBJECT_SIZE(3)> replyBuffer;
      JsonObject& root = replyBuffer.createObject();
      root["status"] = reply.status();
      root["instruction"] = reply.instruction();
      root["value"] = reply.value();
      sendJson(server, root);
    } else {
      server.send(502, "text/plain", "TMCL reply checksum error");
    }
  } else {
    server.send(504, "text/plain", "TMCL reply timeout");
  }
}

void TMCLRequestHandler::handlePut(ESP8266WebServer& server)
{
  server.send(200, "text/plain", "OK");
}

void TMCLRequestHandler::handleUpload(ESP8266WebServer& server, HTTPUpload& upload)
{

}

void TMCLRequestHandler::sendJson(ESP8266WebServer& server, JsonObject& json)
{
  String content;
  json.printTo(content);
  server.send(200, "application/json", content);
}
