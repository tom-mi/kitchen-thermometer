#include <ESP8266WiFi.h>
#include <ArduinoJson.h>

#include "tcp.h"

const int PORT = 5000;
WiFiServer tcpServer(PORT);

const int MAX_CLIENTS = 4;
WiFiClient clients[MAX_CLIENTS];


void setup_tcp_server() {
  tcpServer.begin();
}

void handle_connection_requests() {
  WiFiClient newClient = tcpServer.available();
  if (newClient) {
    Serial.print(F("New TCP connection from "));
    Serial.println(newClient.remoteIP().toString());

    for (int i = 0; i < MAX_CLIENTS; i++) {
      if (!clients[i].connected()) {
        clients[i] = newClient;
        return;
      }
    }
    Serial.println(F("Too many connections, closing connection."));
    newClient.stop();
  }
}

void send_json_object_to_all_clients(JsonObject& object) {
  for (int i = 0; i < MAX_CLIENTS; i++) {
    if (clients[i].connected()) {
      // Discard incoming data
      while(clients[i].available()) {
        clients[i].read();
      }
      object.printTo(clients[i]);
      clients[i].print('\n');
    }
  }
}
