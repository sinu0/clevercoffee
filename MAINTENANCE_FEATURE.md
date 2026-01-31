# Maintenance Reminders Feature

## Overview

The Maintenance Reminders feature helps users keep their espresso machine in optimal condition by tracking usage and alerting when maintenance tasks are due. This feature tracks four key maintenance types and provides notifications through the OLED display, web interface, and MQTT.

## Features

### 🔧 Tracked Maintenance Types

1. **Descaling** - Every 200 shots (configurable)
2. **Backflushing** - Every 50 shots OR 7 days (configurable)
3. **Filter Basket Cleaning** - Every 50 shots (configurable)
4. **Water Tank Refill** - Every 30 shots (configurable)

### 📊 User Interfaces

#### Web Dashboard
- Access at: `/maintenance.html`
- Features:
  - Real-time progress bars for each maintenance type
  - Shot counters and interval displays
  - One-click "Mark as Done" buttons
  - Visual alerts with red badges when maintenance is due

#### OLED Display
- Rotating maintenance warnings on the standard display template
- Shows only items that are currently due
- Rotates every 5 seconds
- Examples: "! DESCALE DUE", "! BACKFLUSH DUE"

#### MQTT Sensors
The following MQTT sensors are published for home automation integration:
- `maintenance_descale_due` - Binary (1.0 = due, 0.0 = not due)
- `maintenance_descale_shots` - Shot count since last descale
- `maintenance_backflush_due` - Binary (1.0 = due, 0.0 = not due)
- `maintenance_backflush_shots` - Shot count since last backflush
- `maintenance_backflush_days` - Days since last backflush
- `maintenance_basket_due` - Binary (1.0 = due, 0.0 = not due)
- `maintenance_refill_due` - Binary (1.0 = due, 0.0 = not due)

## Technical Implementation

### Data Persistence
- Uses ESP32 Preferences API for persistent storage
- All counters and settings survive reboots
- Namespace: "maintenance"

### Counter Logic

#### Shot-Based Counters
All maintenance types track shots:
- Counter increments automatically after each brew completes (`kBrewFinished` state)
- Integrated into `brewHandler.h`

#### Time-Based Counters
Backflushing also tracks days:
- Daily check runs every 24 hours in the main loop
- Uses non-blocking millis() timing
- Handles millis() overflow correctly with unsigned arithmetic

### API Endpoints

#### GET /maintenance
Returns JSON with all maintenance status:
```json
{
  "descale": {
    "shots": 150,
    "interval": 200,
    "due": false,
    "progress": 75
  },
  "backflush": {
    "shots": 45,
    "days": 5,
    "shotsInterval": 50,
    "daysInterval": 7,
    "due": false,
    "progress": 90
  },
  // ... basket and refill similar
}
```

#### POST /maintenance/reset
Resets a specific maintenance counter.

**Parameters:**
- `type` (form data) - One of: "descale", "backflush", "basket", "refill"

**Response:**
- 200 OK - "Counter reset"
- 400 Bad Request - "Invalid type" or "Missing type"

### File Structure

**Core Implementation:**
- `src/maintenance.h` - Data structures and function declarations
- `src/maintenance.cpp` - Implementation with Preferences API

**Integration Points:**
- `src/brewHandler.h` - Shot counter increment
- `src/main.cpp` - Initialization, daily checks, MQTT sensors
- `src/embeddedWebserver.h` - REST API endpoints
- `src/display/displayCommon.h` - Display warning function
- `src/display/displayTemplateStandard.h` - Display integration

**Frontend:**
- `frontend/html/maintenance.html` - Vue.js web interface
- `frontend/html_fragments/header.html` - Navigation menu

## Configuration

### Default Intervals
```cpp
descaleInterval = 200 shots
backflushInterval = 50 shots
backflushDaysInterval = 7 days
basketCleanInterval = 50 shots
refillReminderInterval = 30 shots
```

### Customizing Intervals
Intervals are stored in ESP32 Preferences and can be modified by:
1. Editing the default values in `maintenance.cpp` (`loadMaintenance()` function)
2. Future enhancement: Web UI settings page for interval configuration

## Usage

### For End Users

1. **Monitor Maintenance Status:**
   - Navigate to "Maintenance" in the web interface
   - Check progress bars and shot counts
   - OLED will display warnings when items are due

2. **Mark Maintenance as Complete:**
   - Open the Maintenance page
   - Click "Mark as Done" button for the completed task
   - Counter resets to zero

3. **Home Automation Integration:**
   - Use MQTT sensors to trigger automations
   - Example: Send notification when `maintenance_descale_due` becomes 1.0

### For Developers

#### Adding Shot Counters
The `incrementShotCounters()` function is called in `brewHandler.h` when a brew finishes:
```cpp
case kBrewFinished:
    // ... existing code ...
    incrementShotCounters();
    break;
```

#### Adding Daily Checks
The `checkDailyMaintenance()` function runs in the main loop:
```cpp
static unsigned long lastMaintenanceCheck = 0;
if (millis() - lastMaintenanceCheck > MILLISECONDS_PER_DAY) {
    checkDailyMaintenance();
    lastMaintenanceCheck = millis();
}
```

#### Extending Maintenance Types
To add a new maintenance type:
1. Add fields to `MaintenanceTracker` struct in `maintenance.h`
2. Add load/save logic in `loadMaintenance()` and `saveMaintenance()`
3. Add check logic in `checkMaintenanceDue()`
4. Create reset function (e.g., `resetNewTypeCounter()`)
5. Update web API endpoint to include new type
6. Update frontend HTML to display new type
7. Update display rotation logic in `displayMaintenanceWarning()`
8. Add MQTT sensors in `main.cpp`

## Safety Features

- **Division by Zero Protection:** All progress calculations check for zero intervals
- **Overflow Handling:** Uses unsigned long arithmetic for millis() to handle overflow
- **Smart Display Rotation:** Only shows maintenance items that are actually due
- **Non-Blocking:** All checks are non-blocking and run in the main loop

## Testing

### Manual Testing Checklist
- [ ] Shot counters increment after brew
- [ ] Maintenance warnings appear on OLED when due
- [ ] Web UI shows correct progress bars
- [ ] "Mark as Done" buttons reset counters
- [ ] Counters persist across reboots
- [ ] MQTT sensors publish correctly
- [ ] Display only shows due items (not blank warnings)
- [ ] Daily check increments day counter

### Simulating Maintenance Due
For testing, you can manually set low intervals in Preferences or increment counters multiple times by brewing repeatedly.

## Future Enhancements

Potential improvements for future versions:
1. **Configurable Intervals:** Web UI to adjust maintenance intervals
2. **Last Serviced Display:** Show "Last descaled: 3 days ago" using timestamp fields
3. **Maintenance History:** Log of completed maintenance tasks
4. **Email/Push Notifications:** Alert users when maintenance is due
5. **Predictive Maintenance:** Estimate when next maintenance will be due
6. **Water Sensor Integration:** Automatic refill detection
7. **Maintenance Calendar:** Visual calendar of upcoming maintenance

## Troubleshooting

### Counters Not Incrementing
- Check that brew completes successfully (reaches `kBrewFinished` state)
- Verify `incrementShotCounters()` is called in `brewHandler.h`

### OLED Not Showing Warnings
- Ensure `displayMaintenanceWarning()` is called in display template
- Check that maintenance is actually due (counters >= intervals)
- Verify not in active brew state (warnings don't show during brew)

### Counters Reset on Reboot
- Check ESP32 Preferences API is working
- Verify `saveMaintenance()` is called after counter changes
- Check for flash memory issues

### Web UI Not Loading
- Ensure `maintenance.html` is uploaded to LittleFS
- Check web server endpoints are registered in `embeddedWebserver.h`
- Verify network connectivity

## License

This feature is part of the CleverCoffee project and follows the same license.

## Credits

Feature implementation based on the specification in GitHub issue #[issue_number].
Developed by: GitHub Copilot
