#pragma once



#define MAX_LOG_DATA 72 //store up to 72 readings in a circular buffer

// #define USE_BMP //if BMP sensor installed
#define USE_DHT //in DHT sensor installed

//#define USE_ASYNC

//wifi credentials, no needed because now we use WiFiManager
const char *w_ssid = "YOURWIFI";
const char *w_passwd = "YOURPASSPWD";

//Over the air credentials
const char *OTAName = "air";           // A name and a password for the OTA service
const char *OTAPassword = "esp8266";

const char* mdnsName = "air"; // Domain name for the mDNS responder. Just connect to air.local

#ifndef LED_BUILTIN
  #define LED_BUILTIN 13 // ESP32 DOES NOT DEFINE LED_BUILTIN
#endif

const int LED = LED_BUILTIN;

const int DHTPin = D3;
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321

#define RESET_MODE_PIN D4  //button to enter into wifi configuration

#include <Arduino.h>
#include "helper.hpp"
#include "LittleFS.h"
#include "ntp_timer.hpp"
#include "wifi.hpp"
#include "ota.hpp"
#include "sensors.hpp"


#define SPIFFS LittleFS //dirty hack not to change names in the migration of SPIFFS to LittleFS
#ifdef USE_ASYNC //for ESP32
  #include <ESPAsyncTCP.h_>
  #include <ESPAsyncWebServer.h_>
#else //use links2004 websocketserver
  #include <ESP8266WebServer.h>
  #include <WebSocketsServer.h>
#endif


#include <ESP8266mDNS.h>
#include <FS.h>
#include <ArduinoJson.h>
#include <NTPClient.h> 
#include <WiFiUdp.h>
#include "toshiba_serial.hpp"
#include "MySimpleTimer.hpp"
#include "process_request.hpp"