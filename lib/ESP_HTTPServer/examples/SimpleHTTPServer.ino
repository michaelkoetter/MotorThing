#include <HTTPServer.h>

HTTPServer server;

void setup() {
  server.begin();
}

void loop() {
  server.handle();
}
