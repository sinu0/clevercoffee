/**
 * @file brewTimer.cpp
 *
 * @brief Enhanced brew timer implementation
 *
 */

#include "brewTimer.h"
#include "Logger.h"
#include <cstring>

BrewTimerInfo brewTimerInfo;

void initBrewTimer() {
    memset(&brewTimerInfo, 0, sizeof(brewTimerInfo));
    brewTimerInfo.currentPhase = PHASE_IDLE;
}

void startBrewTimer() {
    brewTimerInfo.totalBrewStartTime = millis();
    brewTimerInfo.phaseStartTime = millis();
    brewTimerInfo.totalElapsedTime = 0;
    brewTimerInfo.phaseElapsedTime = 0;
    LOG(INFO, "Brew timer started");
}

void updateBrewTimer() {
    unsigned long now = millis();
    
    brewTimerInfo.totalElapsedTime = (now - brewTimerInfo.totalBrewStartTime) / 1000.0;
    brewTimerInfo.phaseElapsedTime = (now - brewTimerInfo.phaseStartTime) / 1000.0;
}

void setBrewPhase(BrewPhaseDisplay phase, uint8_t cycleNum, double expectedDuration) {
    brewTimerInfo.currentPhase = phase;
    brewTimerInfo.pulseCycleNumber = cycleNum;
    brewTimerInfo.phaseStartTime = millis();
    brewTimerInfo.phaseElapsedTime = 0;
    brewTimerInfo.phaseExpectedDuration = expectedDuration;
    
    LOG(DEBUG, "Brew phase changed to: %s", getPhaseDisplayName());
}

const char* getPhaseDisplayName() {
    switch (brewTimerInfo.currentPhase) {
        case PHASE_IDLE:
            return "IDLE";
        case PHASE_PRE_INF_SINGLE:
            return "PRE-INF";
        case PHASE_PRE_INF_PULSE_ON:
            return "PULSE ON";
        case PHASE_PRE_INF_PULSE_OFF:
            return "PULSE OFF";
        case PHASE_PRE_INF_SOAK:
            return "SOAK";
        case PHASE_BREWING:
            return "BREWING";
        case PHASE_FINISHED:
            return "DONE";
        default:
            return "UNKNOWN";
    }
}

uint8_t getPhaseProgress() {
    if (brewTimerInfo.phaseExpectedDuration <= 0) return 0;
    
    uint8_t progress = (uint8_t)((brewTimerInfo.phaseElapsedTime / brewTimerInfo.phaseExpectedDuration) * 100);
    return (progress > 100) ? 100 : progress;
}
