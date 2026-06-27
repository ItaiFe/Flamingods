# Station ESP32 - Button Controller for Flamingo Server

A specialized ESP32 device that acts as a button controller station, sending HTTP requests to a central flamingo server when buttons are pressed.

## Features

- **5 Physical Buttons**: Red, Green, Blue, Yellow, and White
- **HTTP Client**: Communicates with flamingo server via HTTP POST requests
- **Color Mixing**: Automatically detects multiple button presses and sends mixed color data
- **Dual Network Support**: Built-in Ethernet (primary) with WiFi fallback
- **OTA Updates**: Over-the-air firmware updates supported
- **Debounced Input**: 50ms debounce time prevents false triggers

## Hardware Requirements

- **ESP32-ETH01V1.4** board with built-in Ethernet (recommended)
  - Or regular ESP32 development board for WiFi-only operation
- **LAN8720A PHY** for Ethernet connectivity (built into ESP32-ETH01)
- 5x momentary push buttons (Red, Green, Blue, Yellow, White)
- Ethernet cable and network access (if using Ethernet)
- Breadboard and jumper wires for testing
- USB-C cable for programming

## Pin Configuration

| Button Color | GPIO Pin | Description |
|--------------|----------|-------------|
| Red          | GPIO 2   | Red button input |
| Green        | GPIO 4   | Green button input |
| Blue         | GPIO 30  | Blue button input |
| Yellow       | GPIO 12  | Yellow button input |
| White        | GPIO 14  | White button input |

## Wiring

- Each button connects between its assigned GPIO pin and GND
- Internal pull-up resistors are enabled (INPUT_PULLUP mode)
- Button press = LOW signal, Button release = HIGH signal

See `wiring_diagram.txt` for detailed wiring instructions.

## Configuration

### Network Settings

The station supports **dual network modes**:

1. **Ethernet (Primary)**: Built-in ESP32-ETH01V1.4 Ethernet
   - Automatically tries Ethernet connection first
   - LAN8720A PHY (PHY_ADDR=1, POWER=GPIO16, MDC=GPIO23, MDIO=GPIO18)
   - GPIO 5 controls PHY power
   - 10 second connection timeout

2. **WiFi (Fallback)**: Automatic fallback if Ethernet fails
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

### With Ethernet Connection
```
=== Station ESP32 Starting ===
Station ID: 1 (station-1)
Firmware Version: 1.0.0
Buttons initialized
Initializing built-in Ethernet...
Setting up power pin GPIO 5...
Configuration: PHY_ADDR=1, POWER=16, MDC=23, MDIO=18, TYPE=LAN8720, CLK=GPIO0_IN
ETH.begin() succeeded - waiting for connection...
Waiting for Ethernet connection...
ETH Started
ETH Connected
ETH MAC: DE:AD:BE:EF:FE:01, IPv4: 192.168.1.100, FULL_DUPLEX, 100Mbps
Built-in Ethernet connected!
IP address: 192.168.1.100
OTA initialized
Station ESP32 initialization complete!
Button red pressed
Sending color red to flamingo server
HTTP Response code: 200
Response: {"status":"success","color":"red"}
Button red released
```

### With WiFi Fallback
```
=== Station ESP32 Starting ===
Station ID: 1 (station-1)
Firmware Version: 1.0.0
Buttons initialized
Initializing built-in Ethernet...
Setting up power pin GPIO 5...
Configuration: PHY_ADDR=1, POWER=16, MDC=23, MDIO=18, TYPE=LAN8720, CLK=GPIO0_IN
ETH.begin() succeeded - waiting for connection...
Waiting for Ethernet connection...
....................
Built-in Ethernet connection failed, trying WiFi...
Connecting to WiFi: DiMax Residency 2.4Ghz
..........
WiFi connected!
IP address: 192.168.1.201
OTA initialized
Station ESP32 initialization complete!
```

## Troubleshooting

### Button Not Responding
- Check GPIO pin connections
- Verify GND connections
- Monitor serial output for button events
- Check button quality and wiring

### Ethernet Not Working
- **Hardware Check**: Ensure you have ESP32-ETH01V1.4 board (not regular ESP32)
- **PHY Chip**: Verify LAN8720A PHY is present and functional
- **Cable**: Check Ethernet cable is connected properly
- **Link LED**: Verify Ethernet link LED is lit on the board
- **Power**: GPIO 5 controls PHY power - ensure it's HIGH
- **Fallback**: System will automatically try WiFi if Ethernet fails

### HTTP Communication Issues
- **Network**: Verify "Ethernet connected!" or "WiFi connected!" in serial
- **Ethernet Link**: Check `Link: UP` in status messages
- **WiFi Signal**: Check RSSI value (should be > -70 for reliable connection)
- Check flamingo server IP address (default: 192.168.1.200)
- Monitor serial output for HTTP responses
- Ping flamingo server from another device
- Check network connectivity

### Multiple Button Triggers
- Check debounce timing (50ms)
- Verify button quality
- Check for electrical noise
- Ensure proper pull-up resistors

## Network Architecture

```
[Station ESP32] --Ethernet/WiFi--> [Flamingo Server (Crown ESP)] --WiFi--> [Other ESPs]
    5 Buttons                          192.168.1.200:80
```

The station first attempts Ethernet connection, then falls back to WiFi if Ethernet is unavailable.

## Development Notes

- **Network Priority**: Ethernet tried first, WiFi as fallback
- **Ethernet Timeout**: 10 seconds to establish Ethernet connection
- **WiFi Timeout**: 10 seconds to establish WiFi connection
- **Debounce Time**: 50ms prevents false triggers from button bounce
- **Button Check Frequency**: Every 10ms for responsive input
- **Status Updates**: Every 5 seconds for monitoring (includes link speed/RSSI)
- **HTTP Timeout**: Default HTTP client timeout settings
- **JSON Payload**: Uses ArduinoJson library for structured data
- **Ethernet PHY**: LAN8720A with GPIO 5 power control

## Future Enhancements

- Configurable button mappings
- Multiple flamingo server support
- Button press duration detection
- LED feedback for button states
- Battery backup for mobile use
- Bluetooth configuration interface
