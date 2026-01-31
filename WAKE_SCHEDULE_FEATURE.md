# Scheduled Wake-Up Timer Feature

## Overview

The Scheduled Wake-Up Timer automatically powers on the espresso machine or exits standby mode at scheduled times, ensuring your coffee machine is ready when you need it.

## Features

- ⏰ **Up to 3 Wake Schedules**: Configure multiple wake times for different routines
- 📅 **Day Selection**: Choose specific days of the week (Mon-Fri, weekends, custom)
- 🌍 **Timezone Support**: Configure your local timezone (UTC-12 to UTC+14)
- 🔄 **NTP Synchronization**: Automatic time sync with internet time servers
- 💾 **Persistent Storage**: Schedules survive power cycles and reboots
- 🌐 **Web Interface**: Easy-to-use Vue.js-based configuration page
- 📡 **MQTT Integration**: Publish schedule status to Home Assistant/MQTT brokers
- 🌙 **Auto Standby** (Optional): Automatically enable standby at night (10 PM - 5 AM)

## Usage

### Web Interface

1. Connect to your CleverCoffee web interface
2. Navigate to **Schedule** in the menu (clock icon)
3. Configure your timezone
4. Enable a schedule slot
5. Set the wake time and select days of the week
6. Save changes

### Configuration

#### Timezone
Select your timezone from the dropdown. The system uses UTC offsets from -12 to +14 hours.

Common timezones:
- UTC-8 (PST - Pacific Standard Time)
- UTC-5 (EST - Eastern Standard Time)
- UTC+0 (GMT - Greenwich Mean Time)
- UTC+1 (CET - Central European Time)
- UTC+2 (EET - Eastern European Time)
- UTC+8 (CST - China Standard Time)
- UTC+9 (JST - Japan Standard Time)

#### Wake Schedules

Each schedule includes:
- **Enabled**: Toggle to activate/deactivate the schedule
- **Wake Time**: 24-hour format (HH:MM)
- **Days of Week**: Select any combination of days
  - Mon, Tue, Wed, Thu, Fri, Sat, Sun
- **Name**: Descriptive label (e.g., "Weekdays", "Saturday Morning")

### MQTT Integration

The following sensors are published to MQTT:

- `scheduleNextWake`: Unix timestamp of next scheduled wake time (0 if no schedule)
- `scheduleNtpSynced`: NTP synchronization status (1 = synced, 0 = not synced)

Example Home Assistant configuration:

```yaml
sensor:
  - platform: mqtt
    name: "Coffee Machine Next Wake"
    state_topic: "clevercoffee/silvia/scheduleNextWake"
    device_class: timestamp
    
  - platform: mqtt
    name: "Coffee Machine NTP Synced"
    state_topic: "clevercoffee/silvia/scheduleNtpSynced"
    value_template: "{{ 'Yes' if value == '1' else 'No' }}"
```

## Technical Details

### Architecture

#### NTP Synchronization
- Uses ESP32's built-in NTP client
- Connects to: pool.ntp.org, time.nist.gov, time.google.com
- 10-second timeout for initial sync
- Timezone is applied via `configTime()` and `setenv("TZ")`

#### Data Storage
- Uses ESP32 Preferences library (NVS - Non-Volatile Storage)
- Namespace: "schedule"
- Stores:
  - Up to 3 WakeSchedule structures
  - Timezone offset (int8_t)

#### Wake Check Logic
- Runs every 60 seconds in main loop
- Compares current time (from RTC) with enabled schedules
- Converts Sunday=0 weekday format to Monday=0 for bitmap matching
- Triggers wake by setting `standbyModeOn = false`

#### Schedule Structure
```cpp
struct WakeSchedule {
    bool enabled;           // Schedule active/inactive
    uint8_t hour;          // 0-23
    uint8_t minute;        // 0-59
    uint8_t daysOfWeek;    // Bitmap: bit 0=Mon, ..., 6=Sun
    char name[16];         // Schedule name
};
```

#### Days of Week Bitmap
- Bit 0 = Monday
- Bit 1 = Tuesday
- Bit 2 = Wednesday
- Bit 3 = Thursday
- Bit 4 = Friday
- Bit 5 = Saturday
- Bit 6 = Sunday

Example: `0x1F` (binary: 0001 1111) = Monday through Friday

### API Endpoints

#### GET /schedule
Returns all schedules, timezone, and NTP status.

**Response:**
```json
{
  "ntpSynced": true,
  "timezone": 1,
  "nextWake": 1706774400,
  "schedules": [
    {
      "enabled": true,
      "hour": 7,
      "minute": 0,
      "days": 31,
      "name": "Weekdays"
    },
    ...
  ]
}
```

#### POST /schedule/update
Update a specific schedule slot.

**Parameters:**
- `index` (required): Schedule slot (0-2)
- `enabled` (optional): "true" or "false"
- `hour` (optional): 0-23
- `minute` (optional): 0-59
- `days` (optional): Bitmap value (0-127)
- `name` (optional): Schedule name

#### POST /schedule/timezone
Update timezone and re-sync NTP.

**Parameters:**
- `offset` (required): UTC offset in hours (-12 to +14)

## Requirements

- WiFi connection (for NTP sync)
- Internet access (for time synchronization)
- Standby mode enabled

## Troubleshooting

### NTP Sync Failed
- Check WiFi connection
- Verify internet access
- Try changing timezone and saving again (triggers re-sync)
- Check firewall settings (NTP uses UDP port 123)

### Schedule Not Triggering
- Verify NTP sync status is "Synced"
- Check schedule is enabled (checkbox)
- Verify correct days are selected
- Ensure standby mode is active
- Check timezone is correct

### Next Wake Shows "No schedule"
- Enable at least one schedule
- Select at least one day of the week
- Verify time is set

## Default Configuration

On first boot, the system creates a default schedule:
- **Name**: "Weekdays"
- **Time**: 7:00 AM
- **Days**: Monday - Friday
- **Enabled**: No (must be manually enabled)
- **Timezone**: UTC+0

## Security Notes

- Web interface uses same authentication as main CleverCoffee UI
- Schedules are stored in ESP32 NVS (encrypted if ESP32 flash encryption is enabled)
- MQTT data is transmitted per your MQTT broker configuration

## Future Enhancements

Potential improvements for future versions:
- Multiple wake times per day
- Seasonal schedule adjustments
- Holiday/vacation mode
- Pre-heat duration configuration
- Smart wake based on historical usage

## Credits

Implemented as part of CleverCoffee firmware enhancement project.
