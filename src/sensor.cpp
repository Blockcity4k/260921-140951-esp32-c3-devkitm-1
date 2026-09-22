#include "sensor.h"

Sensor::Sensor() : pin(0), numSamples(20) {
    // Constructor
}

void Sensor::begin() {
    pinMode(pin, INPUT);
}

int Sensor::readMoisture() {
    return analogRead(pin);
}

int Sensor::getAverageMoisture() {
    long sum = 0;
    int validReadings = 0;
    
    // Take multiple samples to ensure accuracy
    for (int i = 0; i < numSamples; i++) {
        int reading = analogRead(pin);
        // Add basic validation to filter out bad readings
        if (reading > 0 && reading < 4095) {
            sum += reading;
            validReadings++;
        }
        delay(5); 
    }
    
    if (validReadings > 0) {
        return sum / validReadings;
    }
    return -1; // Error code
}
