# Mock Mode for Frontend Testing

This document explains how to use the mock mode for testing the Clever Coffee frontend without connecting to an actual ESP32 device.

## Enabling Mock Mode

To enable mock mode, simply add `?mock=true` to the URL:

```
http://your-device.local/index.html?mock=true
http://your-device.local/parameters.html?mock=true
```

For local testing:
```
http://localhost:8080/html/index.html?mock=true
```

## Features

When mock mode is enabled, you can:

1. **View Simulated Data**: See realistic coffee machine parameters and values
2. **Test Toggle Switches**: Click PID, Steam Mode, and Backflush switches - they work instantly
3. **See Live Temperature**: Temperature fluctuates realistically around 92.3°C (±1°C)
4. **Test Forms**: Change brew temperature and other settings
5. **No Backend Required**: Everything works client-side without ESP32 connection

## Mock Data Included

The mock mode provides:
- PID Controller (enabled by default)
- Brew Temperature (93.5°C setpoint)
- Steam Mode toggle
- Backflush Mode toggle
- Scale functions (Tare, Calibration)
- Live temperature simulation

## Console Messages

When mock mode is active, you'll see:
```
🎭 Mock mode enabled - using test data
🔄 Mock: Toggled STEAM_MODE to 1
📝 Mock: Parameters saved
```

## Use Cases

- **Frontend Development**: Test UI changes without hardware
- **Design Review**: Show stakeholders the interface
- **User Testing**: Collect feedback on UX
- **Screenshots**: Generate documentation images
- **Demo Mode**: Demonstrate features without physical device

## Technical Details

Mock mode is implemented in `/js/mock-data.js` and automatically intercepts:
- `fetchParameters()` - Returns sample parameters
- `toggleFunction()` - Simulates toggle state changes
- Temperature display - Updates every 2 seconds

The mock system checks `window.MockAPI.isMockMode` and provides fallback data when the ESP32 backend is unavailable.
