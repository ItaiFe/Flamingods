# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

The Station ESP32 is a button controller that acts as an input device for the Flamingods art installation. It features 5 physical buttons (Red, Green, Blue, Yellow, White) that send HTTP requests to a central flamingo server (Crown ESP) when pressed. The system supports up to 4 stations, each with a unique ID and name.

## Hardware Architecture

**Button Controller → Flamingo Server → LED Displays**

```
[Station ESP32] --HTTP POST--> [Flamingo Server (Crown ESP)] --Controls--> [LED ESPs]
  (5 buttons)   Ethernet/WiFi       (192.168.1.200:80)
```

- **5 Buttons**: GPIO 2 (Red), GPIO 4 (Green), GPIO 30 (Blue), GPIO 12 (Yellow), GPIO 14 (White)
- **Pull-up Mode**: All buttons use INPUT_PULLUP (press = LOW, release = HIGH)
- **Debounce**: 50ms hardware debounce prevents false triggers
- **Network**: Built-in Ethernet (primary) with WiFi fallback to "DiMax Residency 2.4Ghz"
- **Ethernet**: ESP32-ETH01V1.4 with LAN8720A PHY (PHY_ADDR=1, POWER=GPIO16, MDC=GPIO23, MDIO=GPIO18)
- **Communication**: HTTP POST requests with JSON payloads

## Build Commands

### Standard Build and Upload
```bash
# Build for USB upload
pio run -e esp32dev

# Upload via USB
pio run -e esp32dev --target upload

# Monitor serial output (115200 baud)
pio device monitor
```

### OTA (Over-The-Air) Updates
```bash
# Set OTA password environment variable
export ESP_OTA_PASSWORD=flamingods2024

# Build and upload via OTA
pio run -e esp32dev-ota --target upload
```

### Combined Commands
```bash
# Build, upload, and monitor in sequence
pio run -e esp32dev --target upload && pio device monitor
```

## Configuration System

### Multi-Station Setup

The firmware supports 4 independent stations using `station_config.h`:

**IMPORTANT**: Before building for a specific station, copy the appropriate config:

```bash
# For Station 1
cp station_config_examples/station_1_config.h station_config.h

# For Station 2
cp station_config_examples/station_2_config.h station_config.h

# For Station 3
cp station_config_examples/station_3_config.h station_config.h

# For Station 4
cp station_config_examples/station_4_config.h station_config.h
```

Each config defines:
- `STATION_ID` - Unique integer (1, 2, 3, or 4)
- `STATION_NAME` - String identifier ("station-1", "station-2", etc.)

### Network Configuration

The station supports **dual network modes**:

1. **Ethernet (Primary)**: Built-in ESP32-ETH01V1.4 Ethernet
   - LAN8720A PHY with hardware configuration
   - PHY Address: 1
   - Power Pin: GPIO 16
   - MDC: GPIO 23, MDIO: GPIO 18
   - Clock Mode: GPIO0_IN
   - Power control on GPIO 5

2. **WiFi (Fallback)**: Automatic fallback if Ethernet fails

Edit `src/main.cpp` to change WiFi or server settings:

```cpp
// WiFi credentials (lines 23-24) - used as fallback
const char* ssid = "DiMax Residency 2.4Ghz";
const char* password = "33355555DM";

// Flamingo server endpoint (lines 26-27)
const char* flamingoServer = "http://192.168.1.200";  // Crown ESP IP
const int flamingoPort = 80;
```

### OTA Configuration

OTA settings in `src/main.cpp` and `platformio.ini`:
- Hostname: `station-esp32.local`
- Password: `flamingods2024` (set in `ESP_OTA_PASSWORD` env var)
- Upload protocol: `espota`

## Code Architecture

### Main Components

**`src/main.cpp`** - Single file containing all logic:
- Button initialization and debouncing
- Dual network support (Ethernet + WiFi fallback)
- Ethernet event handling (WiFiEvent callback)
- HTTP client for server communication
- OTA update handling
- Main control loop

### Key Data Structures

**ButtonState struct** (lines 46-51):
```cpp
struct ButtonState {
    bool pressed;                    // Current pressed state
    bool lastState;                  // Previous state for edge detection
    unsigned long lastDebounceTime;  // Timestamp for debounce
    bool active;                     // Active state after debounce
};
```

**Button arrays**:
- `buttons[5]` - State tracking for all 5 buttons
- `buttonColors[5]` - String names: ["red", "green", "blue", "yellow", "white"]

### HTTP Communication

**Single Button Press** → `POST /station-color`
```json
{
  "station": "station-1",
  "action": "color",
  "color": "red",
  "timestamp": 1234567890
}
```

**Multiple Button Press** → `POST /station-mixed-color`
```json
{
  "station": "station-1",
  "action": "mixed-color",
  "colors": ["red", "blue"],
  "timestamp": 1234567890
}
```

### Control Flow

1. **Initialization** (`setup()`):
   - Configure buttons with INPUT_PULLUP
   - Try Ethernet connection first (10 second timeout)
   - Fallback to WiFi if Ethernet fails (10 second timeout)
   - Initialize OTA only if network is connected
   - Print station ID and firmware version

2. **Network Setup** (`setupNetwork()`):
   - Register Ethernet event handler (WiFiEvent)
   - Configure GPIO 5 as PHY power pin
   - Initialize LAN8720A PHY with ESP32-ETH01 settings
   - Wait for Ethernet connection (ARDUINO_EVENT_ETH_GOT_IP)
   - If Ethernet fails, attempt WiFi connection
   - Report connection status to serial

3. **Main Loop** (`loop()`):
   - Handle OTA updates (non-blocking)
   - Poll buttons every 10ms
   - Send HTTP requests on button state changes
   - Print network status every 5 seconds (Ethernet link/speed or WiFi RSSI)

4. **Button Handling** (`checkButtons()`):
   - Read all 5 button states
   - Apply 50ms debounce
   - Detect press/release edges
   - Determine if single or multiple buttons pressed
   - Call appropriate HTTP send function

5. **Ethernet Events** (`WiFiEvent()`):
   - ETH_START: Set hostname
   - ETH_CONNECTED: Log connection
   - ETH_GOT_IP: Set ethernetConnected flag, display IP/MAC/speed
   - ETH_DISCONNECTED/ETH_STOP: Clear ethernetConnected flag

## Timing Constants

- **Button Check**: Every 10ms (line 64: `lastButtonCheck`)
- **Debounce Time**: 50ms (line 43: `DEBOUNCE_TIME`)
- **Status Update**: Every 5 seconds (line 63: `lastStatusUpdate`)
- **HTTP Timeout**: Default HTTPClient timeout

## Dependencies

From `platformio.ini`:
- **FastLED** @ ^3.5.0 - LED library (included for compatibility, not actively used)
- **ArduinoJson** @ ^6.21.0 - JSON serialization for HTTP payloads
- **ESP32 Arduino Framework** - Core WiFi and HTTP functionality

## Serial Output

Monitor at 115200 baud for debugging:

**Ethernet Connection (Primary)**:
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
```

**WiFi Connection (Fallback)**:
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

**Button Events**:
```
Button red pressed
Sending color red to flamingo server
HTTP Response code: 200
Response: {"status":"success","color":"red"}
Button red released
```

**Status Updates (Every 5 seconds)**:
```
Status: Ethernet: 192.168.1.100, Link: UP, Speed: 100 Mbps, OTA: Idle
```
or
```
Status: WiFi: 192.168.1.201, RSSI: -42, OTA: Idle
```

## Common Issues and Solutions

### Buttons Not Responding
- **Check wiring**: Each button connects GPIO pin to GND
- **Monitor serial**: Look for "Button [color] pressed" messages
- **Verify pins**: GPIO 2, 4, 30, 12, 14 (see `wiring_diagram.txt`)
- **Test individually**: Press one button at a time

### HTTP Failures
- **Check Network**: Verify "Ethernet connected!" or "WiFi connected!" in serial output
- **Ethernet**: If using Ethernet, check cable connection and link status
- **WiFi**: If using WiFi fallback, check RSSI and signal strength
- **Server IP**: Ensure `192.168.1.200` is correct Crown ESP address
- **Network**: Ping flamingo server from another device
- **Response codes**: Non-200 codes indicate server issues

### Ethernet Not Working
- **Hardware**: Ensure ESP32-ETH01V1.4 board (not regular ESP32)
- **PHY Chip**: Verify LAN8720A PHY is present and powered
- **Cable**: Check Ethernet cable is connected and link LED is lit
- **Power**: GPIO 5 controls PHY power - verify it's HIGH
- **Pin Config**: PHY_ADDR=1, POWER=16, MDC=23, MDIO=18 must match hardware
- **Fallback**: System will automatically try WiFi if Ethernet fails

### Multiple Stations Conflict
- **Station ID**: Each physical station MUST have unique `STATION_ID` (1-4)
- **Config file**: Verify correct `station_config.h` was copied before build
- **Hostname**: OTA hostname is same for all stations (`station-esp32.local`)

### OTA Upload Fails
- **Password**: Set `export ESP_OTA_PASSWORD=flamingods2024`
- **Network**: Station must be on same network as development machine
- **mDNS**: Ensure `station-esp32.local` resolves (may need IP instead)
- **First upload**: Must use USB for initial firmware, OTA only for updates

## Development Workflow

### Setting Up a New Station

1. Clone and configure:
   ```bash
   cd esps/station
   cp station_config_examples/station_1_config.h station_config.h
   ```

2. Edit WiFi/server if needed in `src/main.cpp`

3. Build and upload:
   ```bash
   pio run -e esp32dev --target upload
   ```

4. Verify via serial monitor:
   ```bash
   pio device monitor
   ```

5. Test buttons and check HTTP responses

### Updating Existing Station

1. Ensure OTA password is set:
   ```bash
   export ESP_OTA_PASSWORD=flamingods2024
   ```

2. OTA upload (if station is powered and on network):
   ```bash
   pio run -e esp32dev-ota --target upload
   ```

3. Fallback to USB if OTA fails:
   ```bash
   pio run -e esp32dev --target upload
   ```

## File Reference

- **`src/main.cpp`** - Complete firmware implementation
- **`station_config.h`** - Active station configuration (git-ignored, must be created)
- **`station_config_examples/`** - Pre-configured examples for stations 1-4
- **`wiring_diagram.txt`** - Detailed hardware connection guide
- **`platformio.ini`** - Build configuration for USB and OTA targets
- **`README.md`** - User-facing documentation

## Integration with Flamingods System

The station expects the flamingo server (Crown ESP at 192.168.1.200) to expose these endpoints:
- `POST /station-color` - Accept single color button press
- `POST /station-mixed-color` - Accept multiple simultaneous buttons

See the crown ESP firmware for server-side implementation details.
