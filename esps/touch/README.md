# ESP32 Touch Sensor Firmware

A specialized ESP32 firmware designed to detect electrical shortage/contact using built-in capacitive touch sensors. This firmware monitors multiple touch pins and provides real-time detection of electrical contact or shortage conditions.

## Features

- **Multi-Sensor Detection**: Monitors 6 capacitive touch sensors simultaneously
- **Shortage Detection**: Detects electrical shortage or contact when sensors are touched
- **Real-time Monitoring**: Continuous monitoring with immediate alerts
- **Visual Feedback**: LED indicators for touch status
- **Serial Output**: Detailed logging and debugging information
- **WiFi Connectivity**: Optional network connection for remote monitoring
- **OTA Updates**: Over-the-air firmware updates via WiFi
- **Debounced Detection**: Prevents false triggers with proper debouncing

## Hardware Requirements

- ESP32 development board
- Touch electrodes/conductive surfaces connected to touch pins
- LED indicators (optional)
- WiFi network access (optional, for OTA updates)
- USB power supply or external power

## Touch Sensor Configuration

The firmware uses ESP32's built-in capacitive touch sensors:

| Sensor | GPIO Pin | Touch Pin | Description |
|--------|----------|-----------|-------------|
| Touch-0 | GPIO 4 | T0 | Primary touch sensor |
| Touch-1 | GPIO 0 | T1 | Secondary touch sensor |
| Touch-2 | GPIO 2 | T2 | Built-in LED pin (can be used as touch) |
| Touch-3 | GPIO 15 | T3 | Additional touch sensor |
| Touch-4 | GPIO 13 | T4 | Additional touch sensor |
| Touch-5 | GPIO 12 | T5 | Additional touch sensor |

## Hardware Setup

### Basic Setup
1. **Connect Touch Electrodes**: Attach conductive surfaces (copper tape, aluminum foil, etc.) to the touch pins
2. **Power Supply**: Connect ESP32 to stable power source
3. **LED Indicators**: Connect LEDs to GPIO 2 (built-in) and GPIO 5 (touch indication)

### Advanced Setup
1. **Multiple Electrodes**: Connect different conductive surfaces to different touch pins
2. **Isolation**: Ensure proper electrical isolation between different touch areas
3. **Grounding**: Connect a common ground reference for better touch detection

## Building and Flashing

### Prerequisites
- PlatformIO CLI installed
- FTDI cable or USB connection to ESP32
- WiFi network with SSID "DiMax Residency 2.4Ghz" and password "33355555DM" (for OTA)

### Quick Start
```bash
# Navigate to the touch directory
cd esps/touch

# Flash via FTDI cable (first time)
./test_flash.sh

# Flash via OTA (after WiFi is set up)
./test_flash.sh --ota
```

### Manual Commands

#### FTDI Cable Flashing
```bash
# Build for FTDI
pio run -e esp32dev

# Upload via FTDI
pio run -e esp32dev --target upload --upload-port /dev/cu.usbserial-A5069RR4

# Monitor serial output
pio device monitor --port /dev/cu.usbserial-A5069RR4 --baud 115200
```

#### OTA Updates
```bash
# Build for OTA
pio run -e esp32dev-ota

# Upload via OTA (ESP32 must be connected to WiFi)
pio run -e esp32dev-ota --target upload
```

## Configuration

### Touch Sensitivity
Adjust the touch sensitivity by modifying `TOUCH_THRESHOLD` in the code:
```cpp
#define TOUCH_THRESHOLD 20  // Lower values = more sensitive
```

### Debounce Time
Adjust the debounce time to prevent false triggers:
```cpp
#define TOUCH_DEBOUNCE_MS 50  // Debounce time in milliseconds
```

### WiFi Settings
Update WiFi credentials in the code:
```cpp
const char* ssid = "YourWiFiSSID";
const char* password = "YourWiFiPassword";
```

## Expected Output

### Normal Operation
```
========================================
    ESP32 Touch Sensor Firmware
========================================
Firmware Version: 1.0.0
Chip ID: 4C3F820C
Free heap: 327680 bytes
CPU Frequency: 240 MHz
========================================
Touch sensor shortage detection active!
========================================
LEDs initialized
Touch sensors initialized
Touch sensor 0 (Touch-0) on GPIO 4 initialized
Touch sensor 1 (Touch-1) on GPIO 0 initialized
Touch sensor 2 (Touch-2) on GPIO 2 initialized
Touch sensor 3 (Touch-3) on GPIO 15 initialized
Touch sensor 4 (Touch-4) on GPIO 13 initialized
Touch sensor 5 (Touch-5) on GPIO 12 initialized
Touch sensors initialized successfully
WiFi connected!
IP address: 192.168.1.234
OTA ready
Setup complete - monitoring touch sensors...
Touch any sensor to detect electrical shortage!

[2] Touch Status: Touch-0:0(45) Touch-1:0(52) Touch-2:0(38) Touch-3:0(41) Touch-4:0(47) Touch-5:0(43) Events:0 Active:NO Free:327680 WiFi:192.168.1.234
```

### Touch Detection
```
🔴 TOUCH DETECTED! Sensor 0 (Touch-0) - SHORTAGE ALERT!
   Raw value: 15 (threshold: 20)
   Time: 12345 ms
   ⚠️  Electrical shortage or contact detected!

[4] Touch Status: Touch-0:1(15) Touch-1:0(52) Touch-2:0(38) Touch-3:0(41) Touch-4:0(47) Touch-5:0(43) Events:1 Active:YES Free:327680 WiFi:192.168.1.234
```

### Touch Release
```
🟢 Touch released on sensor 0 (Touch-0)
   Raw value: 45 (threshold: 20)

[6] Touch Status: Touch-0:0(45) Touch-1:0(52) Touch-2:0(38) Touch-3:0(41) Touch-4:0(47) Touch-5:0(43) Events:1 Active:NO Free:327680 WiFi:192.168.1.234
```

## Troubleshooting

### Touch Not Detected
1. **Check Connections**: Verify touch electrodes are properly connected to touch pins
2. **Adjust Sensitivity**: Lower the `TOUCH_THRESHOLD` value for more sensitive detection
3. **Check Grounding**: Ensure proper ground reference for touch detection
4. **Test Individual Pins**: Use serial monitor to check raw touch values

### False Triggers
1. **Increase Debounce**: Increase `TOUCH_DEBOUNCE_MS` value
2. **Adjust Sensitivity**: Increase `TOUCH_THRESHOLD` value for less sensitive detection
3. **Check Interference**: Ensure no electrical interference near touch sensors

### WiFi Issues
1. **Check Credentials**: Verify WiFi SSID and password are correct
2. **Check Signal**: Ensure strong WiFi signal strength
3. **Check Network**: Verify network allows ESP32 connections

### Upload Issues
1. **Check Port**: Verify correct serial port for FTDI cable
2. **Check Boot Mode**: Ensure ESP32 is in download mode
3. **Check Power**: Ensure stable power during upload

## Applications

### Electrical Safety
- Detect electrical shortage in equipment
- Monitor for unwanted electrical contact
- Safety interlock systems

### Industrial Monitoring
- Machine safety systems
- Equipment status monitoring
- Process control applications

### Art Installations
- Interactive touch surfaces
- Proximity detection
- User interaction systems

## Technical Details

### Touch Detection Algorithm
1. **Raw Value Reading**: Reads capacitive touch values from ESP32 hardware
2. **Threshold Comparison**: Compares values against configurable threshold
3. **Debouncing**: Prevents false triggers with time-based debouncing
4. **State Management**: Tracks touch state changes and events

### Performance
- **Sampling Rate**: Continuous monitoring with 10ms loop delay
- **Response Time**: < 50ms touch detection (configurable debounce)
- **Memory Usage**: Minimal memory footprint with efficient algorithms
- **Power Consumption**: Low power operation suitable for battery applications

### Safety Features
- **Debounced Detection**: Prevents false alarms
- **Multiple Sensors**: Redundant detection capability
- **Visual Indicators**: LED feedback for immediate status
- **Serial Logging**: Detailed event logging for analysis

## Success Criteria

The touch sensor is working correctly if:
- ✅ Firmware uploads without errors
- ✅ Serial output shows initialization messages
- ✅ Touch sensors respond to contact
- ✅ LED indicators show touch status
- ✅ No false triggers during normal operation
- ✅ WiFi connects successfully (if configured)
- ✅ OTA updates work (if WiFi is available)

## Next Steps

If the touch sensor works:
1. Calibrate sensitivity for your specific application
2. Integrate with your main control system
3. Add additional sensors as needed
4. Implement remote monitoring features

If the touch sensor fails:
1. Check hardware connections
2. Verify touch electrode setup
3. Adjust sensitivity settings
4. Check for electrical interference