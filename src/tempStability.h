/**
 * @file tempStability.h
 *
 * @brief Temperature stability monitoring for ready indicator
 *
 */

#ifndef TEMP_STABILITY_H
#define TEMP_STABILITY_H

#include <Arduino.h>

#define TEMP_READINGS_BUFFER 10

struct TempStability {
    double readings[TEMP_READINGS_BUFFER];  // Last 10 temperature readings
    uint8_t readingIndex;                   // Current position in circular buffer
    unsigned long lastReadingTime;          // millis() of last reading
    unsigned long stableStartTime;          // millis() when stability started
    bool isStable;                          // Current stability status
    double stableTemp;                      // Temperature when stable
    double stabilityThreshold;              // Max deviation (°C)
    uint8_t stabilityDuration;              // Required duration (seconds)
};

extern TempStability tempStability;

void initTempStability();
void addTempReading(double temperature, double setpoint);
bool checkTempStability(double setpoint);
double calculateStdDev();
void resetStability();

#endif // TEMP_STABILITY_H
