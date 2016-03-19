#ifndef _HTTP_REQUEST_HANDLER_H
#define _HTTP_REQUEST_HANDLER_H

#include "HTTPRequest.h"
#include "HTTPResponse.h"

class HTTPRequestHandler
{
public:
  /**
   * This is called whenever the server starts handling a new request.
   * The request will not be fully populated.
   *
   * The RequestHandler implementation can specify the headers it is interested
   * in via HTTPRequest#storeHeader.
   */
  virtual void        newRequest(HTTPRequest * request) {}

  /**
   * This is called whenever a valid request was received.
   */
  virtual void        handle(HTTPRequest * request, HTTPResponse * response) = 0;

  // Utility methods for simple responses
  void                sendResponse(HTTPResponse * response, unsigned int status, const uint8_t * content, size_t contentLength);
  void                sendResponse(HTTPResponse * response, const uint8_t * content, size_t contentLength)
                        { sendResponse(response, 200, content, contentLength); }
  void                sendResponse(HTTPResponse * response, unsigned int status, const char * contentStr)
                        { sendResponse(response, status, (const uint8_t*) contentStr, strlen(contentStr)); }
  void                sendResponse(HTTPResponse * response, const char * contentStr)
                        { sendResponse(response, 200, (const uint8_t*) contentStr, strlen(contentStr)); }
};

#endif //_HTTP_REQUEST_HANDLER_H
