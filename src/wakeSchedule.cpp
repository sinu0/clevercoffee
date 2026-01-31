/**
 * @file wakeSchedule.cpp
 *
 * @brief Implementation of scheduled wake-up timer
 */

#include "wakeSchedule.h"
#include <Preferences.h>
#include "Logger.h"

// External variables from main.cpp
extern bool standbyModeOn;

WakeSchedule schedules[MAX_SCHEDULES];
int8_t timezoneOffset = 0;
bool ntpSynced = false;
time_t nextWakeTime = 0;

Preferences schedulePrefs;

void initWakeSchedule() {
    schedulePrefs.begin("schedule", false);
    loadSchedules();
    
    timezoneOffset = schedulePrefs.getChar("timezone", 0);
    
    setupNTP();
    calculateNextWake();
    
    LOGF(INFO, "Wake schedule initialized (TZ: UTC%+d)", timezoneOffset);
}

void setupNTP() {
    LOG(INFO, "Syncing time with NTP...");
    
    // Set timezone
    char tzStr[20];
    snprintf(tzStr, sizeof(tzStr), "UTC%+d", timezoneOffset);
    setenv("TZ", tzStr, 1);
    tzset();
    
    // Configure NTP
    configTime(timezoneOffset * 3600, 0, "pool.ntp.org", "time.nist.gov", "time.google.com");
    
    // Wait up to 10 seconds for sync
    struct tm timeinfo;
    int attempts = 0;
    while (!getLocalTime(&timeinfo) && attempts < 20) {
        delay(500);
        attempts++;
    }
    
    if (attempts < 20) {
        ntpSynced = true;
        LOGF(INFO, "NTP synced: %04d-%02d-%02d %02d:%02d:%02d",
            timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
            timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    } else {
        LOG(ERROR, "NTP sync failed");
        ntpSynced = false;
    }
}

void loadSchedules() {
    size_t len = schedulePrefs.getBytes("schedules", schedules, sizeof(schedules));
    
    if (len != sizeof(schedules)) {
        // Initialize default: weekdays at 7:00 AM
        memset(schedules, 0, sizeof(schedules));
        
        schedules[0].enabled = false;  // Disabled by default
        schedules[0].hour = 7;
        schedules[0].minute = 0;
        schedules[0].daysOfWeek = 0x1F;  // Mon-Fri (bits 0-4)
        strncpy(schedules[0].name, "Weekdays", sizeof(schedules[0].name));
        
        saveSchedules();
    }
}

void saveSchedules() {
    schedulePrefs.putBytes("schedules", schedules, sizeof(schedules));
    schedulePrefs.putChar("timezone", timezoneOffset);
    calculateNextWake();
}

bool shouldWakeNow() {
    if (!ntpSynced) return false;
    
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) return false;
    
    // Check each schedule
    for (int i = 0; i < MAX_SCHEDULES; i++) {
        if (!schedules[i].enabled) continue;
        
        // Check day of week (Monday = 0)
        uint8_t todayBit = (timeinfo.tm_wday + 6) % 7;  // Convert Sun=0 to Mon=0
        if (!(schedules[i].daysOfWeek & (1 << todayBit))) continue;
        
        // Check time match (within 1-minute window)
        if (timeinfo.tm_hour == schedules[i].hour && 
            timeinfo.tm_min == schedules[i].minute) {
            LOGF(INFO, "Wake schedule triggered: %s", schedules[i].name);
            return true;
        }
    }
    
    return false;
}

void calculateNextWake() {
    if (!ntpSynced) {
        nextWakeTime = 0;
        return;
    }
    
    time_t now;
    time(&now);
    struct tm timeinfo;
    localtime_r(&now, &timeinfo);
    
    time_t earliest = 0;
    
    // Check next 7 days
    for (int day = 0; day < 7; day++) {
        struct tm checkTime = timeinfo;
        checkTime.tm_mday += day;
        mktime(&checkTime);  // Normalize
        
        uint8_t dayBit = (checkTime.tm_wday + 6) % 7;
        
        for (int i = 0; i < MAX_SCHEDULES; i++) {
            if (!schedules[i].enabled) continue;
            if (!(schedules[i].daysOfWeek & (1 << dayBit))) continue;
            
            checkTime.tm_hour = schedules[i].hour;
            checkTime.tm_min = schedules[i].minute;
            checkTime.tm_sec = 0;
            
            time_t wakeTime = mktime(&checkTime);
            
            // Skip if in the past
            if (wakeTime <= now) continue;
            
            if (earliest == 0 || wakeTime < earliest) {
                earliest = wakeTime;
            }
        }
    }
    
    nextWakeTime = earliest;
}

void checkWakeSchedule() {
    if (shouldWakeNow() && standbyModeOn) {
        LOG(INFO, "Exiting standby mode (scheduled wake)");
        standbyModeOn = false;
    }
}

void enableStandbyIfScheduled() {
    // Optional: Auto-enable standby at night if wake schedule is set
    if (!ntpSynced) return;
    
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) return;
    
    // Check if any schedule is enabled
    bool hasSchedule = false;
    for (int i = 0; i < MAX_SCHEDULES; i++) {
        if (schedules[i].enabled) {
            hasSchedule = true;
            break;
        }
    }
    
    if (!hasSchedule) return;
    
    // Enable standby between 10 PM and 5 AM
    if ((timeinfo.tm_hour >= 22 || timeinfo.tm_hour < 5) && !standbyModeOn) {
        LOG(INFO, "Auto-enabling standby mode (night time)");
        standbyModeOn = true;
    }
}
