/**
 * @file tempStability.cpp
 *
 * @brief Temperature stability monitoring implementation
 *
 */

#include "tempStability.h"
#include "Logger.h"
#include <math.h>

TempStability tempStability;

void initTempStability() {
    memset(&tempStability, 0, sizeof(tempStability));
    tempStability.stabilityThreshold = 0.3;  // Default ±0.3°C
    tempStability.stabilityDuration = 5;     // Default 5 seconds
    LOG(INFO, "Temperature stability monitoring initialized");
}

void addTempReading(double temperature, double setpoint) {
    unsigned long now = millis();
    
    // Only add reading every 1 second
    if (now - tempStability.lastReadingTime < 1000) return;
    
    tempStability.readings[tempStability.readingIndex] = temperature;
    tempStability.readingIndex = (tempStability.readingIndex + 1) % TEMP_READINGS_BUFFER;
    tempStability.lastReadingTime = now;
    
    // Check stability
    checkTempStability(setpoint);
}

bool checkTempStability(double setpoint) {
    // Need at least 10 readings
    static uint8_t readingCount = 0;
    if (readingCount < TEMP_READINGS_BUFFER) {
        readingCount++;
        return false;
    }
    
    // Calculate mean temperature
    double mean = 0;
    for (int i = 0; i < TEMP_READINGS_BUFFER; i++) {
        mean += tempStability.readings[i];
    }
    mean /= TEMP_READINGS_BUFFER;
    
    // Calculate standard deviation
    double variance = 0;
    for (int i = 0; i < TEMP_READINGS_BUFFER; i++) {
        variance += pow(tempStability.readings[i] - mean, 2);
    }
    double stdDev = sqrt(variance / TEMP_READINGS_BUFFER);
    
    // Check if temperature is stable
    bool isCloseToSetpoint = (fabs(mean - setpoint) < tempStability.stabilityThreshold);
    bool isLowVariance = (stdDev < tempStability.stabilityThreshold);
    bool stable = isCloseToSetpoint && isLowVariance;
    
    unsigned long now = millis();
    
    if (stable) {
        if (!tempStability.isStable) {
            // Just became stable
            tempStability.stableStartTime = now;
        }
        
        // Check if stable for required duration
        unsigned long stableDuration = (now - tempStability.stableStartTime) / 1000;
        if (stableDuration >= tempStability.stabilityDuration) {
            if (!tempStability.isStable) {
                tempStability.isStable = true;
                tempStability.stableTemp = mean;
                LOGF(INFO, "Temperature READY: %.1f°C (stdDev: %.2f)", mean, stdDev);
            }
        }
    } else {
        if (tempStability.isStable) {
            LOG(INFO, "Temperature no longer stable");
        }
        tempStability.isStable = false;
        tempStability.stableStartTime = 0;
    }
    
    return tempStability.isStable;
}

double calculateStdDev() {
    double mean = 0;
    for (int i = 0; i < TEMP_READINGS_BUFFER; i++) {
        mean += tempStability.readings[i];
    }
    mean /= TEMP_READINGS_BUFFER;
    
    double variance = 0;
    for (int i = 0; i < TEMP_READINGS_BUFFER; i++) {
        variance += pow(tempStability.readings[i] - mean, 2);
    }
    
    return sqrt(variance / TEMP_READINGS_BUFFER);
}

void resetStability() {
    tempStability.isStable = false;
    tempStability.stableStartTime = 0;
}
