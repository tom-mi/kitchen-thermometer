#include <Arduino.h>
#include <Adafruit_AMG88xx.h>
#include <ArduinoJson.h>

#include "wifi.h"
#include "tcp.h"

const unsigned long INTERVAL_MS = 100;
const unsigned long IDLE_TIMEOUT_MS = 300000;
const unsigned long BLINK_INTERVAL_MS = 200;
const float VOLTAGE_SCALE = (127. / 27.) / 1024.; // 27 + 100 kOhm voltage divider
const float MIN_VOLTAGE = 3.7;
const float MAX_VOLTAGE = 4.2;
const uint LED = 4;


Adafruit_AMG88xx amg;
float pixels[AMG88xx_PIXEL_ARRAY_SIZE];
unsigned long last_frame_ms;
unsigned long last_client_connected_ms;

void setup() {
  Serial.begin(9600);
  Serial.println(F("Hello :)\nKitchen Thermometer\nhttps://github.com/tom-mi/kitchen-thermometer"));

  pinMode(LED, OUTPUT);
  digitalWrite(LED, HIGH);

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

  last_client_connected_ms = millis();
}

void read_sensor_and_send_data() {
  amg.readPixels(pixels);

  DynamicJsonBuffer buffer;
  JsonObject& root = buffer.createObject();
  float voltage = analogRead(A0) / 1024. * (127. / 27.);
  double battery = (voltage - MIN_VOLTAGE) / (MAX_VOLTAGE - MIN_VOLTAGE);
  battery = min(max(battery, 0.), 1.);
  root["batteryVoltage"] = voltage;
  root["battery"] = battery;
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
}

void check_if_tired() {
  unsigned int time_since_last_client_connected_ms = millis() - last_client_connected_ms;
  if (time_since_last_client_connected_ms > IDLE_TIMEOUT_MS) {
    Serial.println(F("Idle timeout. Going to deep sleep."));
    ESP.deepSleep(0);
  } else {
    if ((millis() / BLINK_INTERVAL_MS) % 2 == 0) {
      digitalWrite(LED, HIGH);
    } else {
      digitalWrite(LED, LOW);
    }
  }
}

void loop() {
  last_frame_ms = millis();
  handle_connection_requests();
  if (number_of_clients() > 0) {
    digitalWrite(LED, HIGH);
    last_client_connected_ms = millis();
    read_sensor_and_send_data();
  } else {
    check_if_tired();
  }


  unsigned long desired_delay_ms = last_frame_ms + INTERVAL_MS - millis();
  if (0 < desired_delay_ms and desired_delay_ms <= INTERVAL_MS) {
    delay(desired_delay_ms);
  }
}
