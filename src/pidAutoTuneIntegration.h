/**
 * @file pidAutoTuneIntegration.h
 *
 * @brief PID Auto-tuning integration using br3ttb/Arduino-PID-AutoTune-Library
 *
 * This file integrates the proven PID_AutoTune_v0 library by Brett Beauregard
 * with the CleverCoffee espresso machine controller. The library uses the
 * relay feedback method to automatically determine optimal PID parameters.
 *
 * Library: https://github.com/br3ttb/Arduino-PID-AutoTune-Library
 */

#pragma once

#include <Arduino.h>
#include <PID_AutoTune_v0.h>
#include "Logger.h"

// Forward declarations
extern double temperature;
extern double pidOutput;
extern double setpoint;
extern PID bPID;

class PIDAutoTuneManager {
    private:
        PID_ATune* autoTuner = nullptr;
        bool tuningActive = false;
        bool tuningComplete = false;
        
        // Calculated results
        double tunedKp = 0;
        double tunedKi = 0;
        double tunedKd = 0;
        
        // Progress tracking
        unsigned long tuneStartTime = 0;
        int progressPercent = 0;
        String statusMessage = "Idle";
        
        // Configuration - optimized for espresso machine boilers
        double targetTemp = 95.0;
        double outputStep = 50.0;     // Output step for relay feedback (0-100%)
        double noiseBand = 1.0;        // Temperature noise band (°C) - larger for espresso machines
        int lookBack = 30;             // Lookback time in seconds - longer for thermal lag
        
    public:
        /**
         * @brief Start the auto-tuning process
         * @param target Target temperature for tuning
         * @return true if tuning started successfully
         */
        bool start(double target) {
            if (tuningActive) {
                LOG(WARNING, "Auto-tune already in progress");
                return false;
            }
            
            targetTemp = target;
            setpoint = targetTemp;
            
            // Create new AutoTune instance
            if (autoTuner != nullptr) {
                delete autoTuner;
            }
            autoTuner = new PID_ATune(&temperature, &pidOutput);
            
            // Configure AutoTune parameters
            autoTuner->SetNoiseBand(noiseBand);
            autoTuner->SetOutputStep(outputStep * 10); // Convert to 0-1000 range
            autoTuner->SetLookbackSec(lookBack);
            autoTuner->SetControlType(1); // PID control type
            
            tuningActive = true;
            tuningComplete = false;
            tuneStartTime = millis();
            progressPercent = 0;
            statusMessage = "Auto-tuning in progress...";
            
            LOGF(INFO, "PID Auto-tune started, target: %.1f°C", targetTemp);
            
            return true;
        }
        
        /**
         * @brief Stop the auto-tuning process
         */
        void stop() {
            if (!tuningActive) {
                return;
            }
            
            if (autoTuner != nullptr) {
                autoTuner->Cancel();
            }
            
            tuningActive = false;
            tuningComplete = false;
            statusMessage = "Auto-tune stopped by user";
            progressPercent = 0;
            
            LOG(INFO, "PID Auto-tune stopped by user");
        }
        
        /**
         * @brief Update the auto-tuning process (call from main loop)
         * @return true if tuning is controlling the output
         */
        bool update() {
            if (!tuningActive || autoTuner == nullptr) {
                return false;
            }
            
            // Run the auto-tuner
            int result = autoTuner->Runtime();
            
            if (result != 0) {
                // Tuning complete
                tuningActive = false;
                tuningComplete = true;
                
                // Get the tuned parameters
                tunedKp = autoTuner->GetKp();
                tunedKi = autoTuner->GetKi();
                tunedKd = autoTuner->GetKd();
                
                progressPercent = 100;
                statusMessage = String("Complete! Kp=") + String(tunedKp, 1) + 
                              ", Ki=" + String(tunedKi, 3) + 
                              ", Kd=" + String(tunedKd, 1);
                
                LOGF(INFO, "PID Auto-tune complete: Kp=%.1f, Ki=%.3f, Kd=%.1f", 
                     tunedKp, tunedKi, tunedKd);
                
                return false;
            }
            
            // Update progress based on elapsed time
            // Espresso machine boilers typically take 8-12 minutes due to thermal mass
            unsigned long elapsed = millis() - tuneStartTime;
            progressPercent = constrain((elapsed / 1000) * 100 / 600, 0, 95); // Assume max 10 minutes
            
            // Tuning still in progress
            return true;
        }
        
        /**
         * @brief Check if auto-tuning is active
         */
        bool isActive() const {
            return tuningActive;
        }
        
        /**
         * @brief Check if tuning is complete with results available
         */
        bool isComplete() const {
            return tuningComplete;
        }
        
        /**
         * @brief Get progress percentage (0-100)
         */
        int getProgress() const {
            return progressPercent;
        }
        
        /**
         * @brief Get status message
         */
        String getStatusMessage() const {
            return statusMessage;
        }
        
        /**
         * @brief Get tuned PID parameters
         * @param kp Reference to store Kp value
         * @param ki Reference to store Ki value
         * @param kd Reference to store Kd value
         * @return true if results are available
         */
        bool getResults(double& kp, double& ki, double& kd) const {
            if (!tuningComplete) {
                return false;
            }
            
            kp = tunedKp;
            ki = tunedKi;
            kd = tunedKd;
            
            return true;
        }
        
        /**
         * @brief Convert Ki/Kd to Tn/Tv form used by this system
         * Ki = Kp / Tn  =>  Tn = Kp / Ki
         * Kd = Tv * Kp  =>  Tv = Kd / Kp
         */
        bool getResultsInTnTvForm(double& kp, double& tn, double& tv) const {
            if (!tuningComplete || tunedKp == 0) {
                return false;
            }
            
            kp = tunedKp;
            tn = (tunedKi == 0) ? 0 : tunedKp / tunedKi;
            tv = tunedKp == 0 ? 0 : tunedKd / tunedKp;
            
            return true;
        }
        
        /**
         * @brief Set tuning parameters
         */
        void setTuningParameters(double step, double noise, int lookBackSec) {
            outputStep = step;
            noiseBand = noise;
            lookBack = lookBackSec;
        }
        
        ~PIDAutoTuneManager() {
            if (autoTuner != nullptr) {
                delete autoTuner;
            }
        }
};

// Global instance
inline PIDAutoTuneManager pidAutoTuneManager;
