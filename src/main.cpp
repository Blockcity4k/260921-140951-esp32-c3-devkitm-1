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
WebServer webServer;

// Global variables
unsigned long lastUpdate = 0;
unsigned long lastWiFiCheck = 0;

void setup() {
    // Initialize serial communication
    Serial.begin(115200);
    while (!Serial) {
        delay(10);
    }
    
    // Initialize hardware
    sensor.begin();
    dataHandler.begin();
    
    // Connect to WiFi
    Serial.print("Connecting to WiFi ");
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    
    // Wait for connection
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    
    Serial.println("\nConnected to WiFi!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.println("System ready!");
    
    // Start web server
    webServer.begin();
}

void loop() {
    unsigned long now = millis();
    
    // Check WiFi connection every 30 seconds
    if (now - lastWiFiCheck > WIFI_CHECK_INTERVAL) {
        lastWiFiCheck = now;
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("WiFi connection lost. Attempting to reconnect...");
            WiFi.reconnect();
            delay(2000);
        }
    }
    
    // Main loop tasks
    if (now - lastUpdate > UPDATE_INTERVAL) {
        lastUpdate = now;
        
        // Read sensor data
        int moisture = sensor.getAverageMoisture();
        
        // Store data
        dataHandler.addReading(moisture, millis());
        
        Serial.print("Moisture: ");
        Serial.println(moisture);
    }
    
    // Handle web server client requests
    webServer.handleClient();
    
    // Add a small delay to prevent overloading the processor
    delay(100);
}
