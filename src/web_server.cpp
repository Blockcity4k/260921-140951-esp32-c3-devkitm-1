#include "web_server.h"
#include <ArduinoJson.h>

WebServer::WebServer(Sensor& sensor, DataHandler& data)
    : sensor(sensor), dataHandler(data) {
    server = new AsyncWebServer(80);
}

void WebServer::begin() {
    server->on("/", [this](AsyncWebServerRequest *request){
        handleRoot(request);
    });
    
    server->on("/data.json", [this](AsyncWebServerRequest *request){
        handleDataJson(request);
    });
    
    server->onNotFound([this](AsyncWebServerRequest *request){
        handleNotFound(request);
    });
    
    server->begin();
}

void WebServer::handleClient() {
    // No need to call server->handleClient() as it's handled by the framework
    // The actual handling is done by the ESPAsyncWebServer library
}

void WebServer::handleRoot(AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "Plant Monitor System");
}

void WebServer::handleDataJson(AsyncWebServerRequest *request) {
    StaticJsonDocument<200> doc;
    doc["moisture"] = dataHandler.getLatestReading();
    doc["timestamp"] = dataHandler.getLatestTimestamp();
    
    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
}

void WebServer::handleNotFound(AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not found");
}
