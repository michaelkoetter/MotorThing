#include "HTTPRequestHandler.h"

void HTTPRequestHandler::sendResponse(HTTPResponse *response, unsigned int status, const uint8_t *content, size_t contentLength)
{
  response->setStatus(status);
  response->setContentLength(contentLength);
  response->write(content, contentLength);
}
