#ifndef DATA_HANDLER_H
#define DATA_HANDLER_H

#include <Arduino.h>

class DataHandler {
public:
    DataHandler();
    void begin();
    void addReading(int moisture, unsigned long timestamp);
    int getLatestReading();
    unsigned long getLatestTimestamp();
    void getRecentReadings(int maxReadings, int* outputArray, unsigned long* timeStamps);
    
private:
    static const int MAX_HISTORY = 100;
    int moistureHistory[100];
    unsigned long timestampHistory[100];
    int readIndex;
    int readingCount;
};

#endif
