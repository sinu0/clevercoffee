# PID Auto-Calibration Guide

## Overview

The PID Auto-Calibration feature automatically determines optimal PID parameters for your espresso machine by analyzing the thermal characteristics of your specific boiler and heating system. This eliminates the tedious manual tuning process and provides professional-grade PID tuning results.

## How It Works

The auto-calibration uses the **Relay Feedback Test (RFT)** method, a well-established control engineering technique particularly suited for espresso machines where temperature sensors may not be directly at the heating element.

### The Process

1. **Warmup Phase (≈2 minutes)**
   - The machine heats up to your target calibration temperature
   - Ensures starting from a stable baseline

2. **Oscillation Phase (≈5 minutes)**
   - The heater relay switches on/off around the setpoint
   - Temperature oscillations are measured and analyzed
   - Multiple peaks and valleys are recorded

3. **Analysis**
   - The system calculates:
     - **Ultimate Gain (Ku)**: Maximum stable gain before oscillation
     - **Ultimate Period (Pu)**: Period of sustained oscillations
     - **Oscillation Amplitude**: Magnitude of temperature swings
   
4. **PID Calculation**
   - Uses modified Ziegler-Nichols tuning rules
   - Optimized for espresso machines to minimize overshoot
   - Conservative tuning for stability and precision

5. **Cooldown (≈30 seconds)**
   - Brief stabilization period before completion

**Total Time**: Approximately 7-8 minutes

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
