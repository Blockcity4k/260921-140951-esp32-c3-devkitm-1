#ifndef CONFIG_H
#define CONFIG_H

// Network Configuration
#define WIFI_SSID "wifi2"
#define WIFI_PASS "j}Y29122"
/*
 * Add your own WiFi credentials here
 * You can also add multiple networks for fallback using the commented lines below
 * For example, you can add more SSIDs with different passwords for different locations
 * and let the ESP try them all until one works
*/

// Hardware Configuration  
#define SENSOR_PIN 0

// Timing Configuration
#define UPDATE_INTERVAL 1000 
#define WIFI_CHECK_INTERVAL 30000  // 30 seconds

// Sensor Configuration
#define DRY_THRESHOLD 55
#define MOISTURE_SAMPLES 20

// ADC calibration points for the moisture estimate (0-4095).
// These are example values: replace them with readings from your own sensor.
// The raw value may rise or fall as the soil gets wetter; either direction works.
#define MOISTURE_RAW_DRY 500
#define MOISTURE_RAW_25  750
#define MOISTURE_RAW_50  1000
#define MOISTURE_RAW_75  1500
#define MOISTURE_RAW_100 2000

// Web Server Configuration
#define WEB_SERVER_PORT 80

#endif
