# Station ESP32 - Button Controller for Flamingo Server

A specialized ESP32 device that acts as a button controller station, sending HTTP requests to a central flamingo server when buttons are pressed.

## Features

- **5 Physical Buttons**: Red, Green, Blue, Yellow, and White
- **HTTP Client**: Communicates with flamingo server via HTTP POST requests
- **Color Mixing**: Automatically detects multiple button presses and sends mixed color data
- **WiFi Connectivity**: Connects to local network to reach flamingo server
- **OTA Updates**: Over-the-air firmware updates supported
- **Debounced Input**: 50ms debounce time prevents false triggers

## Hardware Requirements

- ESP32 development board
- 5x momentary push buttons (Red, Green, Blue, Yellow, White)
- Breadboard and jumper wires for testing
- USB-C cable for programming

## Pin Configuration

| Button Color | GPIO Pin | Description |
|--------------|----------|-------------|
| Red          | GPIO 2   | Red button input |
| Green        | GPIO 4   | Green button input |
| Blue         | GPIO 5   | Blue button input |
| Yellow       | GPIO 18  | Yellow button input |
| White        | GPIO 19  | White button input |

## Wiring

- Each button connects between its assigned GPIO pin and GND
- Internal pull-up resistors are enabled (INPUT_PULLUP mode)
- Button press = LOW signal, Button release = HIGH signal

See `wiring_diagram.txt` for detailed wiring instructions.

## Configuration

### WiFi Settings
```cpp
const char* ssid = "DiMax Residency 2.4Ghz";
const char* password = "33355555DM";
```

### Flamingo Server
```cpp
const char* flamingoServer = "http://192.168.1.200";  // Crown ESP IP
const int flamingoPort = 80;
```

### OTA Settings
```cpp
ArduinoOTA.setHostname("station-esp32");
ArduinoOTA.setPassword("flamingods2024");
```

## Functionality

### Single Button Press
When one button is pressed:
- Sends HTTP POST to `/station-color`
- JSON payload: `{"station": "station-esp32", "action": "color", "color": "red", "timestamp": 1234567890}`

### Multiple Button Press
When multiple buttons are pressed simultaneously:
- Sends HTTP POST to `/station-mixed-color`
- JSON payload: `{"station": "station-esp32", "action": "mixed-color", "colors": ["red", "blue"], "timestamp": 1234567890}`

### Button States
- **Active**: Button is currently pressed
- **Inactive**: Button is released
- **Debounced**: 50ms delay prevents false triggers

## HTTP Endpoints

The station ESP sends requests to these flamingo server endpoints:

- **POST** `/station-color` - Single color selection
- **POST** `/station-mixed-color` - Multiple color selection

## Building and Uploading

### Prerequisites
- PlatformIO IDE or CLI
- ESP32 development environment

### Build Commands
```bash
# Build for standard upload
pio run -e esp32dev

# Build for OTA upload
pio run -e esp32dev-ota

# Upload via USB
pio run -e esp32dev --target upload

# Upload via OTA
pio run -e esp32dev-ota --target upload
```

### OTA Upload
1. Set environment variable: `export ESP_OTA_PASSWORD=flamingods2024`
2. Build and upload: `pio run -e esp32dev-ota --target upload`

## Serial Output

The station ESP provides detailed serial output for debugging:

```
=== Station ESP32 Starting ===
Firmware Version: 1.0.0
Buttons initialized
Connecting to WiFi: DiMax Residency 2.4Ghz
....
WiFi connected!
IP address: 192.168.1.201
OTA initialized
Station ESP32 initialization complete!
Button red pressed
Sending color red to flamingo server
HTTP Response code: 200
Response: {"status":"success","color":"red"}
Button red released
```

## Troubleshooting

### Button Not Responding
- Check GPIO pin connections
- Verify GND connections
- Monitor serial output for button events
- Check button quality and wiring

### HTTP Communication Issues
- Verify WiFi connection
- Check flamingo server IP address
- Monitor serial output for HTTP responses
- Ensure network connectivity

### Multiple Button Triggers
- Check debounce timing (50ms)
- Verify button quality
- Check for electrical noise
- Ensure proper pull-up resistors

## Network Architecture

```
[Station ESP32] --WiFi--> [Flamingo Server (Crown ESP)] --WiFi--> [Other ESPs]
    5 Buttons                   192.168.1.200:80
```

## Development Notes

- **Debounce Time**: 50ms prevents false triggers from button bounce
- **Button Check Frequency**: Every 10ms for responsive input
- **Status Updates**: Every 5 seconds for monitoring
- **HTTP Timeout**: Default HTTP client timeout settings
- **JSON Payload**: Uses ArduinoJson library for structured data

## Future Enhancements

- Configurable button mappings
- Multiple flamingo server support
- Button press duration detection
- LED feedback for button states
- Battery backup for mobile use
- Bluetooth configuration interface
