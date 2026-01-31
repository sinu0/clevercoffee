/**
 * @file maintenance.cpp
 *
 * @brief Maintenance tracking and reminders implementation
 *
 */

#include "maintenance.h"
#include <Preferences.h>
#include "Logger.h"

MaintenanceTracker maintenance;
Preferences maintenancePrefs;

void initMaintenance() {
    maintenancePrefs.begin("maintenance", false);
    loadMaintenance();
    checkMaintenanceDue();
    
    LOG(INFO, "Maintenance tracking initialized");
    LOGF(INFO, "Shots since descale: %d/%d", maintenance.shotsSinceDescale, maintenance.descaleInterval);
}

void loadMaintenance() {
    maintenance.shotsSinceDescale = maintenancePrefs.getUInt("descaleShots", 0);
    maintenance.descaleInterval = maintenancePrefs.getUInt("descaleInt", 200);
    maintenance.lastDescaleTimestamp = maintenancePrefs.getUInt("descaleTime", 0);
    
    maintenance.shotsSinceBackflush = maintenancePrefs.getUInt("backflushShots", 0);
    maintenance.backflushInterval = maintenancePrefs.getUInt("backflushInt", 50);
    maintenance.daysSinceBackflush = maintenancePrefs.getUInt("backflushDays", 0);
    maintenance.backflushDaysInterval = maintenancePrefs.getUInt("backflushDInt", 7);
    maintenance.lastBackflushTimestamp = maintenancePrefs.getUInt("backflushTime", 0);
    
    maintenance.shotsSinceBasketClean = maintenancePrefs.getUInt("basketShots", 0);
    maintenance.basketCleanInterval = maintenancePrefs.getUInt("basketInt", 50);
    maintenance.lastBasketCleanTimestamp = maintenancePrefs.getUInt("basketTime", 0);
    
    maintenance.shotsSinceRefill = maintenancePrefs.getUInt("refillShots", 0);
    maintenance.refillReminderInterval = maintenancePrefs.getUInt("refillInt", 30);
    maintenance.lastRefillTimestamp = maintenancePrefs.getUInt("refillTime", 0);
}

void saveMaintenance() {
    maintenancePrefs.putUInt("descaleShots", maintenance.shotsSinceDescale);
    maintenancePrefs.putUInt("descaleInt", maintenance.descaleInterval);
    maintenancePrefs.putUInt("descaleTime", maintenance.lastDescaleTimestamp);
    
    maintenancePrefs.putUInt("backflushShots", maintenance.shotsSinceBackflush);
    maintenancePrefs.putUInt("backflushInt", maintenance.backflushInterval);
    maintenancePrefs.putUInt("backflushDays", maintenance.daysSinceBackflush);
    maintenancePrefs.putUInt("backflushDInt", maintenance.backflushDaysInterval);
    maintenancePrefs.putUInt("backflushTime", maintenance.lastBackflushTimestamp);
    
    maintenancePrefs.putUInt("basketShots", maintenance.shotsSinceBasketClean);
    maintenancePrefs.putUInt("basketInt", maintenance.basketCleanInterval);
    maintenancePrefs.putUInt("basketTime", maintenance.lastBasketCleanTimestamp);
    
    maintenancePrefs.putUInt("refillShots", maintenance.shotsSinceRefill);
    maintenancePrefs.putUInt("refillInt", maintenance.refillReminderInterval);
    maintenancePrefs.putUInt("refillTime", maintenance.lastRefillTimestamp);
}

void checkMaintenanceDue() {
    maintenance.descaleDue = 
        (maintenance.shotsSinceDescale >= maintenance.descaleInterval);
    
    maintenance.backflushDue = 
        (maintenance.shotsSinceBackflush >= maintenance.backflushInterval) ||
        (maintenance.daysSinceBackflush >= maintenance.backflushDaysInterval);
    
    maintenance.basketCleanDue = 
        (maintenance.shotsSinceBasketClean >= maintenance.basketCleanInterval);
    
    maintenance.refillDue = 
        (maintenance.shotsSinceRefill >= maintenance.refillReminderInterval);
    
    if (maintenance.descaleDue) LOG(WARN, "DESCALING DUE!");
    if (maintenance.backflushDue) LOG(WARN, "BACKFLUSHING DUE!");
}

void incrementShotCounters() {
    maintenance.shotsSinceDescale++;
    maintenance.shotsSinceBackflush++;
    maintenance.shotsSinceBasketClean++;
    maintenance.shotsSinceRefill++;
    
    checkMaintenanceDue();
    saveMaintenance();
}

void checkDailyMaintenance() {
    maintenance.daysSinceBackflush++;
    checkMaintenanceDue();
    saveMaintenance();
}

void resetDescaleCounter() {
    maintenance.shotsSinceDescale = 0;
    maintenance.lastDescaleTimestamp = millis();
    maintenance.descaleDue = false;
    saveMaintenance();
    LOG(INFO, "Descale counter reset");
}

void resetBackflushCounter() {
    maintenance.shotsSinceBackflush = 0;
    maintenance.daysSinceBackflush = 0;
    maintenance.lastBackflushTimestamp = millis();
    maintenance.backflushDue = false;
    saveMaintenance();
    LOG(INFO, "Backflush counter reset");
}

void resetBasketCleanCounter() {
    maintenance.shotsSinceBasketClean = 0;
    maintenance.lastBasketCleanTimestamp = millis();
    maintenance.basketCleanDue = false;
    saveMaintenance();
    LOG(INFO, "Basket clean counter reset");
}

void resetRefillCounter() {
    maintenance.shotsSinceRefill = 0;
    maintenance.lastRefillTimestamp = millis();
    maintenance.refillDue = false;
    saveMaintenance();
    LOG(INFO, "Refill counter reset");
}
