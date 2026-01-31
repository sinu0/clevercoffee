/**
 * @file brewStats.cpp
 *
 * @brief Brew statistics tracking and persistence implementation
 *
 */

#include "brewStats.h"
#include "Logger.h"
#include <ArduinoJson.h>
#include <LittleFS.h>

#define STATS_FILE "/brewstats.json"

BrewStatistics brewStats;

void initBrewStats() {
    loadBrewStats();

    // Check if daily/weekly/monthly reset needed
    uint32_t now = millis() / 1000;

    // Daily reset (check every startup)
    if (brewStats.lastResetTimestamp > 0 && now - brewStats.lastResetTimestamp > 86400) {
        resetDailyStats();
    }

    LOG(INFO, "Brew statistics initialized. Total shots: %d", brewStats.totalShots);
}

void loadBrewStats() {
    // Initialize to zeros
    memset(&brewStats, 0, sizeof(brewStats));

    if (!LittleFS.exists(STATS_FILE)) {
        LOG(INFO, "Stats file does not exist, starting fresh");
        return;
    }

    File file = LittleFS.open(STATS_FILE, "r");

    if (!file) {
        LOG(ERROR, "Failed to open stats file for reading");
        return;
    }

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, file);
    file.close();

    if (error) {
        LOG(ERROR, "Failed to parse stats file");
        return;
    }

    // Load counters
    brewStats.totalShots = doc["totalShots"] | 0;
    brewStats.dailyShots = doc["dailyShots"] | 0;
    brewStats.weeklyShots = doc["weeklyShots"] | 0;
    brewStats.monthlyShots = doc["monthlyShots"] | 0;

    // Load aggregated metrics
    brewStats.totalBrewTime = doc["totalTime"] | 0.0;
    brewStats.avgBrewTime = doc["avgTime"] | 0.0;
    brewStats.avgWeight = doc["avgWeight"] | 0.0;

    // Load reset timestamps
    brewStats.lastResetTimestamp = doc["lastReset"] | 0;
    brewStats.weekStartTimestamp = doc["weekStart"] | 0;
    brewStats.monthStartTimestamp = doc["monthStart"] | 0;

    // Load circular buffer
    brewStats.lastShotIndex = doc["lastIndex"] | 0;

    if (doc.containsKey("lastShots")) {
        JsonArray shots = doc["lastShots"];
        int idx = 0;

        for (JsonVariant shot : shots) {
            if (idx >= 10)
                break;
            brewStats.lastShots[idx].timestamp = shot["timestamp"] | 0;
            brewStats.lastShots[idx].brewTime = shot["brewTime"] | 0.0;
            brewStats.lastShots[idx].weight = shot["weight"] | 0.0;
            brewStats.lastShots[idx].temperature = shot["temperature"] | 0.0;
            brewStats.lastShots[idx].preInfusionMode = shot["preInfMode"] | 0;
            idx++;
        }
    }

    LOG(INFO, "Loaded brew statistics from file");
}

void saveBrewStats() {
    JsonDocument doc;

    // Save counters
    doc["totalShots"] = brewStats.totalShots;
    doc["dailyShots"] = brewStats.dailyShots;
    doc["weeklyShots"] = brewStats.weeklyShots;
    doc["monthlyShots"] = brewStats.monthlyShots;

    // Save aggregated metrics
    doc["totalTime"] = brewStats.totalBrewTime;
    doc["avgTime"] = brewStats.avgBrewTime;
    doc["avgWeight"] = brewStats.avgWeight;

    // Save reset timestamps
    doc["lastReset"] = brewStats.lastResetTimestamp;
    doc["weekStart"] = brewStats.weekStartTimestamp;
    doc["monthStart"] = brewStats.monthStartTimestamp;

    // Save circular buffer
    doc["lastIndex"] = brewStats.lastShotIndex;

    JsonArray shots = doc.createNestedArray("lastShots");

    for (int i = 0; i < 10; i++) {
        JsonObject shot = shots.createNestedObject();
        shot["timestamp"] = brewStats.lastShots[i].timestamp;
        shot["brewTime"] = brewStats.lastShots[i].brewTime;
        shot["weight"] = brewStats.lastShots[i].weight;
        shot["temperature"] = brewStats.lastShots[i].temperature;
        shot["preInfMode"] = brewStats.lastShots[i].preInfusionMode;
    }

    File file = LittleFS.open(STATS_FILE, "w");

    if (!file) {
        LOG(ERROR, "Failed to open stats file for writing");
        return;
    }

    if (serializeJson(doc, file) == 0) {
        LOG(ERROR, "Failed to write stats file");
    }

    file.close();
}

void recordShot(double brewTime, double weight, double temperature, uint8_t preInfMode) {
    // Increment counters
    brewStats.totalShots++;
    brewStats.dailyShots++;
    brewStats.weeklyShots++;
    brewStats.monthlyShots++;

    // Update totals
    brewStats.totalBrewTime += brewTime;

    // Store in circular buffer
    brewStats.lastShots[brewStats.lastShotIndex].timestamp = millis() / 1000;
    brewStats.lastShots[brewStats.lastShotIndex].brewTime = brewTime;
    brewStats.lastShots[brewStats.lastShotIndex].weight = weight;
    brewStats.lastShots[brewStats.lastShotIndex].temperature = temperature;
    brewStats.lastShots[brewStats.lastShotIndex].preInfusionMode = preInfMode;

    brewStats.lastShotIndex = (brewStats.lastShotIndex + 1) % 10;

    // Update averages
    updateAverages();

    // Save to file
    saveBrewStats();

    LOG(INFO, "Shot recorded: %.1fs, %.1fg, %.1f°C (Total: %d)", brewTime, weight, temperature, brewStats.totalShots);
}

void updateAverages() {
    if (brewStats.totalShots > 0) {
        brewStats.avgBrewTime = brewStats.totalBrewTime / brewStats.totalShots;

        // Calculate average weight from last 10 shots
        double totalWeight = 0;
        int validShots = 0;

        for (int i = 0; i < 10; i++) {
            if (brewStats.lastShots[i].timestamp > 0 && brewStats.lastShots[i].weight > 0) {
                totalWeight += brewStats.lastShots[i].weight;
                validShots++;
            }
        }

        if (validShots > 0) {
            brewStats.avgWeight = totalWeight / validShots;
        }
    }
}

void resetDailyStats() {
    brewStats.dailyShots = 0;
    brewStats.lastResetTimestamp = millis() / 1000;
    saveBrewStats();
    LOG(INFO, "Daily stats reset");
}

void resetWeeklyStats() {
    brewStats.weeklyShots = 0;
    brewStats.weekStartTimestamp = millis() / 1000;
    saveBrewStats();
    LOG(INFO, "Weekly stats reset");
}

void resetMonthlyStats() {
    brewStats.monthlyShots = 0;
    brewStats.monthStartTimestamp = millis() / 1000;
    saveBrewStats();
    LOG(INFO, "Monthly stats reset");
}

void resetAllStats() {
    memset(&brewStats, 0, sizeof(brewStats));
    saveBrewStats();
    LOG(INFO, "All stats reset");
}
