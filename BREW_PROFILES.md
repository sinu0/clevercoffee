# Brew Profiles Feature Documentation

## Overview

The Brew Profiles feature allows users to save and quickly switch between different brewing configurations optimized for different coffee beans. This eliminates the need to manually adjust parameters when switching between different types of coffee.

## Features

- **5 Profile Slots**: Store up to 5 independent brew profiles
- **Comprehensive Settings**: Each profile stores temperature, pre-infusion, brew mode, and PID parameters
- **One-Click Loading**: Quickly switch between profiles with a single click
- **Web UI Management**: User-friendly interface for creating, editing, and deleting profiles
- **Persistent Storage**: Profiles are stored in ESP32 flash memory and survive reboots
- **Active Profile Indicator**: Visual feedback showing which profile is currently active
- **MQTT Integration**: Load profiles via MQTT commands for home automation integration

## What's Stored in Each Profile

Each brew profile captures and stores the following settings:

### Temperature Settings
- **Brew Setpoint**: Target brewing temperature (°C)
- **Steam Setpoint**: Target steam temperature (°C)
- **Temperature Offset**: Brew temperature offset adjustment

### Pre-Infusion Settings
- **Pre-Infusion Enabled**: Whether pre-infusion is active
- **Pre-Infusion Time**: Duration of pre-infusion phase (seconds)
- **Pre-Infusion Pause**: Pause time after pre-infusion (seconds)

### Brew Control Settings
- **Brew Mode**: Manual (0) or Automatic (1)
- **Brew by Time Enabled**: Automatic stopping by time
- **Target Brew Time**: Target brewing duration (seconds)
- **Brew by Weight Enabled**: Automatic stopping by weight
- **Target Brew Weight**: Target shot weight (grams)

### PID Settings (Optional)
- **Custom PID**: Flag to use profile-specific PID values
- **PID Kp, Tn, Tv**: PID controller parameters

## Using the Web Interface

### Accessing Profiles

1. Navigate to the CleverCoffee web interface
2. Click on "Profiles" in the navigation menu
3. You'll see 5 profile cards representing available slots

### Saving a Profile

1. Configure your machine with desired settings (temperature, pre-infusion, etc.)
2. Click the "Save Current Settings" button
3. Enter a descriptive name (e.g., "Light Roast", "Dark Espresso")
4. Select which slot to save to (1-5)
5. Click "Save"

### Loading a Profile

Simply click on any active profile card, or click the "Load Profile" button on a specific profile.

### Renaming a Profile

1. Click the three-dot menu (⋮) on a profile card
2. Select "Rename"
3. Enter the new name
4. Confirm

### Deleting a Profile

1. Click the three-dot menu (⋮) on a profile card
2. Select "Delete"
3. Confirm the deletion

## Using MQTT

### Loading a Profile via MQTT

Topic: `{mqtt_topic_prefix}{hostname}/profileLoad/set`  
Payload: Profile index (0-4)

Example:
```
Topic: custom/kitchen/silvia/profileLoad/set
Payload: 2
```

This will load profile at index 2 (the 3rd profile slot).

### Monitoring Active Profile

Topic: `{mqtt_topic_prefix}{hostname}/profileActive`

This sensor publishes the currently active profile index (0-4, or 255 if no profile is active).

## Example Use Cases

### Profile 1: Light Roast
- Temperature: 94°C
- Pre-infusion: 2s on, 5s pause
- Mode: Automatic
- Brew time: 25 seconds

### Profile 2: Dark Espresso
- Temperature: 92°C
- Pre-infusion: 3s on, 3s pause
- Mode: Automatic
- Brew time: 20 seconds

### Profile 3: Decaf
- Temperature: 96°C
- Pre-infusion: Disabled
- Mode: Automatic
- Brew time: 30 seconds

### Profile 4: Lungo
- Temperature: 93°C
- Pre-infusion: 2s on, 4s pause
- Mode: Manual

### Profile 5: Experimental
- Temperature: 95°C
- Pre-infusion: 4s on, 6s pause
- Mode: Automatic
- Brew by weight: 40g

## Home Assistant Integration

Example MQTT sensor configuration for Home Assistant:

```yaml
sensor:
  - platform: mqtt
    name: "Espresso Active Profile"
    state_topic: "custom/kitchen/silvia/profileActive"
    
number:
  - platform: mqtt
    name: "Espresso Load Profile"
    command_topic: "custom/kitchen/silvia/profileLoad/set"
    min: 0
    max: 4
    step: 1
```

## Technical Details

### Storage Implementation

Profiles are stored using the ESP32 Preferences library in the `profiles` namespace. The data is persisted in NVS (Non-Volatile Storage) flash memory.

### Data Structure

```cpp
struct BrewProfile {
    char name[20];              // Profile name
    bool active;                // Is this slot used?
    double brewSetpoint;        // Brew temperature
    double steamSetpoint;       // Steam temperature
    double brewTempOffset;      // Temperature offset
    bool preInfusionEnabled;    // Pre-infusion on/off
    double preInfusionTime;     // Pre-infusion duration
    double preInfusionPause;    // Pre-infusion pause
    int brewMode;               // Manual/Automatic
    bool brewByTimeEnabled;     // Brew by time on/off
    double targetBrewTime;      // Target time
    bool brewByWeightEnabled;   // Brew by weight on/off
    double targetBrewWeight;    // Target weight
    bool customPID;             // Use custom PID
    double pidKp, pidTn, pidTv; // PID parameters
};
```

### Memory Usage

Each profile uses approximately 100 bytes of flash memory. With 5 profiles, total storage is around 500 bytes.

## API Reference

### REST Endpoints

#### GET /profiles
Returns list of all profiles with their details.

**Response:**
```json
{
  "profiles": [
    {
      "index": 0,
      "name": "Light Roast",
      "active": true,
      "isCurrentActive": true,
      "brewSetpoint": 94.0,
      "brewMode": 1,
      "preInfusionEnabled": true
    },
    ...
  ],
  "activeIndex": 0
}
```

#### POST /profiles/load
Load a specific profile.

**Parameters:**
- `index` (0-4): Profile slot to load

**Response:** `200 OK` or `400 Bad Request`

#### POST /profiles/save
Save current settings as a profile.

**Parameters:**
- `index` (0-4): Profile slot to save to
- `name`: Profile name (max 19 characters)

**Response:** `200 OK` or `400 Bad Request`

#### POST /profiles/delete
Delete a profile.

**Parameters:**
- `index` (0-4): Profile slot to delete

**Response:** `200 OK` or `400 Bad Request`

#### POST /profiles/rename
Rename a profile.

**Parameters:**
- `index` (0-4): Profile slot to rename
- `name`: New profile name (max 19 characters)

**Response:** `200 OK` or `400 Bad Request`

## Troubleshooting

### Profile doesn't load
- Ensure the profile slot is active (not empty)
- Check that the profile index is valid (0-4)
- Verify no brewing operation is in progress

### Settings not saved correctly
- Ensure all required parameters are configured
- Check that the profile name is not empty and less than 20 characters
- Verify sufficient flash memory is available

### MQTT commands not working
- Confirm MQTT is enabled and connected
- Verify the topic format matches your configuration
- Check that the payload is a valid profile index (0-4)

## Best Practices

1. **Descriptive Names**: Use clear, descriptive names like "Light Roast 94°C" instead of just "Profile 1"
2. **Document Your Profiles**: Keep notes on which coffee beans work best with each profile
3. **Regular Backups**: Use the config backup feature to save your profiles externally
4. **Test Before Use**: After loading a profile, verify the temperature and other settings before brewing
5. **Start Simple**: Begin with basic temperature and pre-infusion settings, then refine over time

## Future Enhancements

Potential future improvements to the brew profiles feature:

- Cloud backup and sharing of profiles
- Import/export individual profiles
- Profile recommendations based on coffee bean type
- Automatic profile switching based on time of day
- Profile history and usage statistics

## Support

For questions, issues, or feature requests related to brew profiles:
- Visit our [Discord](https://discord.gg/Kq5RFznuU4) server
- Create an issue on [GitHub](https://github.com/rancilio-pid/clevercoffee)
- Check the [project website](https://clevercoffee.de) for documentation
