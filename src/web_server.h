#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include "sensor.h"
#include "data_handler.h"

class WebServer {
public:
    WebServer(Sensor& sensor, DataHandler& data);
    void begin();
    void handleClient();
    
private:
    AsyncWebServer* server;
    Sensor& sensor;
    DataHandler& dataHandler;
};

#endif
