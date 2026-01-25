/**
 * @file pidAutoTune.h
 *
 * @brief PID Auto-tuning using Relay Feedback Test (RFT) method
 *
 * This implementation uses the relay feedback test to automatically determine
 * optimal PID parameters for espresso machine boilers. The method is particularly
 * suitable for systems where temperature sensors may not be directly at the heating
 * element, which is common in espresso machines.
 *
 * The tuning process:
 * 1. Oscillates the heater relay around the setpoint
 * 2. Measures the response (period and amplitude)
 * 3. Calculates PID parameters using modified Ziegler-Nichols rules
 * 4. Optimizes for fast response, minimal overshoot, and stable temperature
 */

#pragma once

#include <Arduino.h>
#include "Logger.h"

enum PidCalibrationState {
    kCalibrationIdle = 0,
    kCalibrationWarmup = 1,      // Initial warmup phase
    kCalibrationOscillation = 2, // Collecting oscillation data
    kCalibrationCooldown = 3,    // Brief cooldown phase
    kCalibrationComplete = 4,    // Tuning complete
    kCalibrationError = 5        // Error occurred
};

class PIDAutoTune {
    private:
        // Calibration state
        PidCalibrationState state = kCalibrationIdle;
        unsigned long calibrationStartTime = 0;
        unsigned long phaseStartTime = 0;
        
        // Target temperature for calibration
        double targetTemp = 95.0;
        
        // Oscillation detection
        bool heaterState = false;
        double lastTemp = 0;
        bool risingEdge = true;
        
        // Data collection arrays
        static constexpr int MAX_PEAKS = 10;
        double peakTemps[MAX_PEAKS];
        double valleyTemps[MAX_PEAKS];
        unsigned long peakTimes[MAX_PEAKS];
        unsigned long valleyTimes[MAX_PEAKS];
        int peakCount = 0;
        int valleyCount = 0;
        
        // Results
        double ultimateGain = 0;      // Ku - ultimate gain
        double ultimatePeriod = 0;    // Pu - ultimate period (seconds)
        double oscillationAmplitude = 0;
        
        // Calculated PID values
        double calculatedKp = 0;
        double calculatedTn = 0;
        double calculatedTv = 0;
        
        // Progress tracking
        int progressPercent = 0;
        String statusMessage = "Idle";
        
        // Timing constants (milliseconds)
        static constexpr unsigned long WARMUP_TIME = 120000;      // 2 minutes warmup
        static constexpr unsigned long OSCILLATION_TIME = 300000; // 5 minutes oscillation
        static constexpr unsigned long COOLDOWN_TIME = 30000;     // 30 seconds cooldown
        static constexpr unsigned long TOTAL_TIME = WARMUP_TIME + OSCILLATION_TIME + COOLDOWN_TIME;
        
        // Temperature thresholds
        static constexpr double TEMP_DEADBAND = 0.5;  // Dead band around setpoint
        static constexpr double MIN_OSCILLATION = 0.5; // Minimum oscillation amplitude
        
    public:
        /**
         * @brief Start the auto-calibration process
         * @param target Target temperature for calibration
         * @return true if calibration started successfully
         */
        bool start(double target) {
            if (state != kCalibrationIdle && state != kCalibrationComplete && state != kCalibrationError) {
                LOG(WARNING, "Calibration already in progress");
                return false;
            }
            
            // Reset all data
            state = kCalibrationWarmup;
            calibrationStartTime = millis();
            phaseStartTime = millis();
            targetTemp = target;
            
            peakCount = 0;
            valleyCount = 0;
            heaterState = false;
            lastTemp = 0;
            risingEdge = true;
            
            ultimateGain = 0;
            ultimatePeriod = 0;
            oscillationAmplitude = 0;
            
            calculatedKp = 0;
            calculatedTn = 0;
            calculatedTv = 0;
            
            progressPercent = 0;
            statusMessage = "Starting calibration...";
            
            LOGF(INFO, "PID Auto-calibration started, target: %.1f°C", targetTemp);
            
            return true;
        }
        
        /**
         * @brief Stop the calibration process
         */
        void stop() {
            if (state == kCalibrationIdle) {
                return;
            }
            
            LOG(INFO, "PID Auto-calibration stopped by user");
            state = kCalibrationIdle;
            statusMessage = "Stopped by user";
            progressPercent = 0;
        }
        
        /**
         * @brief Update the calibration process (call from main loop)
         * @param currentTemp Current temperature reading
         * @param currentSetpoint Current setpoint (will be overridden during calibration)
         * @param pidOutput Reference to PID output variable
         * @return true if calibration is actively controlling, false otherwise
         */
        bool update(double currentTemp, double& overrideSetpoint, double& pidOutput) {
            if (state == kCalibrationIdle) {
                return false;
            }
            
            unsigned long now = millis();
            unsigned long elapsed = now - calibrationStartTime;
            unsigned long phaseElapsed = now - phaseStartTime;
            
            // Update progress
            progressPercent = constrain((elapsed * 100) / TOTAL_TIME, 0, 100);
            
            switch (state) {
                case kCalibrationWarmup:
                    return handleWarmup(currentTemp, overrideSetpoint, pidOutput, phaseElapsed);
                    
                case kCalibrationOscillation:
                    return handleOscillation(currentTemp, overrideSetpoint, pidOutput, phaseElapsed);
                    
                case kCalibrationCooldown:
                    return handleCooldown(currentTemp, overrideSetpoint, pidOutput, phaseElapsed);
                    
                case kCalibrationComplete:
                case kCalibrationError:
                    return false;
                    
                default:
                    return false;
            }
        }
        
        /**
         * @brief Check if calibration is active
         */
        bool isActive() const {
            return (state != kCalibrationIdle && state != kCalibrationComplete && state != kCalibrationError);
        }
        
        /**
         * @brief Get current calibration state
         */
        PidCalibrationState getState() const {
            return state;
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
         * @brief Get calculated PID parameters
         */
        bool getResults(double& kp, double& tn, double& tv) const {
            if (state != kCalibrationComplete) {
                return false;
            }
            
            kp = calculatedKp;
            tn = calculatedTn;
            tv = calculatedTv;
            
            return true;
        }
        
    private:
        /**
         * @brief Handle warmup phase
         */
        bool handleWarmup(double currentTemp, double& overrideSetpoint, double& pidOutput, unsigned long phaseElapsed) {
            overrideSetpoint = targetTemp;
            statusMessage = "Warming up to target temperature...";
            
            // Check if we've reached close to target or time is up
            if (abs(currentTemp - targetTemp) < 2.0 || phaseElapsed >= WARMUP_TIME) {
                state = kCalibrationOscillation;
                phaseStartTime = millis();
                lastTemp = currentTemp;
                LOGF(INFO, "Warmup complete, starting oscillation phase at %.1f°C", currentTemp);
            }
            
            // Let normal PID handle the warmup
            return true;
        }
        
        /**
         * @brief Handle oscillation phase (relay feedback test)
         */
        bool handleOscillation(double currentTemp, double& overrideSetpoint, double& pidOutput, unsigned long phaseElapsed) {
            overrideSetpoint = targetTemp;
            statusMessage = "Analyzing temperature oscillations...";
            
            // Relay feedback logic
            if (currentTemp < targetTemp - TEMP_DEADBAND) {
                heaterState = true;
                pidOutput = 1000; // Full power
            } else if (currentTemp > targetTemp + TEMP_DEADBAND) {
                heaterState = false;
                pidOutput = 0; // Off
            }
            // Keep current state in deadband
            
            // Detect peaks and valleys
            detectExtremes(currentTemp);
            
            lastTemp = currentTemp;
            
            // Check if we have enough data or time is up
            if (phaseElapsed >= OSCILLATION_TIME || (peakCount >= 5 && valleyCount >= 5)) {
                if (calculateResults()) {
                    state = kCalibrationCooldown;
                    phaseStartTime = millis();
                    LOGF(INFO, "Oscillation complete, Ku=%.2f, Pu=%.2f s", ultimateGain, ultimatePeriod);
                } else {
                    state = kCalibrationError;
                    statusMessage = "Calibration failed: insufficient oscillation data";
                    LOG(ERROR, "Failed to calculate PID parameters");
                }
            }
            
            return true;
        }
        
        /**
         * @brief Handle cooldown phase
         */
        bool handleCooldown(double currentTemp, double& overrideSetpoint, double& pidOutput, unsigned long phaseElapsed) {
            overrideSetpoint = targetTemp;
            statusMessage = "Calibration complete, cooling down...";
            
            if (phaseElapsed >= COOLDOWN_TIME) {
                state = kCalibrationComplete;
                progressPercent = 100;
                statusMessage = String("Complete! Kp=") + String(calculatedKp, 1) + 
                              ", Tn=" + String(calculatedTn, 1) + 
                              ", Tv=" + String(calculatedTv, 1);
                LOGF(INFO, "PID Auto-calibration complete: Kp=%.1f, Tn=%.1f, Tv=%.1f", 
                     calculatedKp, calculatedTn, calculatedTv);
            }
            
            // Let normal PID handle cooldown
            return true;
        }
        
        /**
         * @brief Detect peaks and valleys in temperature
         * A peak occurs when temperature stops rising and starts falling
         * A valley occurs when temperature stops falling and starts rising
         */
        void detectExtremes(double currentTemp) {
            // Detect direction changes with hysteresis
            if (risingEdge) {
                // We're rising - check if we've started falling (peak detected)
                if (currentTemp < lastTemp - MIN_OSCILLATION / 2) {
                    // Temperature is now falling - we passed a peak at lastTemp
                    if (peakCount < MAX_PEAKS) {
                        peakTemps[peakCount] = lastTemp;
                        peakTimes[peakCount] = millis();
                        peakCount++;
                        LOGF(DEBUG, "Peak %d detected: %.2f°C", peakCount, lastTemp);
                    }
                    risingEdge = false;
                }
            } 
            else {
                // We're falling - check if we've started rising (valley detected)
                if (currentTemp > lastTemp + MIN_OSCILLATION / 2) {
                    // Temperature is now rising - we passed a valley at lastTemp
                    if (valleyCount < MAX_PEAKS) {
                        valleyTemps[valleyCount] = lastTemp;
                        valleyTimes[valleyCount] = millis();
                        valleyCount++;
                        LOGF(DEBUG, "Valley %d detected: %.2f°C", valleyCount, lastTemp);
                    }
                    risingEdge = true;
                }
            }
        }
        
        /**
         * @brief Calculate PID parameters from collected data
         */
        bool calculateResults() {
            // Need at least 3 complete cycles for reliable results
            if (peakCount < 3 || valleyCount < 3) {
                LOG(WARNING, "Insufficient oscillation data for calibration");
                return false;
            }
            
            // Calculate average period (time between peaks)
            double totalPeriod = 0;
            int periodCount = 0;
            for (int i = 1; i < peakCount; i++) {
                double period = (peakTimes[i] - peakTimes[i-1]) / 1000.0; // Convert to seconds
                if (period > 5 && period < 300) { // Sanity check: 5s to 5min
                    totalPeriod += period;
                    periodCount++;
                }
            }
            
            if (periodCount == 0) {
                LOG(WARNING, "No valid periods detected");
                return false;
            }
            
            ultimatePeriod = totalPeriod / periodCount;
            
            // Calculate average amplitude (peak to valley)
            double totalAmplitude = 0;
            int ampCount = 0;
            int minCount = min(peakCount, valleyCount);
            for (int i = 0; i < minCount; i++) {
                double amplitude = abs(peakTemps[i] - valleyTemps[i]);
                if (amplitude > MIN_OSCILLATION) {
                    totalAmplitude += amplitude;
                    ampCount++;
                }
            }
            
            if (ampCount == 0) {
                LOG(WARNING, "No valid amplitudes detected");
                return false;
            }
            
            oscillationAmplitude = totalAmplitude / ampCount;
            
            // Calculate ultimate gain (Ku)
            // For relay feedback: Ku = 4d / (π * a)
            // where d = relay amplitude (assume 50% duty cycle ≈ 0.5 * max power)
            // and a = oscillation amplitude
            const double relayAmplitude = 0.5; // Normalized relay output
            ultimateGain = (4.0 * relayAmplitude) / (PI * oscillationAmplitude);
            
            // Calculate PID parameters using modified Ziegler-Nichols
            // Standard ZN gives aggressive tuning, we use conservative multipliers
            // for espresso machines to minimize overshoot and ensure stability
            
            // Classic PID (modified for stability):
            // Kp = 0.45 * Ku (instead of 0.6 * Ku - more conservative control)
            // Ti = Pu / 1.5 (instead of Pu / 2 - slower integration)
            // Td = Pu / 8 (standard derivative time)
            
            calculatedKp = 0.45 * ultimateGain;
            double Ti = ultimatePeriod / 1.5;  // Integral time constant
            double Td = ultimatePeriod / 8.0;  // Derivative time constant
            
            // Convert to Tn/Tv form used by this system
            // Ki = Kp / Tn  =>  Tn = Kp / Ki
            // Kd = Tv * Kp  =>  Tv = Kd / Kp
            // Ti = Tn, Td = Tv in many systems
            
            calculatedTn = Ti;
            calculatedTv = Td;
            
            // Apply sanity checks and constraints
            calculatedKp = constrain(calculatedKp, 10.0, 200.0);
            calculatedTn = constrain(calculatedTn, 10.0, 200.0);
            calculatedTv = constrain(calculatedTv, 5.0, 50.0);
            
            LOGF(INFO, "Calculated results: Ku=%.2f, Pu=%.2f s, Amplitude=%.2f°C", 
                 ultimateGain, ultimatePeriod, oscillationAmplitude);
            LOGF(INFO, "PID params: Kp=%.1f, Tn=%.1f, Tv=%.1f", 
                 calculatedKp, calculatedTn, calculatedTv);
            
            return true;
        }
};

// Global instance
inline PIDAutoTune pidAutoTune;
