/**
 * @file brewTimer.h
 *
 * @brief Enhanced brew timer with phase tracking
 *
 */

#ifndef BREW_TIMER_H
#define BREW_TIMER_H

#include <Arduino.h>

enum BrewPhaseDisplay {
    PHASE_IDLE,
    PHASE_PRE_INF_SINGLE,
    PHASE_PRE_INF_PULSE_ON,
    PHASE_PRE_INF_PULSE_OFF,
    PHASE_PRE_INF_SOAK,
    PHASE_BREWING,
    PHASE_FINISHED
};

struct BrewTimerInfo {
    BrewPhaseDisplay currentPhase;
    uint8_t pulseCycleNumber;        // Current cycle (1-based)
    uint8_t totalPulseCycles;        // Total cycles configured
    unsigned long phaseStartTime;    // millis() when phase started
    unsigned long totalBrewStartTime;// millis() when brew started
    double phaseElapsedTime;         // Seconds in current phase
    double totalElapsedTime;         // Seconds since brew start
    double phaseExpectedDuration;    // Expected phase duration for progress bar
};

extern BrewTimerInfo brewTimerInfo;

void initBrewTimer();
void startBrewTimer();
void updateBrewTimer();
void setBrewPhase(BrewPhaseDisplay phase, uint8_t cycleNum = 0, double expectedDuration = 0);
const char* getPhaseDisplayName();
uint8_t getPhaseProgress(); // Returns 0-100 for progress bar

#endif // BREW_TIMER_H
