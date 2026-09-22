#ifndef CONFIG_H
#define CONFIG_H

// Network Configuration
#define WIFI_SSID "SM-G965W9644"
#define WIFI_PASS "200703110"

// Hardware Configuration  
#define SENSOR_PIN 0
#define LED_BUILTIN 2

// Timing Configuration
#define UPDATE_INTERVAL 30000  // 30 seconds
#define WIFI_CHECK_INTERVAL 30000  // 30 seconds

// Sensor Configuration
#define DRY_THRESHOLD 55
#define MOISTURE_SAMPLES 20

// Web Server Configuration
#define WEB_SERVER_PORT 80

#endif
