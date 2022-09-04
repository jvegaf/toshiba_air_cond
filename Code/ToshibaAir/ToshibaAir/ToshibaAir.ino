#include "helper.hpp"
#include "LittleFS.h"
#include "ntp_timer.hpp"
#include "tb_wifi.hpp"
#include "ota.hpp"
#include "sensors.hpp"

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
#include "config.h"

//#include <GDBStub.h>

air_status_t air_status;
MySimpleTimer timerAC;

#ifdef USE_ASYNC
AsyncWebServer server(80);
AsyncWebSocket webSocket("/ws");
#else
ESP8266WebServer server(80);    // create a web server on port 80
WebSocketsServer webSocket(81); // create a websocket server on port 81
#endif

File fsUploadFile; // a File variable to temporarily store the received file

MySimpleTimer timerTemperature;
int temp_interval = 1800; // in secs, 30 mins

float dht_h[MAX_LOG_DATA];
float dht_t[MAX_LOG_DATA];
float ac_sensor[MAX_LOG_DATA];
// float ac_target[MAX_LOG_DATA];
int ac_outdoor_te[MAX_LOG_DATA];
float bmp_t[MAX_LOG_DATA];
float bmp_p[MAX_LOG_DATA];
unsigned long timestamps[MAX_LOG_DATA];
int temp_idx = 0;
float dht_h_current, dht_t_current, bmp_t_current, bmp_p_current = 0;

MySimpleTimer timerStatus;
MySimpleTimer timerReadSerial;
MySimpleTimer timerSaveFile;

void configModeCallback(WiFiManager *myWiFiManager);
void handleNotFound();
bool handleFileRead(String path);
void handleFileUpload();
void onWsEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length);
int save_file(String name, String text);

void startLittleFS();
void startWebSocket();
void startMDNS();
void startServer();
void startReadSerial(); // Start timer for serial readings (1s)
void handleTimer();
void handleTemperature();
// void handleSaveFile();
void getTemperatureCurrent();
void handleStatus();
void handleReadSerial();
void startStatus(); // Start timer for status print (10s)
void startTemperature();

void setup() {

  Serial.begin(115200); // Start the Serial communication to send messages to
                        // the computer
  //  gdbstub_init();
  delay(10);
  Serial.println("\r\n");
  Serial.println("Air conditioning starts!");

  TBWifi::initManager(); // Use wifimanager to connect

  // is_reset_button();

  OTA::initialize(); // Start the OTA service

  startLittleFS(); // Start the LittleFS and list all contents

  startWebSocket(); // Start a WebSocket server

  startMDNS(); // Start the mDNS responder

  startServer(); // Start a HTTP server with a file read handler and an upload
                 // handler

  init_air_serial(
      &air_status); // Start air conditioning structure and software serial

  air_status.ip = WiFi.localIP().toString();

  startReadSerial(); // Start timer for serial readings (1s)

  startStatus(); // Start timer for status print (10s)

  NTPTimer::initialize(air_status.boot_time); // Get time from NTP server

  startTemperature(); // Start timer for temperature readings (120s)
}

void loop() {
  handleTimer(); // Set all handlers
  handleTemperature();
  handleStatus();
  handleReadSerial();

#ifndef USE_ASYNC
  webSocket.loop();      // constantly check for websocket events
  server.handleClient(); // run the server
#endif

  ArduinoOTA.handle(); // listen for OTA events
}

void startTemperature() {
  timerTemperature.setUnit(ONE_SECOND);
  timerTemperature.setInterval(temp_interval);
  timerTemperature.repeat();
  timerTemperature.start();

  Sensors::initialize();

  // clear arrays
  memset(dht_h, 0, MAX_LOG_DATA * sizeof(float));
  memset(dht_t, 0, MAX_LOG_DATA * sizeof(float));
  memset(ac_sensor, 0, MAX_LOG_DATA * sizeof(float));
  memset(ac_outdoor_te, 0, MAX_LOG_DATA * sizeof(float));
  memset(bmp_p, 0, MAX_LOG_DATA * sizeof(float));
  memset(bmp_t, 0, MAX_LOG_DATA * sizeof(float));
  memset(timestamps, 0, MAX_LOG_DATA * sizeof(unsigned long));

  if (Sensors::dht_status) {
    auto data = Sensors::read_dht();
    dht_t[0] = data.temperature;
    dht_h[0] = data.humidity;
  }

  if (Sensors::bmp_status) {
    auto data = Sensors::read_bmp();
    bmp_t[0] = data.temperature;
    bmp_p[0] = data.pressure;
  }

  ac_sensor[0] = air_status.sensor_temp;
  // ac_target[0] = air_status.target_temp * air_status.power;

  air_query_sensor(&air_status, OUTDOOR_TE);
  ac_outdoor_te[0] = air_status.outdoor_te;

  timestamps[0] = 0; // timeClient.getEpochTime();
}

void startStatus() {
  timerStatus.setUnit(ONE_SECOND); // 1000ms
  timerStatus.setInterval(120);    // update every XX s
  timerStatus.repeat();
  timerStatus.start();

  air_query_sensors(&air_status); // query sensors on start
}

void startReadSerial() {
  timerReadSerial.setUnit(ONE_SECOND);
  timerReadSerial.setInterval(1);
  timerReadSerial.repeat();
  timerReadSerial.start();
}

// save status every hour
// void startSaveFile() {
//   timerSaveFile.setUnit(kTimerUnit);
//   timerSaveFile.setInterval(60);
//   timerSaveFile.repeat();
//   timerSaveFile.start();
// }

// check power each minute
// void startCheckPowerConsumption() {
//   timerCheckPowerConsumption.setUnit(kTimerUnit);
//   timerCheckPowerConsumption.setInterval(60);
//   timerCheckPowerConsumption.repeat();
//   timerCheckPowerConsumption.start();
// }

// software air conditioning timer
void handleTimer() {
  if (timerAC.isTime()) {
    if (air_status.timer_mode_req == TIMER_SW_OFF) {
      // set power off
      Serial.println("TIMER - POWER OFF");
      air_set_power_off(&air_status);
    } else if (air_status.timer_mode_req == TIMER_SW_ON) {
      // set power on
      Serial.println("TIMER - POWER ON");
      air_set_power_on(&air_status);
    }
  }
}

bmp_data_t readBPMSensor() {
  if (!Sensors::bmp_status) {
    // set to -1 when not available
    return {-1, -1};
  }

  return Sensors::read_bmp();
}

dht_data_t readDHTSensor() {
  if (!Sensors::dht_status) {
    return {-1, -1};
  }

  return Sensors::read_dht();
}
// we will call this every sampling time
void handleTemperature() {
  if (timerTemperature.isTime()) {
    Serial.printf("[TEMP] Reading sensors\n");
    // Reading temperature or humidity takes about 250 milliseconds!

    auto bmp_d = readBPMSensor();
    bmp_t[temp_idx] = bmp_d.temperature;
    bmp_p[temp_idx] = bmp_d.pressure;
    Serial.printf("BMP %d temp %.1f press %.1f\n", temp_idx, bmp_t[temp_idx],
                  bmp_p[temp_idx]);

    auto dht_d = readDHTSensor();
    dht_t[temp_idx] = dht_d.temperature;
    dht_h[temp_idx] = dht_d.humidity;
    Serial.printf("DHT %d temp %.1f hum %.1f\n", temp_idx, dht_t[temp_idx],
                  dht_h[temp_idx]);
    // ac_target[temp_idx] = air_status.target_temp * air_status.power; //0 if
    // powered off
    ac_sensor[temp_idx] = air_status.sensor_temp;

    air_query_sensor(&air_status, OUTDOOR_TE);
    ac_outdoor_te[temp_idx] = air_status.outdoor_te;

    NTPTimer::timeClient.update();
    timestamps[temp_idx] = NTPTimer::timeClient.getEpochTime();

    temp_idx = (temp_idx + 1) % MAX_LOG_DATA;
  }
}

void handleSaveFile() {
  if (timerSaveFile.isTime()) {
    String txt = air_to_json(&air_status);
    save_file("/status.json", txt);
    Serial.print("Saving status.json");
  }
}

// get temp without logging it
void getTemperatureCurrent() {
  // Reading temperature or humidity takes about 250 milliseconds!
  auto dht_d = readDHTSensor();
  dht_t_current = dht_d.temperature;
  dht_h_current = dht_d.humidity;
  Serial.printf("[DHT] temp %.1f hum %.1f\n", dht_d.temperature,
                dht_d.humidity);

  auto bmp_d = readBPMSensor();
  bmp_t_current = bmp_d.temperature;
  bmp_p_current = bmp_d.pressure;
  Serial.printf("[BMP] temp %.1f press %.1f\n", bmp_d.temperature,
                bmp_d.pressure);
}

// get current status, temperature and sensors. not intended for logging
void handleStatus() {
  if (timerStatus.isTime()) {
    getTemperatureCurrent();
    air_query_sensors(&air_status); // query few sensors, if this takes too much
                                    // time it can crash wifi
    air_print_status(&air_status);
    // air_explore_all_sensors(&air_status); //it takes a lot of time, use it
    // just to discover sensors
    air_status.power_consumption +=
        air_status.outdoor_current * 10 /
        (3600 / temp_interval); // 30 readings per hour
  }
}

void handleReadSerial() {
  if (timerReadSerial.isTime()) {
    air_parse_serial(&air_status);
  }
}

void startLittleFS() { // Start the LittleFS and list all contents
  LittleFS.begin();    // Start the SPI Flash File System (LittleFS)
  Serial.println("LittleFS started. Contents:");
  {
    Dir dir = LittleFS.openDir("/");
    while (dir.next()) { // List the file system contents
      String fileName = dir.fileName();
      size_t fileSize = dir.fileSize();
      Serial.printf("\tFS File: %s, size: %s\r\n", fileName.c_str(),
                    formatBytes(fileSize).c_str());
    }
    Serial.printf("\n");
  }
}

void startWebSocket() { // Start a WebSocket server
#ifdef USE_ASYNC
  webSocket.onEvent(onWsAsyncEvent);
  server.addHandler(&webSocket);
#else // use links2004
  webSocket.begin();            // start the websocket server
  webSocket.onEvent(onWsEvent); // if there's an incomming websocket message, go
                                // to function 'webSocketEvent'
#endif
  Serial.println("WebSocket server started.");
}

void startMDNS() {      // Start the mDNS responder
  MDNS.begin(mdnsName); // start the multicast domain name server
  Serial.print("mDNS responder started: http://");
  Serial.print(mdnsName);
  Serial.println(".local");
}

void startServer() { // Start a HTTP server with a file read handler and an
                     // upload handler
#ifdef USE_ASYNC
  Serial.print("Using Async");
  server.on(
      "/edit.html", HTTP_POST,
      [](AsyncWebServerRequest
             *request) { // If a POST request is sent to the /edit.html address,
        server.send(200, "text/plain", "");
      },
      handleFileUpload); // go to 'handleFileUpload'

  server.onNotFound(handleNotFound); // if someone requests any other file or
                                     // page, go to function 'handleNotFound'
  // and check if the file exists

#else
  server.on(
      "/edit.html", HTTP_POST,
      []() { // If a POST request is sent to the /edit.html address,
        server.send(200, "text/plain", "");
      },
      handleFileUpload); // go to 'handleFileUpload'

  server.onNotFound(handleNotFound); // if someone requests any other file or
                                     // page, go to function 'handleNotFound'
                                     // and check if the file exists

#endif

  server.begin(); // start the HTTP server
  Serial.println("HTTP server started.");
}

/*__________________________________________________________SERVER_HANDLERS__________________________________________________________*/

void handleNotFound() { // if the requested file or page doesn't exist, return a
                        // 404 not found error
  if (!handleFileRead(server.uri())) { // check if the file exists in the flash
                                       // memory (LittleFS), if so, send it
    server.send(404, "text/plain", "404: File Not Found: " + server.uri());
  }
}

/** send the right file to the client (if it exists) */
bool handleFileRead(String path) {
  bool ret;

  Serial.println("handleFileRead: " + path);
  if (path.endsWith("/"))
    path += "index.html"; // If a folder is requested, send the index file
  String contentType = getContentType(path); // Get the MIME type
  String pathWithGz = path + ".gz";
  if (LittleFS.exists(pathWithGz) ||
      LittleFS.exists(path)) {       // If the file exists, either as a compressed
                                   // archive, or normal
    if (LittleFS.exists(pathWithGz)) // If there's a compressed version available
      path += ".gz";               // Use the compressed verion
    File file = LittleFS.open(path, "r"); // Open the file
    if (!file) {
      Serial.println("file open failed" + path);
      ret = false;
    } else {
      size_t sent =
          server.streamFile(file, contentType); // Send it to the client
      file.close();                             // Close the file again
      Serial.println(String("\tSent file: ") + path);
      Serial.println(String("\tSize: ") + sent);
      ret = true;
    }
  } else
    Serial.println(String("\tFile Not Found: ") +
                   path); // If the file doesn't exist, return false
  ret = false;

  return ret;
}

void handleFileUpload() { // upload a new file to the LittleFS
  HTTPUpload &upload = server.upload();
  String path;
  if (upload.status == UPLOAD_FILE_START) {
    path = upload.filename;
    if (!path.startsWith("/"))
      path = "/" + path;
      /** The file server always prefers a compressed 
       *  version of a file
       *  */
    if (!path.endsWith(".gz")) {
      String pathWithGz = path + ".gz"; // So if an uploaded file is not
                                        // compressed, the existing compressed
      if (LittleFS.exists(pathWithGz)) // version of that file must be deleted (if
                                     // it exists)
        LittleFS.remove(pathWithGz);
    }
    Serial.print("handleFileUpload Name: ");
    Serial.println(path);
    fsUploadFile =
        LittleFS.open(path, "w"); // Open the file for writing in LittleFS (create
                                // if it doesn't exist)
    path = String();
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (fsUploadFile)
      fsUploadFile.write(
          upload.buf,
          upload.currentSize); // Write the received bytes to the file
  } else if (upload.status == UPLOAD_FILE_END) {
    if (fsUploadFile) {     // If the file was successfully created
      fsUploadFile.close(); // Close the file again
      Serial.print("handleFileUpload Size: ");
      Serial.println(upload.totalSize);
      server.sendHeader(
          "Location",
          "/success.html"); // Redirect the client to the success page
      server.send(303);
    } else {
      server.send(500, "text/plain", "500: couldn't create file");
    }
  }
}

/*
    websocket callback
*/
void onWsEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
  switch (type) {
  case WStype_DISCONNECTED: // if the websocket is disconnected
    Serial.printf("[WS (%u)] Disconnected!\n", num);
    break;
  case WStype_CONNECTED: { // if a new websocket connection is established
    IPAddress ip = webSocket.remoteIP(num);
    Serial.printf("[WS (%u)] Connected from %d.%d.%d.%d url: %s\n", num, ip[0],
                  ip[1], ip[2], ip[3], payload);
  } break;
  case WStype_TEXT: // if new text data is received
    Serial.printf("[WS (%u)] Length %d Received: %s\n", num, length, payload);
    // pendingWSRequest
    processRequest(payload);
  default:
    break;
  }
}

int save_file(String name, String text) {
  File file = LittleFS.open(name, "w");

  if (!file) {
    Serial.println("Error opening file for writing");
    return (0);
  }

  int bytes = file.print(text);

  file.close();

  return (bytes);
}
