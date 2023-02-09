#if !(defined(WIFI_STA_SSID) && defined(WIFI_STA_PASSWORD) && defined(WIFI_STA_PASS_LOGIN_URL) && defined(WIFI_AP_SSID) && defined(WIFI_AP_PASSWORD))
#error "Please specify your WIFI_STA_SSID, WIFI_STA_PASSWORD, WIFI_STA_PASS_LOGIN_URL, WIFI_AP_SSID, and WIFI_AP_PASSWORD"
#endif

#if defined(ESP8266)
// Additional board manager URL: https://arduino.esp8266.com/stable/package_esp8266com_index.json
#if !(defined(IP_NAPT_MAX) && defined(IP_PORTMAP_MAX))
#error "Please specify your IP_NAPT_MAX and IP_PORTMAP_MAX"
#endif

#undef HIGH
#define HIGH 0x0
#undef LOW
#define LOW 0x1

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <lwip/napt.h>
#include <lwip/dns.h>
#elif defined(ESP32)
// Additional board manager URL: https://github.com/tasmota/arduino-esp32/releases/latest/download/package_esp32_index.json
#if !defined(ESP_LWIP)
#error "No ESP_LWIP"
#endif
#if !defined(IP_FORWARD)
#error "No IP_FORWARD"
#endif
#if !defined(IP_NAPT)
#error "No IP_NAPT"
#endif

#undef HIGH
#define HIGH 0x1
#undef LOW
#define LOW 0x0

#define LED_BUILTIN 2

#include <WiFi.h>
#include <HTTPClient.h>
#include <lwip/lwip_napt.h>
#include <dhcpserver/dhcpserver.h>
#include <dhcpserver/dhcpserver_options.h>
#endif

#ifndef _UTILS_H
#define _UTILS_H


namespace Utils {
namespace {
std::function<bool(HTTPClient* httpClient)> isLoggedInCallback;

void blink() {
  digitalWrite(LED_BUILTIN, HIGH);  // Turn led on
  delay(250);
  digitalWrite(LED_BUILTIN, LOW);  // Turn led off
  delay(250);
}

#if defined(ESP8266)
void configureDNSAndNAPT() {
  DhcpServer& dhcpServer = WiFi.softAPDhcpServer();
  dhcpServer.setDns(WiFi.dnsIP(0));

  WiFi.softAPConfig(IPAddress(172, 217, 28, 254), IPAddress(172, 217, 28, 254), IPAddress(255, 255, 255, 0));
  WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASSWORD);

  err_t err = ip_napt_init(IP_NAPT_MAX, IP_PORTMAP_MAX);
  if (err == ERR_OK) {
    err = ip_napt_enable_no(SOFTAP_IF, 1);
  } else {
#ifdef DEBUG
    Serial.println("NAPT initialization failed.");
#endif
  }
}
#elif defined(ESP32)
void printErrIfExist(esp_err_t err) {
  if (err != ESP_OK) {
#ifdef DEBUG
    Serial.println(esp_err_to_name(err));
#endif
  }
}

void configureDNSAndNAPT() {
  esp_err_t err;
  tcpip_adapter_dns_info_t ip_dns;
  err = tcpip_adapter_dhcps_stop(TCPIP_ADAPTER_IF_AP);
  printErrIfExist(err);
  err = tcpip_adapter_get_dns_info(TCPIP_ADAPTER_IF_STA, ESP_NETIF_DNS_MAIN, &ip_dns);
  printErrIfExist(err);
  err = tcpip_adapter_set_dns_info(TCPIP_ADAPTER_IF_AP, ESP_NETIF_DNS_MAIN, &ip_dns);
  printErrIfExist(err);
  dhcps_offer_t opt_val = OFFER_DNS;
  err = tcpip_adapter_dhcps_option(ESP_NETIF_OP_SET, ESP_NETIF_DOMAIN_NAME_SERVER, &opt_val, 1);
  printErrIfExist(err);
  err = tcpip_adapter_dhcps_start(TCPIP_ADAPTER_IF_AP);
  printErrIfExist(err);
  ip_napt_enable(WiFi.softAPIP(), 1);
}
#endif
};

void checkWiFiConnection() {
  static bool isDisconnected = true;

  while (!WiFi.isConnected()) {
    isDisconnected = true;
#ifdef DEBUG
    Serial.println("Connecting to the WiFi network..");
#endif
    blink();
  }

  if (isDisconnected) {
    isDisconnected = false;
#ifdef DEBUG
    Serial.println("Connected to the WiFi network.");
#endif
  }
}

void loginToWiFiNetwork() {
  static WiFiClient wifiClient;
  static HTTPClient httpClient;
  static bool isLoggedIn = false;

  for (bool isBegin = false; !isBegin;) {
    isBegin = httpClient.begin(wifiClient, WIFI_STA_PASS_LOGIN_URL);
    if (!isBegin) {
#ifdef DEBUG
      Serial.println("Unable to connect to WiFi login site.");
#endif
    }
  }

  for (int httpResponseCode; !isLoggedIn;) {
    httpResponseCode = httpClient.GET();

    if (httpResponseCode != 200) {
      isLoggedIn = false;
#ifdef DEBUG
      Serial.print("Response Code: ");
      Serial.println(httpResponseCode);
      Serial.print("Error: ");
      Serial.println(httpClient.errorToString(httpResponseCode));
      Serial.println("Logging in to the WiFi network...");
#endif
      blink();
    } else if (!isLoggedIn) {
      if (isLoggedInCallback != NULL) {
        isLoggedIn = isLoggedInCallback(&httpClient);
      } else {
        isLoggedIn = true;
      }
      if (isLoggedIn) {
#ifdef DEBUG
        Serial.println("Logged in to the WiFi network...");
#endif
      }
    }
  }

  httpClient.end();
}

void runWiFiExtender() {
  WiFi.begin(WIFI_STA_SSID, WIFI_STA_PASSWORD);

  checkWiFiConnection();
  loginToWiFiNetwork();

  configureDNSAndNAPT();
}


#if defined(ESP8266)
void onWiFiSTADisconnected(const WiFiEventStationModeDisconnected& event) {
  runWiFiExtender();
}
#elif defined(ESP32)
void onWiFiSTADisconnected(arduino_event_id_t event) {
  runWiFiExtender();
}
#endif

void setIsLoggedInCallback(std::function<bool(HTTPClient* httpClient)> callback) {
  isLoggedInCallback = callback;
}
};

#endif