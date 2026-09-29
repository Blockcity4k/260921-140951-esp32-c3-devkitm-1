#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include "config.h"
#include "sensor.h"
#include "data_handler.h"
#include "web_server.h"

// Global objects
Sensor sensor;
DataHandler dataHandler;
MyWebServer* myWebServer = nullptr; // Changed to pointer to avoid constructor issues

// Global variables
unsigned long lastUpdate = 0;
unsigned long lastWiFiCheck = 0;
bool yeswifi = false;

void setup() {
    Serial.begin(115200);

    // Initialize hardware
    sensor.begin();
    dataHandler.begin();

    int initialMoisture = sensor.getAverageMoisture();
    if (initialMoisture >= 0) {
        dataHandler.addReading(initialMoisture, millis());
        Serial.print("Moisture: ");
        Serial.println(initialMoisture);
    }
    lastUpdate = millis();
    
    // Connect to WiFi with better error handling
    Serial.println("Attempting to connect to WiFi...");
    Serial.print("SSID: ");
    Serial.println(WIFI_SSID);
    
    // Set WiFi mode to prevent issues
    WiFi.mode(WIFI_AP_STA);
    delay(10);
    
    WiFi.setTxPower(WIFI_POWER_8_5dBm); // VERY IMPORTANT DO NOT REMOVE: Set WiFi power to 8.5 dBm to prevent brownout issues
    WiFi.begin(WIFI_SSID, WIFI_PASS);

    myWebServer = new MyWebServer(sensor, dataHandler);
    myWebServer->begin();
    Serial.println("Sensor sampling started; Wi-Fi connection continues in the background.");
}

void loop() {
    unsigned long now = millis();
    
    // Check Wi-Fi without blocking sensor sampling.
    if (now - lastWiFiCheck >= WIFI_CHECK_INTERVAL) {
        lastWiFiCheck = now;
        if (WiFi.status() == WL_CONNECTED) {
            if (!yeswifi) {
                Serial.println("Connected to WiFi successfully!");
                Serial.print("IP Address: ");
                Serial.println(WiFi.localIP());
                yeswifi = true;
            }
        } else {
            if (yeswifi) {
                Serial.println("WiFi connection lost. Attempting to reconnect...");
            }
            yeswifi = false;
            WiFi.reconnect();
        }
    }
    
    // Main loop tasks
    if (now - lastUpdate >= UPDATE_INTERVAL) {
        // Read sensor data
        int moisture = sensor.getAverageMoisture();
        
        if (moisture >= 0) {
            dataHandler.addReading(moisture, millis());
            Serial.print("Moisture: ");
            Serial.println(moisture);
        }
        lastUpdate = millis();
    }
    
    // Add a small delay to prevent overloading the processor
    delay(100);
}
