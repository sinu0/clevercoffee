/**
 * @file brewProfiles.h
 *
 * @brief Brew profile management - save and load brewing configurations
 *
 */

#ifndef BREW_PROFILES_H
#define BREW_PROFILES_H

#include <Arduino.h>

#define MAX_PROFILES 5
#define PROFILE_NAME_LENGTH 20

struct BrewProfile {
    char name[PROFILE_NAME_LENGTH];  // User-defined name
    bool active;                     // Is this profile slot used?
    
    // Temperature settings
    double brewSetpoint;             // Brew temperature (°C)
    double steamSetpoint;            // Steam temperature (°C)
    double brewTempOffset;           // Temperature offset
    
    // Pre-infusion settings
    bool preInfusionEnabled;
    double preInfusionTime;          // Pre-infusion time
    double preInfusionPause;         // Pre-infusion pause time
    
    // Brew control
    int brewMode;                    // 0=Manual, 1=Automatic
    bool brewByTimeEnabled;
    double targetBrewTime;           // Target brew duration
    bool brewByWeightEnabled;
    double targetBrewWeight;         // Target shot weight (g)
    
    // PID settings (optional - for advanced users)
    bool customPID;                  // If false, use global PID
    double pidKp;
    double pidTn;
    double pidTv;
};

extern BrewProfile profiles[MAX_PROFILES];
extern uint8_t activeProfileIndex;   // 0-4, or 0xFF for none

// Function declarations
void initProfiles();
void saveProfiles();
void loadProfiles();
void loadProfile(uint8_t index);
void saveCurrentAsProfile(uint8_t index, const char* name);
void deleteProfile(uint8_t index);
void renameProfile(uint8_t index, const char* newName);
const char* getProfileName(uint8_t index);
bool isProfileActive(uint8_t index);
void applyProfileSettings(const BrewProfile& profile);
void captureCurrentSettings(BrewProfile& profile);

#endif // BREW_PROFILES_H
