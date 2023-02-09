#if !(defined(ESP32) || defined(ESP8266))
#error "This code can only be used on ESP32 and ESP8266 devices"
#endif

#if defined(ESP8266)
// Additional board manager URL: https://arduino.esp8266.com/stable/package_esp8266com_index.json
#include <ESP8266WiFi.h>
#elif defined(ESP32)
// Additional board manager URL: https://github.com/tasmota/arduino-esp32/releases/latest/download/package_esp32_index.json
#include <WiFi.h>
#endif

#include "env.h"
#include "utils.h"

void setup() {
  Serial.begin(9600);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);  // Turn led on

  WiFi.mode(WIFI_AP_STA);

#if defined(ESP8266)
  WiFi.onStationModeDisconnected(Utils::onWiFiSTADisconnected);
#elif defined(ESP32)
  WiFi.onEvent(Utils::onWiFiSTADisconnected, ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
#endif

  Utils::runWiFiExtender();

  Utils::setIsLoggedInCallback(isLoggedInCallback);
}

void loop() {
}

bool isLoggedInCallback(HTTPClient* httpClient) {
  // Modify this callback as needed
  String payload = httpClient->getString();
  return payload.indexOf("You are logged in") > 0;
}