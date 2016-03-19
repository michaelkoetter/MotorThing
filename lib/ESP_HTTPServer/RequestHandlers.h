#ifndef _REQUEST_HANDLERS_H
#define _REQUEST_HANDLERS_H

#include "HTTPRequestHandler.h"
class NotFoundRequestHandler : public HTTPRequestHandler
{
public:
  virtual void handle(HTTPRequest* request, HTTPResponse* response)
  {
    sendResponse(response, 404, "Not found");
  }
};

#endif //_REQUEST_HANDLERS_H
