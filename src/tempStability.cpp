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
    
    // Increment reading count up to buffer size
    if (tempStability.readingCount < TEMP_READINGS_BUFFER) {
        tempStability.readingCount++;
    }
    
    // Check stability
    checkTempStability(setpoint);
}

// Helper function to calculate mean temperature
static double calculateMean() {
    // Only calculate mean from valid readings
    if (tempStability.readingCount == 0) {
        return 0.0;
    }
    
    double mean = 0;
    int count = tempStability.readingCount < TEMP_READINGS_BUFFER ? tempStability.readingCount : TEMP_READINGS_BUFFER;
    for (int i = 0; i < count; i++) {
        mean += tempStability.readings[i];
    }
    return mean / count;
}

bool checkTempStability(double setpoint) {
    // Need at least TEMP_READINGS_BUFFER readings
    if (tempStability.readingCount < TEMP_READINGS_BUFFER) {
        return false;
    }
    
    // Calculate standard deviation and mean
    double stdDev = calculateStdDev();
    double mean = calculateMean();
    
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
        // Properly handle millis() rollover by checking if we've exceeded a reasonable maximum
        // If the calculated duration is > 1 hour, assume rollover occurred and we're just starting
        unsigned long stableDuration = (unsigned long)(now - tempStability.stableStartTime) / 1000;
        if (stableDuration > 3600) {
            // Likely a rollover, reset the start time
            tempStability.stableStartTime = now;
            stableDuration = 0;
        }
        
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
    // Need valid readings to calculate standard deviation
    if (tempStability.readingCount == 0) {
        return 0.0;
    }
    
    double mean = calculateMean();
    
    int count = tempStability.readingCount < TEMP_READINGS_BUFFER ? tempStability.readingCount : TEMP_READINGS_BUFFER;
    double variance = 0;
    for (int i = 0; i < count; i++) {
        variance += pow(tempStability.readings[i] - mean, 2);
    }
    
    return sqrt(variance / count);
}

void resetStability() {
    tempStability.isStable = false;
    tempStability.stableStartTime = 0;
    tempStability.readingCount = 0;
    tempStability.readingIndex = 0;
    memset(tempStability.readings, 0, sizeof(tempStability.readings));
}

bool isTempReadyIndicatorActive() {
    extern bool tempReadyEnabled;
    extern bool steamON;
    extern MachineState machineState;
    
    return tempReadyEnabled && tempStability.isStable && machineState == kPidNormal && !steamON;
}
