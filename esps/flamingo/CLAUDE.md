# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Flamingo ESP32 is an LED lighting controller for the Midburn art installation. It controls 4 WS2812B LED strips (100 LEDs each) with different lighting plans triggered via HTTP endpoints. The system supports dual network connectivity (Ethernet primary, WiFi fallback) and provides OTA firmware updates.

## Hardware Architecture

**LED Controller → 4 LED Strips**

```
[Flamingo ESP32] ──┬── GPIO 4 → Red LED Strip (100 LEDs)
                   ├── GPIO 2 → Green LED Strip (100 LEDs)
                   ├── GPIO 5 → Blue LED Strip (100 LEDs)
                   └── GPIO 18 → Yellow LED Strip (100 LEDs)
```

- **ESP32-ETH01V1.4** board with built-in Ethernet (LAN8720A PHY)
- **4 WS2812B LED Strips**: 100 LEDs each, controlled via FastLED
- **Network**: Ethernet (primary) with WiFi fallback
- **Pin Swap**: Red and Green pins are swapped (Red→GPIO4, Green→GPIO2)
- **Power**: 5V power supply, 8A recommended for full brightness

## Build Commands

### Standard Build and Upload
```bash
# Using Makefile (recommended)
make build          # Build project
make upload         # Build and upload via USB
make monitor        # Monitor serial output (115200 baud)
make deploy         # Build + upload
make deploy-monitor # Build + upload + monitor

# Using PlatformIO directly
pio run                    # Build
pio run --target upload    # Upload via USB
pio device monitor         # Monitor serial
```

### OTA (Over-The-Air) Updates
```bash
# Set OTA password
export ESP_OTA_PASSWORD=flamingods2024

# Upload via OTA (using hostname)
pio run -e esp32dev-ota --target upload

# Upload via OTA (using IP address - more reliable)
pio run -e esp32dev-ota --target upload --upload-port <IP_ADDRESS>
```

## Code Architecture

### Main Components

**`src/main.cpp`** - Main controller (400+ lines):
- Dual network setup (Ethernet + WiFi fallback)
- HTTP web server with lighting plan endpoints
- OTA update handling
- Main loop coordination
- Ethernet event handling (WiFiEvent callback)

**`src/led_plans.cpp`** - LED pattern implementations:
- `playIdleAnimation()` - Flowing rainbow pattern
- `playMovingPattern()` - Moving blue dot pattern
- `clearAllLeds()` - Turn off all LEDs

**`include/led_plans.h`** - Configuration and declarations:
- LED pin definitions
- Strip configuration (NUM_LEDS_PER_STRIP, BRIGHTNESS)
- Pattern state enum
- Function prototypes

### Lighting Plans

The system has 5 lighting plans triggered via HTTP and station button presses:

1. **PLAN_IDLE** (0) - Default state
   - Flowing rainbow pattern across all strips
   - Dynamic wave motion with varying brightness
   - 20ms update interval
   - Auto-returns to this when no station colors are active

2. **PLAN_SKIP** (1) - Skip mode
   - Moving blue dot pattern
   - 100ms update interval

3. **PLAN_SHOW** (2) - Show mode
   - Moving blue dot pattern
   - Same as SKIP

4. **PLAN_SPECIAL** (3) - Special mode
   - Moving blue dot pattern
   - Same as SKIP

5. **PLAN_MIXED_COLORS** (4) - **NEW**: Multi-station color mixing
   - Blends colors from different stations together
   - Example: Station 1 presses RED + Station 2 presses BLUE = PURPLE displayed
   - Colors expire after 5 seconds (configurable via `COLOR_TIMEOUT_MS`)
   - Solid color fill with subtle breathing effect
   - Automatically activated when stations send color data
   - Returns to IDLE when all colors expire

### HTTP Endpoints

**Lighting Control**:
- `POST /idle` - Activate idle (rainbow) pattern
- `POST /skip` - Activate skip (moving) pattern
- `POST /show` - Activate show (moving) pattern
- `POST /special` - Activate special (moving) pattern

**Station Integration** (for button controller):
- `POST /station-color` - Handle single button press from station
  - Expects JSON: `{"station_id": 1, "station_name": "station-1", "color": "red", "action": "color", "timestamp": 123456}`
  - Activates SHOW plan
  - Returns JSON: `{"status": "success", "color": "red", "station_id": 1}`

- `POST /station-mixed-color` - Handle multiple button presses from station
  - Expects JSON: `{"station_id": 1, "station_name": "station-1", "colors": ["red", "blue"], "action": "mixed-color", "timestamp": 123456}`
  - Activates SPECIAL plan
  - Returns JSON: `{"status": "success", "station_id": 1, "colors": ["red", "blue"]}`

**Status & Monitoring**:
- `GET /status` - JSON status (plan, network, uptime, station tracking)
  - Includes: `total_requests`, `last_station_id`, `last_station_name`, `last_request_age_ms`
  - Useful for monitoring concurrent station activity
- `GET /health` - Health check
- `GET /version` - Firmware version
- `GET /ota` - OTA information
- `GET /ota/status` - OTA progress

### Network Configuration

**Ethernet (Primary)**:
- ESP32-ETH01V1.4 with LAN8720A PHY
- PHY Address: 1
- Power Pin: GPIO 5
- MDC: GPIO 23, MDIO: GPIO 18
- Clock Mode: GPIO0_IN

**WiFi (Fallback)**:
```cpp
const char* ssid = "DiMax Residency 2.4Ghz";
const char* password = "33355555DM";
```

**OTA Settings**:
- Hostname: `flamingo-esp32.local`
- Password: `flamingods2024`
- Port: Default (3232)

### Control Flow

1. **Initialization** (`setup()`):
   - Initialize 4 LED strips with FastLED
   - Set brightness to 100 (0-255 scale)
   - Try Ethernet connection first
   - Fallback to WiFi if Ethernet fails
   - Setup OTA
   - Setup HTTP server with endpoints
   - Start with PLAN_IDLE

2. **Main Loop** (`loop()`):
   - Handle OTA updates (non-blocking)
   - Handle HTTP requests
   - Update LED patterns based on currentPlan
   - Print status every 5 seconds
   - 20ms delay for stability

3. **Pattern Updates**:
   - PLAN_IDLE: Continuous rainbow animation
   - PLAN_SKIP/SHOW/SPECIAL: Moving dot pattern
   - Patterns run independently in main loop

## LED Configuration

From `led_plans.h`:
- **NUM_LEDS_PER_STRIP**: 100 LEDs per strip
- **BRIGHTNESS**: 100 (out of 255)
- **LED_RED_PIN**: 4 (swapped with green)
- **LED_GREEN_PIN**: 2 (swapped with red)
- **LED_BLUE_PIN**: 5
- **LED_YELLOW_PIN**: 18

**IMPORTANT**: Red and Green are physically swapped due to pin configuration.

## Dependencies

From `platformio.ini`:
- **FastLED** @ ^3.5.0 - LED control library
- **ArduinoJson** @ ^6.21.0 - JSON serialization
- **ESP32 Arduino Framework** - Core WiFi/Ethernet/WebServer
- **ArduinoOTA** - Built-in OTA updates

## Serial Output

Monitor at 115200 baud:

**Ethernet Connection**:
```
=== Flamingo ESP32 Starting ===
Firmware Version: 1.0.0
Initializing built-in Ethernet...
ETH Started
ETH Connected
ETH MAC: xx:xx:xx:xx:xx:xx, IPv4: 192.168.1.xxx, FULL_DUPLEX, 100Mbps
Ethernet connected!
IP address: 192.168.1.xxx
OTA initialized
HTTP Server started on port 80
Flamingo ESP32 initialization complete!
```

**Status Updates (Every 5 seconds)**:
```
Status: Plan 0, Ethernet: 192.168.1.xxx, OTA: Idle
```

## Testing

**Test Patterns Script** (`test_patterns.py`):
```bash
# Test all patterns via HTTP
python test_patterns.py
```

**Manual HTTP Testing**:
```bash
# Activate idle pattern
curl -X POST http://<IP>/idle

# Activate show pattern
curl -X POST http://<IP>/show

# Check status
curl http://<IP>/status | jq
```

## Common Issues

### LEDs Not Lighting
- **Power**: Check 5V supply (8A recommended for 400 LEDs at full brightness)
- **Data Flow**: Verify LED strip orientation (data flows one direction)
- **Pins**: Red→GPIO4, Green→GPIO2 (swapped!), Blue→GPIO5, Yellow→GPIO18
- **Wiring**: Check connections in `wiring_diagram.txt`

### Network Issues
- **Ethernet**: Verify ESP32-ETH01V1.4 hardware (not regular ESP32)
- **Cable**: Check Ethernet cable connection
- **WiFi Fallback**: System auto-switches to WiFi if Ethernet fails
- **IP Discovery**: Check serial output for assigned IP address

### Pattern Issues
- **Brightness**: Default is 100/255 - adjust in `led_plans.h`
- **Speed**: Idle updates every 20ms, Moving every 100ms
- **Stuck Pattern**: Send HTTP POST to change plan

### OTA Upload Fails
- **Password**: Must set `export ESP_OTA_PASSWORD=flamingods2024`
- **Network**: ESP must be powered and on network
- **mDNS**: Use IP address instead of `flamingo-esp32.local` if mDNS fails
- **First Upload**: Must use USB for initial firmware

## Development Workflow

### Adding New LED Patterns

1. Add pattern function to `led_plans.cpp`:
   ```cpp
   void playNewPattern() {
       // Pattern implementation
       FastLED.show();
   }
   ```

2. Add declaration to `led_plans.h`:
   ```cpp
   void playNewPattern();
   ```

3. Add enum value to main.cpp:
   ```cpp
   enum LightingPlan {
       PLAN_IDLE = 0,
       PLAN_SKIP = 1,
       PLAN_SHOW = 2,
       PLAN_SPECIAL = 3,
       PLAN_NEW = 4  // Add this
   };
   ```

4. Add HTTP endpoint and loop case in `main.cpp`

5. Build and test

### Changing LED Configuration

Edit `include/led_plans.h`:
```cpp
#define NUM_LEDS_PER_STRIP 100  // Change number of LEDs
#define BRIGHTNESS 100          // Change brightness (0-255)
```

### Modifying Network Settings

Edit `src/main.cpp`:
```cpp
const char* ssid = "YourSSID";
const char* password = "YourPassword";
```

## File Reference

- **`src/main.cpp`** - Main controller and HTTP server
- **`src/led_plans.cpp`** - LED pattern implementations
- **`include/led_plans.h`** - LED configuration and declarations
- **`platformio.ini`** - Build configuration (USB and OTA)
- **`Makefile`** - Convenient build commands
- **`test_patterns.py`** - HTTP testing script
- **`wiring_diagram.txt`** - Hardware connection guide
- **`README.md`** - User documentation

## Integration with Flamingods System

The flamingo ESP acts as an LED controller in the larger Flamingods ecosystem:

### Station Integration

The flamingo receives HTTP POST requests from station controllers (esps/station/):

**Single Button Press Flow**:
1. Station detects button press (e.g., red button)
2. Station sends POST to `http://192.168.1.200/station-color`
3. Flamingo parses JSON payload
4. Flamingo switches to SHOW plan (moving pattern)
5. Flamingo returns success response

**Multiple Button Press Flow**:
1. Station detects multiple buttons pressed
2. Station sends POST to `http://192.168.1.200/station-mixed-color`
3. Flamingo parses JSON with color array
4. Flamingo switches to SPECIAL plan (moving pattern)
5. Flamingo returns success response

### JSON Payload Examples

**From station (single color)**:
```json
{
  "station_id": 1,
  "station_name": "station-1",
  "action": "color",
  "color": "red",
  "timestamp": 1234567890
}
```

**From station (mixed colors)**:
```json
{
  "station_id": 1,
  "station_name": "station-1",
  "action": "mixed-color",
  "colors": ["red", "blue", "yellow"],
  "timestamp": 1234567890
}
```

### Communication Architecture

```
[Station ESP32] ──HTTP POST──> [Flamingo ESP32] ──Controls──> [4 LED Strips]
  (Buttons)      192.168.1.200     (This Device)              (WS2812B)
```

- Station controllers at various IPs send button presses
- Flamingo listens on 192.168.1.200:80 (configurable)
- Flamingo translates button data to lighting patterns
- Runs autonomously with idle animation when not triggered
- Provides status endpoints for monitoring

### Parallel Request Handling

The flamingo is designed to handle multiple concurrent requests from different stations:

**Non-Blocking Request Processing**:
- HTTP handlers respond immediately without waiting for LED updates
- Pattern changes happen asynchronously in the main loop
- No blocking delays in request handlers

**Timestamp-Based Conflict Resolution**:
- Each request includes a `timestamp` field from the station
- If an old request arrives after a newer one, it's ignored (within 60 second window)
- Prevents race conditions where network delays cause old requests to override new ones
- Example: If Station 2's request (timestamp: 1000) arrives before Station 1's request (timestamp: 500), Station 1's request is rejected

**Request Tracking**:
- `totalRequests` - Total number of requests received (all stations)
- `lastStationId` - ID of the station that sent the most recent valid request
- `lastStationName` - Name of the station that sent the most recent valid request
- `lastRequestTimestamp` - Timestamp from the most recent valid request
- `lastRequestReceived` - System time (millis) when the request was received

**Monitoring**:
- Use `GET /status` to see which station last triggered the flamingo
- `last_request_age_ms` shows how long ago the last request was received
- `total_requests` tracks overall activity from all stations

### Color Mixing System

The flamingo features real-time color blending when multiple stations press buttons:

**How It Works**:
1. Each station button press sends a color name ("red", "green", "blue", "yellow", "white")
2. The flamingo converts color names to RGB values using `colorNameToRGB()`
3. Colors are added to an active colors list (max 10 concurrent colors)
4. Every frame, `getBlendedColor()` averages all active RGB values
5. The blended result is displayed on all 4 LED strips with a breathing effect

**Color Blending Examples**:
- Station 1: RED (255,0,0) + Station 2: BLUE (0,0,255) = PURPLE (127,0,127)
- Station 1: RED + Station 2: GREEN = YELLOW
- Station 1: RED + Station 2: BLUE + Station 3: GREEN = WHITE (or gray)
- Single station: RED = Pure RED

**Color Expiration**:
- Each color has a 5-second lifetime (configurable via `COLOR_TIMEOUT_MS`)
- Colors automatically expire after timeout
- When all colors expire, system returns to PLAN_IDLE (rainbow pattern)
- If a station presses a new color, its previous color is updated (one color per station)

**Supported Colors**:
- red, green, blue, yellow, white
- orange, purple, pink, cyan
- Unknown colors default to white

**Configuration**:
- `MAX_ACTIVE_COLORS` (10) - Maximum concurrent colors from different stations
- `COLOR_TIMEOUT_MS` (5000) - Color expiration time in milliseconds
- Defined in `src/main.cpp` lines 49-50

**Implementation Details**:
- `colorNameToRGB()` - Maps color strings to CRGB values
- `addActiveColor()` - Adds/updates color for a station
- `cleanupExpiredColors()` - Removes old colors, returns to IDLE if all expired
- `getBlendedColor()` - Averages RGB values of all active colors
- `playMixedColorsPattern()` - Displays blended color with breathing effect
