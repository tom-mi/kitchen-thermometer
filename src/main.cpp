#include <Arduino.h>
#include <Adafruit_AMG88xx.h>
#include <ArduinoJson.h>

#include "wifi.h"
#include "tcp.h"

ADC_MODE(ADC_VCC);

const unsigned long INTERVAL_MS = 100;

Adafruit_AMG88xx amg;
float pixels[AMG88xx_PIXEL_ARRAY_SIZE];
unsigned long last_frame_ms;

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
  amg.setMovingAverageMode(true);

  Serial.println();

  delay(100); // let sensor boot up
}

void loop() {
  last_frame_ms = millis();
  handle_connection_requests();

  amg.readPixels(pixels);

  DynamicJsonBuffer buffer;
  JsonObject& root = buffer.createObject();
  root["battery"] = ESP.getVcc() / 1024.;
  root["width"] = 8;
  root["height"] = 8;
  // The min/max values are taken from the data sheet, however values outside
  // this range have been observed.
  root["temperatureRangeMin"] = 0;
  root["temperatureRangeMax"] = 80;
  JsonArray& temperatures = root.createNestedArray("temperatures");
  temperatures.copyFrom(pixels);

  send_json_object_to_all_clients(root);
  Serial.print(".");

  unsigned long desired_delay_ms = last_frame_ms + INTERVAL_MS - millis();
  if (0 < desired_delay_ms and desired_delay_ms <= INTERVAL_MS) {
    delay(desired_delay_ms);
  }
}
