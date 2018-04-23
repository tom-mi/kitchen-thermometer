#include <Arduino.h>
#include <Adafruit_AMG88xx.h>
#include <ArduinoJson.h>

#include "wifi.h"
#include "tcp.h"

ADC_MODE(ADC_VCC);

Adafruit_AMG88xx amg;
float pixels[AMG88xx_PIXEL_ARRAY_SIZE];

void setup() {
  Serial.begin(9600);
  Serial.println(F("Hello :)\nKitchen Thermometer\nhttps://github.com/tom-mi/kitchen-thermometer"));

  connect_wifi();
  setup_tcp_server();

  bool status;
  status = amg.begin();
  if (!status) {
    Serial.println(F("Could not find a valid AMG88xx sensor, check wiring!"));
    while (1);
  }

  Serial.println();

  delay(100); // let sensor boot up
}

void loop() {
  handle_connection_requests();

  amg.readPixels(pixels);

  DynamicJsonBuffer buffer;
  JsonObject& root = buffer.createObject();
  root["battery"] = ESP.getVcc() / 1024.;
  root["width"] = 8;
  root["height"] = 8;
  JsonArray& temperatures = root.createNestedArray("temperatures");
  temperatures.copyFrom(pixels);

  send_json_object_to_all_clients(root);
  Serial.print(".");

  delay(100);
}
