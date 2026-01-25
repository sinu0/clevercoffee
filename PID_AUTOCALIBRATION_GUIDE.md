# PID Auto-Calibration Guide

## Overview

The PID Auto-Calibration feature uses the proven **br3ttb/Arduino-PID-AutoTune-Library** by Brett Beauregard to automatically determine optimal PID parameters for your espresso machine. This library has been tested and used in thousands of Arduino projects worldwide and implements the industry-standard relay feedback method.

## How It Works

The library uses the **Relay Feedback Method** (also known as the relay oscillation method), a classic and well-established technique for auto-tuning PID controllers.

### The Process

1. **Setup Phase**
   - The library takes over control of the heater output
   - Sets the target temperature for calibration

2. **Relay Feedback Phase (8-12 minutes for espresso machines)**
   - The output is switched between high and low values (relay mode)
   - The system observes how the temperature responds to these changes
   - Multiple oscillation cycles are analyzed
   - Takes longer for espresso machines due to boiler thermal mass and sensor lag

3. **Parameter Calculation**
   - The library automatically calculates the ultimate gain (Ku) and period (Pu)
   - Uses these values with proven tuning rules (Ziegler-Nichols, etc.)
   - Generates optimal Kp, Ki, and Kd values

4. **Results**
   - Parameters are converted to Tn/Tv form used by this system
   - You review and apply the results

**Total Time**: Typically 8-12 minutes for espresso machine boilers (slower than small systems due to thermal mass)

## Library Information

- **Library**: br3ttb/Arduino-PID-AutoTune-Library
- **Author**: Brett Beauregard  
- **Repository**: https://github.com/br3ttb/Arduino-PID-AutoTune-Library
- **Compatibility**: Works with PID_v1 library (already used in this project)
- **Proven**: Used in thousands of projects worldwide
- **Method**: Standard relay feedback / oscillation method

## Why This Library?

✅ **Proven and tested** in real-world applications
✅ **Optimized** - completes in 8-12 minutes for espresso machines  
✅ **Reliable** - implements standard control engineering methods
✅ **Well-documented** - extensive community support
✅ **Compatible** - works perfectly with existing PID_v1 library
✅ **Professional** - based on academic research and industrial practice

## When to Use Auto-Calibration

### Recommended Scenarios:
- ✅ First-time setup of a new machine
- ✅ After changing the boiler or heating element
- ✅ When temperature sensor location is changed
- ✅ If you're experiencing temperature instability
- ✅ After significant modifications to the machine
- ✅ When you want to optimize existing PID values

### Not Recommended:
- ❌ During regular brewing operations
- ❌ When the machine is already hot (cool down first for best results)
- ❌ If you have hardware issues (fix those first)

## Step-by-Step Instructions

### Preparation

1. **Ensure Machine is Cool**
   - Best results when starting from room temperature
   - Or wait at least 15-20 minutes after last use

2. **Check Water Level**
   - Ensure water tank is full
   - Avoid running dry during calibration

3. **Clear the Area**
   - Don't interact with the machine during calibration
   - Avoid opening the steam wand or brew group

### Running the Calibration

1. **Access Web Interface**
   - Connect to your machine's web interface
   - Navigate to **System** page

2. **Configure Calibration Temperature**
   - Set the "Calibration Temperature (°C)" field
   - Use your normal brewing temperature (typically 93-98°C)
   - This is stored in the `pid.calibration.target_temp` parameter

3. **Start Auto-Tune**
   - Click "Start Auto-Tune" button
   - Confirm the dialog prompt

4. **Monitor Progress**
   - Progress bar shows completion percentage
   - Status message shows "Auto-tuning in progress..."
   - Typically completes in 8-12 minutes for espresso machine boilers
   - Be patient - the thermal mass requires multiple oscillation cycles

5. **Review Results**
   - When complete, recommended PID values are displayed in Tn/Tv form:
     - **Kp**: Proportional gain
     - **Tn**: Integral time constant (Kp/Ki)
     - **Tv**: Derivative time constant (Kd/Kp)

6. **Apply or Dismiss**
   - Click "Apply These Values" to use the new parameters
   - Or "Dismiss" if you want to keep current settings

### If You Need to Stop

- Click "Stop Auto-Tune" at any time
- The machine will return to normal PID control
- No parameters will be changed

## Understanding the Results

### What the Parameters Mean

| Parameter | Meaning | Effect |
|-----------|---------|--------|
| **Kp** | Proportional Gain | Higher = more aggressive response to temperature error |
| **Tn** | Integral Time (Kp/Ki) | Lower = faster correction of persistent errors |
| **Tv** | Derivative Time (Kd/Kp) | Higher = more damping of rapid changes |

### Typical Values

For most espresso machines:
- **Kp**: 40-80 (50-70 common)
- **Tn**: 30-70 (40-60 common)  
- **Tv**: 8-20 (10-15 common)

Values outside these ranges may indicate:
- Unusual machine characteristics (normal for some machines)
- Need to repeat calibration from cooler starting temperature
- Potential hardware issues worth investigating

### Evaluating Performance

After applying calibrated values, observe:

✅ **Good Performance**:
- Reaches setpoint in 5-10 minutes
- Minimal overshoot (< 1°C)
- Stable temperature (±0.1-0.3°C)
- Quick recovery after brew

⚠️ **May Need Adjustment**:
- Excessive overshoot (> 2°C)
- Very slow approach to setpoint (> 15 min)
- Large oscillations (> 1°C amplitude)

## Troubleshooting

### "Auto-tune already in progress"

**Solution:**
- Wait for current tuning to complete
- Or stop the current tuning first

### Results seem too aggressive/conservative

The library uses standard tuning rules that work for most systems. If needed:

**If temperature overshoots significantly:**
- Reduce Kp by 10-20%
- Slightly increase Tv

**If temperature approaches too slowly:**
- Increase Kp by 10-20%
- Slightly decrease Tn

### Want to re-tune

- Simply run the auto-tune again
- The library will generate fresh parameters
- You can run it multiple times to verify consistency

## Advanced Topics

### Library Configuration

The integration allows configuring:
- **Output Step**: Magnitude of relay output changes (default: 50% or 500/1000)
- **Noise Band**: Temperature variation tolerance (default: 0.5°C)
- **Lookback Time**: Analysis window (default: 20 seconds)

These are optimized for espresso machines and typically don't need adjustment.

### Manual Tuning After Auto-Tune

You can fine-tune the auto-calibrated values:

1. **For faster heating**: Increase Kp by small increments (5-10)
2. **For less overshoot**: Decrease Kp, increase Tv
3. **For better stability**: Adjust Tn (integral time)

Always make small changes and test thoroughly.

## Configuration Parameter

### `pid.calibration.target_temp`

- **Location**: Parameters page → PID section
- **Type**: Double (°C)
- **Default**: 95.0
- **Range**: 70.0-110.0
- **Description**: Target temperature for auto-calibration

This parameter is automatically used when starting calibration from the System page.

## API Endpoints

For advanced users or automation:

### Start Auto-Tune
```
POST /startPidCalibration
Response: {"success": true/false, "message": "..."}
```

### Stop Auto-Tune
```
POST /stopPidCalibration
Response: {"success": true/false, "message": "..."}
```

### Check Status
```
GET /calibrationStatus
Response: {
  "active": true/false,
  "complete": true/false,
  "progress": 0-100,
  "status": "...",
  "hasResults": true/false,
  "results": {"kp": ..., "tn": ..., "tv": ...}
}
```

### Apply Results
```
POST /applyCalibrationResults
Response: {"success": true/false, "message": "..."}
```

## Best Practices

1. **Run auto-tune after hardware changes**
2. **Use your actual brewing temperature**
3. **Start from a cool machine when possible**
4. **Don't interrupt the process**
5. **Test new values thoroughly before relying on them**
6. **Keep a record of good values** (download config backup)
7. **Re-calibrate if you change sensor position**

## Safety Notes

⚠️ The auto-calibration:
- Operates within normal temperature ranges
- Uses existing safety limits (emergency stop at 145°C)
- Can be stopped at any time
- Does not automatically apply results (you must confirm)

⚠️ Always:
- Monitor the first few heating cycles with new parameters
- Have emergency stop capability
- Verify temperature readings are accurate

## Support

If you encounter issues with auto-calibration:

1. Check the logs for error messages
2. Verify hardware is functioning normally
3. Try running auto-tune again from cold start
4. Consult the library documentation: https://github.com/br3ttb/Arduino-PID-AutoTune-Library
5. Join our [Discord](https://discord.gg/Kq5RFznuU4) for community support

## Technical References

- **Primary Library**: https://github.com/br3ttb/Arduino-PID-AutoTune-Library
- **Blog Post by Author**: http://brettbeauregard.com/blog/2012/01/arduino-pid-autotune-library/
- Ziegler-Nichols Tuning Method
- Relay Feedback Test (Åström & Hägglund)
- PID Control Theory for Thermal Systems

---

**Version**: 4.0.0+
**Library**: br3ttb/Arduino-PID-AutoTune-Library
**Last Updated**: January 2026


## When to Use Auto-Calibration

### Recommended Scenarios:
- ✅ First-time setup of a new machine
- ✅ After changing the boiler or heating element
- ✅ When temperature sensor location is changed
- ✅ If you're experiencing temperature instability
- ✅ After significant modifications to the machine
- ✅ When you want to optimize existing PID values

### Not Recommended:
- ❌ During regular brewing operations
- ❌ When the machine is already hot (cool down first)
- ❌ If you have hardware issues (fix those first)

## Step-by-Step Instructions

### Preparation

1. **Ensure Machine is Cool**
   - Best results when starting from room temperature
   - Or wait at least 15-20 minutes after last use

2. **Check Water Level**
   - Ensure water tank is full
   - Avoid running dry during calibration

3. **Clear the Area**
   - Don't interact with the machine during calibration
   - Avoid opening the steam wand or brew group

### Running the Calibration

1. **Access Web Interface**
   - Connect to your machine's web interface
   - Navigate to **System** page

2. **Configure Calibration Temperature**
   - Set the "Calibration Temperature (°C)" field
   - Use your normal brewing temperature (typically 93-98°C)
   - This is stored in the `pid.calibration.target_temp` parameter

3. **Start Calibration**
   - Click "Start Calibration" button
   - Confirm the dialog prompt

4. **Monitor Progress**
   - Progress bar shows completion percentage
   - Status message indicates current phase:
     - "Warming up to target temperature..."
     - "Analyzing temperature oscillations..."
     - "Calibration complete, cooling down..."

5. **Review Results**
   - When complete, recommended PID values are displayed:
     - **Kp**: Proportional gain
     - **Tn**: Integral time constant
     - **Tv**: Derivative time constant

6. **Apply or Dismiss**
   - Click "Apply These Values" to use the new parameters
   - Or "Dismiss" if you want to keep current settings

### If You Need to Stop

- Click "Stop Calibration" at any time
- The machine will return to normal PID control
- No parameters will be changed

## Understanding the Results

### What the Parameters Mean

| Parameter | Meaning | Effect |
|-----------|---------|--------|
| **Kp** | Proportional Gain | Higher = more aggressive response to temperature error |
| **Tn** | Integral Time (Kp/Ki) | Lower = faster correction of persistent errors |
| **Tv** | Derivative Time (Kd/Kp) | Higher = more damping of rapid changes |

### Typical Values

For most espresso machines:
- **Kp**: 40-80 (50-70 common)
- **Tn**: 30-70 (40-60 common)  
- **Tv**: 8-20 (10-15 common)

Values outside these ranges may indicate:
- Unusual machine characteristics (normal for some machines)
- Need to repeat calibration from cooler starting temperature
- Potential hardware issues worth investigating

### Evaluating Performance

After applying calibrated values, observe:

✅ **Good Performance**:
- Reaches setpoint in 5-10 minutes
- Minimal overshoot (< 1°C)
- Stable temperature (±0.1-0.3°C)
- Quick recovery after brew

⚠️ **May Need Adjustment**:
- Excessive overshoot (> 2°C)
- Very slow approach to setpoint (> 15 min)
- Large oscillations (> 1°C amplitude)

## Troubleshooting

### "Calibration failed: insufficient oscillation data"

**Causes:**
- Temperature sensor placement issue
- System too heavily insulated (no oscillations)
- Starting temperature too close to target

**Solutions:**
- Let machine cool down completely
- Verify sensor is properly attached
- Check that heater is functioning normally

### Results seem too aggressive/conservative

**If temperature overshoots significantly:**
- Reduce Kp by 10-20%
- Slightly increase Tv

**If temperature approaches too slowly:**
- Increase Kp by 10-20%
- Slightly decrease Tn

### Oscillations continue after calibration

**Causes:**
- Parameters outside optimal range
- Mechanical/thermal issues

**Solutions:**
- Re-run calibration from cold start
- Try starting from lower temperature (e.g., 85°C)
- Inspect machine for issues (loose connections, air gaps)

## Advanced Topics

### Why Modified Ziegler-Nichols?

Classic Ziegler-Nichols tuning is designed for industrial processes and tends to be aggressive. Espresso machines benefit from:

- **Lower proportional gain (0.45×Ku vs 0.6×Ku)**: Reduces overshoot
- **Slower integration (Pu/1.5 vs Pu/2)**: More stable long-term
- **Conservative derivative**: Prevents noise amplification

### Calibration Algorithm Details

The implementation uses:

1. **Relay amplitude**: Normalized to 50% duty cycle
2. **Peak/valley detection**: Requires minimum 0.5°C oscillation
3. **Period averaging**: Uses multiple cycles for accuracy
4. **Sanity checks**: Validates periods (5s-300s) and amplitudes

### Manual Tuning After Auto-Calibration

You can fine-tune the auto-calibrated values:

1. **For faster heating**: Increase Kp by small increments (5-10)
2. **For less overshoot**: Decrease Kp, increase Tv
3. **For better stability**: Adjust Tn (integral time)

Always make small changes and test thoroughly.

## Configuration Parameter

### `pid.calibration.target_temp`

- **Location**: Parameters page → PID section
- **Type**: Double (°C)
- **Default**: 95.0
- **Range**: 70.0-110.0
- **Description**: Target temperature for calibration process

This parameter is automatically used when starting calibration from the System page. It's separate from your normal brewing setpoint and brew detection setpoint.

## API Endpoints

For advanced users or automation:

### Start Calibration
```
POST /startPidCalibration
Response: {"success": true/false, "message": "..."}
```

### Stop Calibration
```
POST /stopPidCalibration
Response: {"success": true/false, "message": "..."}
```

### Check Status
```
GET /calibrationStatus
Response: {
  "active": true/false,
  "state": 0-5,
  "progress": 0-100,
  "status": "...",
  "hasResults": true/false,
  "results": {"kp": ..., "tn": ..., "tv": ...}
}
```

### Apply Results
```
POST /applyCalibrationResults
Response: {"success": true/false, "message": "..."}
```

## Best Practices

1. **Run calibration after hardware changes**
2. **Use your actual brewing temperature**
3. **Start from a cold machine when possible**
4. **Don't interrupt the process**
5. **Test new values thoroughly before relying on them**
6. **Keep a record of good values** (download config backup)
7. **Re-calibrate if you change sensor position**

## Safety Notes

⚠️ The auto-calibration:
- Operates within normal temperature ranges
- Uses existing safety limits (emergency stop at 145°C)
- Can be stopped at any time
- Does not automatically apply results (you must confirm)

⚠️ Always:
- Monitor the first few heating cycles with new parameters
- Have emergency stop capability
- Verify temperature readings are accurate

## Support

If you encounter issues with auto-calibration:

1. Check the logs for error messages
2. Verify hardware is functioning normally
3. Try running calibration again from cold start
4. Join our [Discord](https://discord.gg/Kq5RFznuU4) for community support

## Technical References

- Ziegler-Nichols Tuning Method
- Relay Feedback Test (Åström & Hägglund)
- PID Control Theory for Thermal Systems

---

**Version**: 4.0.0+
**Last Updated**: January 2026
