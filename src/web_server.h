#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h> 
/* 
    asynchronous network = 1+ connections, non-blocking, event-driven, no delay() needed
    ESPAsyncWebServer library is used to handle HTTP requests and responses
    It allows for efficient handling of multiple clients simultaneously
    https://circuitlabs.net/building-web-interfaces-for-esp32/
*/ 
#include "sensor.h"
#include "data_handler.h"

class MyWebServer {
public:
    MyWebServer(Sensor& sensor, DataHandler& data);
    void begin();
    void handleClient();
    
private:
    void handleRoot(AsyncWebServerRequest *request);
    void handleDataJson(AsyncWebServerRequest *request);
    void handleHistoryJson(AsyncWebServerRequest *request);
    void handleDataHtml(AsyncWebServerRequest *request);
    void handleNotFound(AsyncWebServerRequest *request);
    AsyncWebServer* server;
    Sensor& sensor;
    DataHandler& dataHandler;
};

#endif
