#include "HTTPServer.h"
#include "HTTPClient.h"

HTTPServer::HTTPServer(int port)
  : m_server(port)
{
  m_server.close();
}

HTTPServer::~HTTPServer()
{

}

void HTTPServer::begin()
{
  m_server.begin();
}

void HTTPServer::handle()
{
  // accept new clients
  WiFiClient _client = m_server.available();
  if (_client) {
    HTTP_DEBUG("New client \n");
    new HTTPClient(_client);
  }

  if (HTTPClient::first()) {
    HTTPClient::first()->handle();
  }
}
