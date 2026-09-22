#ifndef SENSOR_H
#define SENSOR_H

#include "config.h"
#include <Arduino.h>

class Sensor {
public:
    Sensor();
    void begin();
    int readMoisture();
    int getAverageMoisture();
    
private:
    int pin;
    int numSamples;
};

#endif
