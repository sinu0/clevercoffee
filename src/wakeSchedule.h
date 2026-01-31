/**
 * @file wakeSchedule.h
 *
 * @brief Scheduled wake-up timer for automatic standby exit
 */

#pragma once

#include <Arduino.h>
#include <time.h>

#define MAX_SCHEDULES 3

struct WakeSchedule {
    bool enabled;
    uint8_t hour;                // 0-23
    uint8_t minute;              // 0-59
    uint8_t daysOfWeek;          // Bitmap: bit 0=Mon, 1=Tue, ..., 6=Sun
    char name[16];               // "Weekdays", "Saturday", etc.
};

extern WakeSchedule schedules[MAX_SCHEDULES];
extern int8_t timezoneOffset;    // Hours from UTC (-12 to +14)
extern bool ntpSynced;
extern time_t nextWakeTime;

void initWakeSchedule();
void setupNTP();
void checkWakeSchedule();
bool shouldWakeNow();
void calculateNextWake();
void saveSchedules();
void loadSchedules();
void enableStandbyIfScheduled();
