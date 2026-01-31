/**
 * @file brewProfiles.cpp
 *
 * @brief Brew profile management implementation
 *
 */

#include "brewProfiles.h"
#include "Config.h"
#include "Logger.h"
#include <Preferences.h>

BrewProfile profiles[MAX_PROFILES];
uint8_t activeProfileIndex = NO_ACTIVE_PROFILE;  // No profile active initially
Preferences profilePrefs;

// External references to global config variables
extern Config config;
extern double brewSetpoint;
extern double steamSetpoint;
extern double brewTempOffset;
extern double preinfusion;
extern double preinfusionPause;
extern double targetBrewTime;
extern double aggKp, aggTn, aggTv;

void initProfiles() {
    profilePrefs.begin("profiles", false);
    loadProfiles();
    
    LOG(INFO, "Brew profiles initialized. Active profile: %d", activeProfileIndex);
}

void loadProfiles() {
    size_t len = profilePrefs.getBytes("profiles", profiles, sizeof(profiles));
    
    if (len != sizeof(profiles)) {
        // First time or corrupted - initialize with defaults
        LOG(WARN, "Profiles not found or corrupted, initializing defaults");
        memset(profiles, 0, sizeof(profiles));
        
        // Create one default profile
        strncpy(profiles[0].name, "Default", PROFILE_NAME_LENGTH);
        profiles[0].active = true;
        profiles[0].brewSetpoint = 94.0;
        profiles[0].steamSetpoint = 120.0;
        profiles[0].brewTempOffset = 0.0;
        profiles[0].preInfusionEnabled = true;
        profiles[0].preInfusionTime = 2.0;
        profiles[0].preInfusionPause = 5.0;
        profiles[0].brewMode = 0;
        profiles[0].brewByTimeEnabled = false;
        profiles[0].targetBrewTime = 25.0;
        profiles[0].brewByWeightEnabled = false;
        profiles[0].targetBrewWeight = 36.0;
        profiles[0].customPID = false;
        
        saveProfiles();
    }
    
    activeProfileIndex = profilePrefs.getUChar("activeIndex", NO_ACTIVE_PROFILE);
}

void saveProfiles() {
    profilePrefs.putBytes("profiles", profiles, sizeof(profiles));
    profilePrefs.putUChar("activeIndex", activeProfileIndex);
    LOG(INFO, "Profiles saved to flash");
}

void loadProfile(uint8_t index) {
    if (index >= MAX_PROFILES) {
        LOG(ERROR, "Invalid profile index: %d", index);
        return;
    }
    
    if (!profiles[index].active) {
        LOG(WARN, "Profile %d is not active", index);
        return;
    }
    
    LOG(INFO, "Loading profile %d: %s", index, profiles[index].name);
    
    applyProfileSettings(profiles[index]);
    activeProfileIndex = index;
    saveProfiles();
    
    LOG(INFO, "Profile loaded successfully");
}

void applyProfileSettings(const BrewProfile& profile) {
    // Temperature
    brewSetpoint = profile.brewSetpoint;
    steamSetpoint = profile.steamSetpoint;
    brewTempOffset = profile.brewTempOffset;
    config.set("brew.setpoint", brewSetpoint);
    config.set("steam.setpoint", steamSetpoint);
    config.set("brew.temp_offset", brewTempOffset);
    
    // Pre-infusion
    config.set("brew.pre_infusion.enabled", profile.preInfusionEnabled);
    preinfusion = profile.preInfusionTime;
    preinfusionPause = profile.preInfusionPause;
    config.set("brew.pre_infusion.time", preinfusion);
    config.set("brew.pre_infusion.pause", preinfusionPause);
    
    // Brew control
    config.set("brew.mode", profile.brewMode);
    config.set("brew.by_time.enabled", profile.brewByTimeEnabled);
    targetBrewTime = profile.targetBrewTime;
    config.set("brew.by_time.target_time", targetBrewTime);
    config.set("brew.by_weight.enabled", profile.brewByWeightEnabled);
    config.set("brew.by_weight.target_weight", profile.targetBrewWeight);
    
    // PID (if custom)
    if (profile.customPID) {
        aggKp = profile.pidKp;
        aggTn = profile.pidTn;
        aggTv = profile.pidTv;
        config.set("pid.regular.kp", aggKp);
        config.set("pid.regular.tn", aggTn);
        config.set("pid.regular.tv", aggTv);
    }
}

void captureCurrentSettings(BrewProfile& profile) {
    // Temperature
    profile.brewSetpoint = brewSetpoint;
    profile.steamSetpoint = steamSetpoint;
    profile.brewTempOffset = brewTempOffset;
    
    // Pre-infusion
    profile.preInfusionEnabled = config.get<bool>("brew.pre_infusion.enabled");
    profile.preInfusionTime = preinfusion;
    profile.preInfusionPause = preinfusionPause;
    
    // Brew control
    profile.brewMode = config.get<int>("brew.mode");
    profile.brewByTimeEnabled = config.get<bool>("brew.by_time.enabled");
    profile.targetBrewTime = targetBrewTime;
    profile.brewByWeightEnabled = config.get<bool>("brew.by_weight.enabled");
    profile.targetBrewWeight = config.get<double>("brew.by_weight.target_weight");
    
    // PID
    profile.customPID = false;  // Default to using global PID
    profile.pidKp = aggKp;
    profile.pidTn = aggTn;
    profile.pidTv = aggTv;
}

void saveCurrentAsProfile(uint8_t index, const char* name) {
    if (index >= MAX_PROFILES) {
        LOG(ERROR, "Invalid profile index: %d", index);
        return;
    }
    
    LOG(INFO, "Saving current settings as profile %d: %s", index, name);
    
    captureCurrentSettings(profiles[index]);
    strncpy(profiles[index].name, name, PROFILE_NAME_LENGTH - 1);
    profiles[index].name[PROFILE_NAME_LENGTH - 1] = '\0';
    profiles[index].active = true;
    
    activeProfileIndex = index;
    saveProfiles();
    
    LOG(INFO, "Profile saved successfully");
}

void deleteProfile(uint8_t index) {
    if (index >= MAX_PROFILES) return;
    
    memset(&profiles[index], 0, sizeof(BrewProfile));
    profiles[index].active = false;
    
    if (activeProfileIndex == index) {
        activeProfileIndex = NO_ACTIVE_PROFILE;
    }
    
    saveProfiles();
    LOG(INFO, "Profile %d deleted", index);
}

void renameProfile(uint8_t index, const char* newName) {
    if (index >= MAX_PROFILES || !profiles[index].active) return;
    
    strncpy(profiles[index].name, newName, PROFILE_NAME_LENGTH - 1);
    profiles[index].name[PROFILE_NAME_LENGTH - 1] = '\0';
    saveProfiles();
    
    LOG(INFO, "Profile %d renamed to: %s", index, newName);
}

const char* getProfileName(uint8_t index) {
    if (index >= MAX_PROFILES || !profiles[index].active) {
        return "Empty";
    }
    return profiles[index].name;
}

bool isProfileActive(uint8_t index) {
    return (index < MAX_PROFILES) && profiles[index].active;
}
