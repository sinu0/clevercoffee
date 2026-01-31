/**
 * @file brewStats.h
 *
 * @brief Brew statistics tracking and persistence
 *
 */

#ifndef BREW_STATS_H
#define BREW_STATS_H

#include <Arduino.h>

struct BrewStatistics {
    // Counters
    uint32_t totalShots;        // Total shots since installation
    uint32_t dailyShots;        // Shots today
    uint32_t weeklyShots;       // Shots this week
    uint32_t monthlyShots;      // Shots this month

    // Aggregated metrics
    double totalBrewTime;       // Total brew time (seconds)
    double avgBrewTime;         // Average brew time
    double avgWeight;           // Average shot weight (if scale enabled)

    // Reset timestamps
    uint32_t lastResetTimestamp;   // Last daily reset timestamp (seconds since boot via millis()/1000)
    uint32_t weekStartTimestamp;   // Week start for weekly counter (seconds since boot)
    uint32_t monthStartTimestamp;  // Month start for monthly counter (seconds since boot)
                                    // NOTE: These use uptime, not calendar time. Resets are based on elapsed time
                                    // since last reset, not actual midnight/Monday/month boundaries.

    // Last 10 shots circular buffer
    struct ShotRecord {
        uint32_t timestamp;         // Seconds since boot (millis()/1000) - resets on device restart
        double brewTime;
        double weight;
        double temperature;
        uint8_t preInfusionMode;
    } lastShots[10];

    uint8_t lastShotIndex;      // Current position in circular buffer
};

extern BrewStatistics brewStats;

// Function declarations
void initBrewStats();
void saveBrewStats();
void loadBrewStats();
void recordShot(double brewTime, double weight, double temperature, uint8_t preInfMode);
void resetDailyStats();
void resetWeeklyStats();
void resetMonthlyStats();
void resetAllStats();
void updateAverages();

#endif // BREW_STATS_H
