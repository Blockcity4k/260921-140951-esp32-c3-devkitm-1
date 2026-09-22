#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h> 
#include <esp_wifi.h> // Required for direct hardware power scaling

// WIFI setup
const char* ssid = "SM-G965W9644";
const char* password = "200703110";

// ntfy setup
const char* ntfy_topic = "my_secret_plant_123"; 
bool notificationSent = false;
unsigned long lastCheckTime = 0;
const unsigned long checkInterval = 10000; 

// Sensor Configuration
const int SENSOR_PIN = 0; 
const int NUM_SAMPLES = 20;
bool samples = true;

// CUSTOM CALIBRATION: Under 55 is dry, over 55 is moist
const int DRY_THRESHOLD = 55;

// WiFi connection handling
unsigned long lastWiFiCheck = 0;
const unsigned long wifiCheckInterval = 30000; // Check WiFi every 30 seconds

void sendNtfyNotification(String message) {
  // Only attempt to send notification if we're connected to WiFi
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    
    // Using the standard URL string since your hotspot connection is fully stable
    String url = "http://ntfy.sh/" + String(ntfy_topic);

    http.begin(url);
    // Note: We don't need the custom "Host" header anymore when using the direct domain name
    http.addHeader("Content-Type", "text/plain");
    http.addHeader("X-Title", "Plant Monitor Alert");
    http.addHeader("X-Priority", "high"); 

    int httpResponseCode = http.POST(message);
    // 502: bad gateway
    // 200: success

    if (httpResponseCode > 0) {
      Serial.print("Notification sent successfully! Response code: ");
      Serial.println(httpResponseCode);
    } else {
      Serial.print("Error sending notification. HTTP code: ");
      Serial.println(httpResponseCode);
    }

    http.end(); 
  } else {
    Serial.println("WiFi disconnected. Unable to send alert.");
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    delay(10); 
  }
  
  // Set up internal ADC
  analogSetAttenuation(ADC_11db);
  analogReadResolution(12); 

  // Initialize Wi-Fi in Station Mode
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  // HARDWARE FIX: Force the Wi-Fi radio to a stable power tier (8.5 dBm)
  // This prevents the small voltage regulator from browning out during connection
  esp_wifi_set_max_tx_power(34); // 34 corresponds to 8.5 dBm in Espressif SDK steps
  
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    // Check for connection failure
    if (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      Serial.println("Failed to connect to WiFi");
      return;
    }
  }

  Serial.println("\nConnected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  // Check WiFi connection every 30 seconds
  unsigned long now = millis();
  if (now - lastWiFiCheck > wifiCheckInterval) {
    lastWiFiCheck = now;
    if (WiFi.status() != WL_CONNECTED) {
      // Try to reconnect
      Serial.println("WiFi connection lost. Attempting to reconnect...");
      WiFi.reconnect();
      // Wait a bit for reconnection
      delay(2000);
    }
  }

  int currentMoisture = 0;

  // 1. Gather data readings
  if (samples == true) {
    long sum = 0;
    int validReadings = 0;
    
    // Take multiple samples to ensure accuracy
    for (int i = 0; i < NUM_SAMPLES; i++) {
      int reading = analogRead(SENSOR_PIN);
      // Add basic validation to filter out bad readings
      if (reading > 0 && reading < 4095) {
        sum += reading;
        validReadings++;
      }
      delay(5); 
    }
    
    if (validReadings > 0) {
      currentMoisture = sum / validReadings;
    } else {
      // If all readings were invalid, use a default value
      currentMoisture = -1; // Error code for sensor reading failure
      Serial.println("Warning: All sensor readings invalid");
    }
    Serial.print("Soil Moisture Level (Averaged): ");
  } else {
    currentMoisture = analogRead(SENSOR_PIN);
    Serial.print("Soil Moisture Level (Raw Single): ");
  }
  Serial.println(currentMoisture);

  // 2. ntfy Alert Logic 
  if (millis() - lastCheckTime >= checkInterval) {
    lastCheckTime = millis(); 

    // Only send notification if we have a valid reading
    if (currentMoisture >= 0) {
      if (currentMoisture < DRY_THRESHOLD) {
        // Moisture is low - send alert if not already sent
        if (!notificationSent) { 
          sendNtfyNotification("🚨 Your plant is thirsty! Current level is under 55.");
          notificationSent = true; 
        }
      } 
      else if (currentMoisture >= DRY_THRESHOLD) { 
        // Moisture is normal - send confirmation if we were previously sending alerts
        if (notificationSent) {
          sendNtfyNotification("🌱 Thank you! The plant is moist and happy again.");
          notificationSent = false; // Reset for next dry period
        }
      }
    }
  }

  delay(30000); 
}