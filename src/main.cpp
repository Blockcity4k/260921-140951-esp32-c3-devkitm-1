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
bool yeswifi = true; // Flag to indicate if WiFi is enabled

void setup() {
    // Initialize serial communication
    Serial.begin(115200);
    while (!Serial) {
        delay(10);
    }
    delay(10000); // give time to open serial monitor in vscode
    // Initialize hardware
    sensor.begin();
    dataHandler.begin();
    
    // Connect to WiFi with better error handling
    Serial.println("Attempting to connect to WiFi...");
    Serial.print("SSID: ");
    Serial.println(WIFI_SSID);
    
    // Set WiFi mode to prevent issues
    WiFi.mode(WIFI_AP_STA);
    delay(10);
    
    // Begin connection
    WiFi.setTxPower(WIFI_POWER_8_5dBm); // VERY IMPORTANT DO NOT REMOVE: Set WiFi power to 8.5 dBm to prevent brownout issues
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    
    
    int timeout = 30;  // 30 seconds
    int count = 0;

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
        count++;
        
        // Show connection attempt every 10 seconds
        if (count % 10 == 0) {
            Serial.println("Still trying to connect...");
        }
        
        if (count > timeout) {
            Serial.println("\nFailed to connect to WiFi after " + String(timeout) + " seconds. Continuing without connection.");
            Serial.println("Make sure your WiFi credentials are correct.");
            break;
        }
    }
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nConnected to WiFi successfully!");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
        Serial.println("System ready!");
    } else {
        Serial.println("\nCould not connect to WiFi. Continuing without connection.");
        Serial.println("This is not necessarily an error - system will continue to operate.");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
        yeswifi = false;
    }
    
    // Start web server
    myWebServer = new MyWebServer(sensor, dataHandler);
    myWebServer->begin();
}

void loop() {
    unsigned long now = millis();
    
    // Check WiFi connection every 30 seconds
    if (now - lastWiFiCheck > 30000 && yeswifi == true) {
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
    
    // Add a small delay to prevent overloading the processor
    delay(100);
}
