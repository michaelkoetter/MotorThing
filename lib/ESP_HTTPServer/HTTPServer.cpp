#include "HTTPServer.h"

HTTPServer::HTTPServer(int port, bool tcpNoDelay)
  : m_server(port), m_tcpNoDelay(tcpNoDelay), m_connections(0),
    m_canHandle(true), m_canAccept(true)
{
}

HTTPServer::~HTTPServer()
{
  if (m_connections) delete[] m_connections;
}

void HTTPServer::begin()
{
  m_server.setNoDelay(m_tcpNoDelay);
  m_server.begin();
  m_connections = new ConnectionEntry[HTTPSERVER_MAX_CONNECTIONS];
}

void HTTPServer::handle()
{

  // Check memory thresholds
  size_t freeHeap = ESP.getFreeHeap();
  if (freeHeap < HTTPSERVER_ACCEPT_MEMORY_THRESHOLD) {
    if (m_canAccept) {
      Serial.printf("HTTPServer: low memory threshold reached, dropping new connections (%d bytes free) \n", freeHeap);
      m_canAccept = false;
    }
  } else {
    m_canAccept = true;
  }

  if (freeHeap < HTTPSERVER_HANDLE_MEMORY_THRESHOLD) {
    if (m_canHandle) {
      Serial.printf("HTTPServer: low memory threshold reached, stopping HTTP handling (%d bytes free) \n", freeHeap);
      m_canHandle = false;
    }
  } else {
    m_canHandle = true;
  }

  // FIXME the actual "accept" happened already somewhere in the WiFiClient
  //    so there is no way for us to not accept a client.
  if (m_canAccept && m_server.hasClient()) {

    // find an unused connection slot
    int slot = 0;
    while (slot < HTTPSERVER_MAX_CONNECTIONS && m_connections[slot].client) slot++;

    if (slot == HTTPSERVER_MAX_CONNECTIONS) {
      // give it a second chance, maybe we can kill an idle connection?
      slot = 0;
      while (slot < HTTPSERVER_MAX_CONNECTIONS && !m_connections[slot].httpConnection.idle()) slot++;
      if (slot < HTTPSERVER_MAX_CONNECTIONS) {
        Serial.printf("HTTPServer: dropping idle connection %d to serve new client. \n", slot);
      }
    }

    if (slot == HTTPSERVER_MAX_CONNECTIONS) {
      // no chance.
      Serial.printf("HTTPServer: too many active connections (%d), cannot serve client. \n",
        HTTPSERVER_MAX_CONNECTIONS);
    } else {
      // get new connection
      WiFiClient client = m_server.available();

      HTTP_DEBUG("<%d> HTTPServer::handle New connection \n", slot);
      client.setNoDelay(m_tcpNoDelay);
      ConnectionEntry& conn = m_connections[slot];

      // drop the client if this slot is being reused.
      if (conn.client) conn.client.stop();

      conn.client = client;
      conn.httpConnection.init();
    }
  }

  // handle connections
  for (int slot = 0; slot < HTTPSERVER_MAX_CONNECTIONS; slot++) {
    ConnectionEntry& conn = m_connections[slot];
    if (m_canHandle) {
      conn.httpConnection.handle(conn.client, m_buffer, HTTPSERVER_BUFFER);
    } else {
      // not much we can do, except wait for memory to become available again...
    }
  }

}
