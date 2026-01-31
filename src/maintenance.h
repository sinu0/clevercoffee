/**
 * @file maintenance.h
 *
 * @brief Maintenance tracking and reminders
 *
 */

#ifndef MAINTENANCE_H
#define MAINTENANCE_H

#include <Arduino.h>

struct MaintenanceTracker {
    // Descaling
    uint32_t shotsSinceDescale;
    uint32_t descaleInterval;          // shots (default: 200)
    uint32_t lastDescaleTimestamp;     // millis() when last done
    
    // Backflushing
    uint32_t shotsSinceBackflush;
    uint32_t backflushInterval;        // shots (default: 50)
    uint32_t daysSinceBackflush;
    uint32_t backflushDaysInterval;    // days (default: 7)
    uint32_t lastBackflushTimestamp;
    
    // Filter basket cleaning
    uint32_t shotsSinceBasketClean;
    uint32_t basketCleanInterval;      // shots (default: 50)
    uint32_t lastBasketCleanTimestamp;
    
    // Water tank refill reminder
    uint32_t shotsSinceRefill;
    uint32_t refillReminderInterval;   // shots (default: 30)
    uint32_t lastRefillTimestamp;
    
    // Status flags
    bool descaleDue;
    bool backflushDue;
    bool basketCleanDue;
    bool refillDue;
};

extern MaintenanceTracker maintenance;

void initMaintenance();
void saveMaintenance();
void loadMaintenance();
void checkMaintenanceDue();
void incrementShotCounters();
void checkDailyMaintenance();
void resetDescaleCounter();
void resetBackflushCounter();
void resetBasketCleanCounter();
void resetRefillCounter();

#endif // MAINTENANCE_H
