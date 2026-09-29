#ifndef DATA_HANDLER_H
#define DATA_HANDLER_H

#include <Arduino.h>
#include "config.h"

class DataHandler {
public:
    DataHandler();
    void begin();
    void addReading(int moisture, unsigned long timestamp);
    int getLatestReading();
    unsigned long getLatestTimestamp();
    int getReadingCount();
    uint64_t getTotalReadingCount();
    int getRecentReadings(int maxReadings, int* outputArray, unsigned long* timeStamps);
    int getAverageReading();
    
private:
    static const int MAX_HISTORY = DATA_HISTORY_SIZE;
    int moistureHistory[MAX_HISTORY];
    unsigned long timestampHistory[MAX_HISTORY];
    int readIndex;
    int readingCount;
    uint64_t totalReadingCount;
};

#endif
