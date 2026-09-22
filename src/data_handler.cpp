#include "data_handler.h"

DataHandler::DataHandler() {
    readIndex = 0;
    readingCount = 0;
}

void DataHandler::begin() {
    // Initialize data structures
    for (int i = 0; i < MAX_HISTORY; i++) {
        moistureHistory[i] = 0;
        timestampHistory[i] = 0;
    }
}

void DataHandler::addReading(int moisture, unsigned long timestamp) {
    int index = (readIndex++) % MAX_HISTORY;
    moistureHistory[index] = moisture;
    timestampHistory[index] = timestamp;
    
    if (readingCount < MAX_HISTORY) {
        readingCount++;
    }
}

int DataHandler::getLatestReading() {
    if (readingCount == 0) return -1;
    int index = (readIndex - 1 + MAX_HISTORY) % MAX_HISTORY;
    return moistureHistory[index];
}

unsigned long DataHandler::getLatestTimestamp() {
    if (readingCount == 0) return 0;
    int index = (readIndex - 1 + MAX_HISTORY) % MAX_HISTORY;
    return timestampHistory[index];
}

void DataHandler::getRecentReadings(int maxReadings, int* outputArray, unsigned long* timeStamps) {
    int actualReadings = min(readingCount, maxReadings);
    int startIndex = (readIndex - actualReadings + MAX_HISTORY) % MAX_HISTORY;
    
    for (int i = 0; i < actualReadings; i++) {
        int index = (startIndex + i) % MAX_HISTORY;
        outputArray[i] = moistureHistory[index];
        timeStamps[i] = timestampHistory[index];
    }
}
