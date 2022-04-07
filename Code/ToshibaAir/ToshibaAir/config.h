#pragma once


#define MASTER  0x00
#define REMOTE  0x40
#define BCAST   0xFE

//position of bytes in packet
#define FROM  0
#define TO    1
#define COUNT 3


#define MAX_RX_BUFFER 128//256 //maximum rx buffer size
#define MAX_CMD_BUFFER 32 //maximum message size

#define FAN_AUTO   2
#define FAN_HIGH   3
#define FAN_MEDIUM 4
#define FAN_LOW    5

#define MODE_COOL 2
#define MODE_FAN  3
#define MODE_AUTO 5
#define MODE_HEAT 1
#define MODE_DRY  4

#define TIMER_SW_OFF 0
#define TIMER_SW_ON  1
#define TIMER_SW_RESET  2

#define TIMER_HW_CANCEL     0x00
#define TIMER_HW_ON         0x07
#define TIMER_HW_REPEAT_OFF 0x06
#define TIMER_HW_OFF        0x05

//pg 18 http://www.toshiba-aircon.co.uk/assets/uploads/product_assets/20131115_IM_1115460101_Standard_Duct_RAV-SM_6BTP-E_EN.pdf
//indoor unit data
//#define INDOOR_ROOM   0x00 //Room Temp (Control Temp) (°C) 
//#define INDOOR_ROOM           0x01 //Room temperature (remote controller)
#define INDOOR_TA             0x02 //Indoor unit intake air temperature (TA)
#define INDOOR_TCJ            0x03 //Indoor unit heat exchanger (coil) temperature (TCJ) TCJ Coil Liquid Temp (°C)
#define INDOOR_TC             0x04 //Indoor unit heat exchanger (coil) temperature (TC)  Coil Vapour Temp (°C)
#define INDOOR_FAN_SPEED      0x07 //Fan Speed (rpm)
#define INDOOR_FAN_RUN_TIME   0xF2 //Fan Run Time (x 100h)
#define INDOOR_FILTER_TIME    0xF3 //Filter sign time x 1h
#define INDOOR_DISCHARGE_TEMP 0xF4 //Indoor discharge temperature*1  F8???
 
//outdoor unit data
#define OUTDOOR_TE              0x60 //Outdoor unit heat exchanger (coil) temperature (TE)
#define OUTDOOR_TO              0x61 //Outside air temperature (TO)
#define OUTDOOR_TD              0x62 //Compressor discharge temperature (TD)
#define OUTDOOR_TS              0x63 //Compressor suction temperature (TS)
#define OUTDOOR_THS             0x65 //Heatsink temperature (THS)
#define OUTDOOR_CURRENT         0x6A //Operating current (x1/10)
#define OUTDOOR_TL              0x6D //TL Liquid Temp (°C)
#define OUTDOOR_COMP_FREQ       0x70 //Compressor Frequency (rps)
#define OUTDOOR_LOWER_FAN_SPEED 0x72 //Fan Speed (Lower) (rpm
#define OUTDOOR_UPPER_FAN_SPEED 0x73 //Fan Speed (Upper) (rpm
#define OUTDOOR_HOURS           0xF1 //Compressor cumulative operating hours (x100 h)

//TA = Return Air Sensor; indoor unit
//TC = Coil Sensor; indoor unit
//TL = Liquid Pipe Sensor (fan speed); outdoor unit
//TCJ = Coil Sensor; indoor unit
//TE = Heat Exchange Sensor (defrost); outdoor unit
//TD = Discharge Pipe Sensor; outdoor unit
//TO = Ambient; outdoor unit
//TS = Suction; outdoor unitTK = Oil sensor (VRF)



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

static const uint8_t kTimerUnit = 1000;


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