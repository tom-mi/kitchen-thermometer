#ifndef TCP_H
#define TCP_H
#include <Arduino.h>
#include <ArduinoJson.h> 

void setup_tcp_server();
void handle_connection_requests();
void send_json_object_to_all_clients(JsonObject&);

#endif
