# Crown ESP32 - LED Lighting Controller

A WiFi-enabled ESP32 device that controls 1 LED strip with different lighting plans triggered via HTTP endpoints. Designed for the Flamingods crown art installation project.

## Features

### 3 LED Lighting Plans
1. **IDLE Plan** - Soft yellow pulsating halo mode for ambient lighting
2. **BUTTON Plan** - Crazy colors party mode with dynamic patterns and effects
3. **WiFi Fallback Plan** - Halo mode + running colorful pixels when WiFi is unavailable

### HTTP API Endpoints
- `POST /idle` - Switch to idle lighting plan (soft yellow pulsating halo)
- `POST /button` - Switch to button lighting plan (crazy colors party mode)
- `GET /status` - Get current status and system information
- `GET /health` - Health check endpoint

### Hardware
- **Controller**: ESP32 development board
- **LEDs**: 1x WS2812B LED strip (100 LEDs)
- **Power**: 5V power supply for LED strip
- **Connectivity**: WiFi for HTTP control

## Pin Configuration

| Pin | Function | LED Strip |
|-----|----------|-----------|
| 4   | LED Strip Data | Single WS2812B strip |

## Network Configuration

The ESP32 connects to the local WiFi network:
- **SSID**: DiMax Residency 2.4Ghz
- **Password**: 33355555DM
- **IP**: Automatically assigned via DHCP
- **Port**: 80 (HTTP)

## Building and Flashing

### Prerequisites
- PlatformIO IDE or CLI
- ESP32 development board
- USB-C cable for programming

### Build Commands
```bash
# Navigate to the crown directory
cd esps/crown

# Build the project
pio run

# Upload to ESP32
pio run --target upload

# Monitor serial output
pio device monitor
```

### Dependencies
- FastLED library (v3.5.0+)
- Arduino_JSON library (v6.21.0+)
- ESP32 Arduino framework

## Usage Examples

### Switch to Idle Mode (Halo)
```bash
curl -X POST http://[ESP32_IP]/idle
```

### Switch to Button Mode (Party)
```bash
curl -X POST http://[ESP32_IP]/button
```

### Check Status
```bash
curl http://[ESP32_IP]/status
```

### Health Check
```bash
curl http://[ESP32_IP]/health
```

## LED Plan Details

### IDLE Plan - Halo Mode
- **Purpose**: Ambient background lighting
- **Pattern**: Soft yellow/amber pulsating halo effect
- **Colors**: Warm yellow tones (RGB: brightness, brightness*0.7, brightness*0.3)
- **Speed**: Slow, gentle pulse for calming effect
- **Brightness**: Low to medium (50-100)
- **Auto-return**: Stays in this mode until changed

### BUTTON Plan - Party Mode
- **Purpose**: High-energy, dynamic lighting
- **Pattern**: 4 different party patterns that cycle every second:
  1. **Rainbow Wave**: Smooth rainbow colors flowing across the strip
  2. **Color Explosion**: Random bright colors with black spaces
  3. **Alternating Colors**: Alternating bright colors across the strip
  4. **Sparkle Effect**: Black background with random colorful sparkles
- **Speed**: Fast color cycling and pattern changes
- **Brightness**: Full brightness (255)
- **Auto-return**: Automatically returns to IDLE after 10 seconds
- **Effects**: Includes glitter effects for extra party feel

### WiFi Fallback Plan
- **Purpose**: Visual interest when WiFi is unavailable
- **Pattern**: Combines halo background with running colorful pixels
- **Background**: Same soft yellow pulsating halo as IDLE mode
- **Foreground**: 3 colorful running pixels with fade trails
- **Colors**: Various bright colors cycling through the running pixels
- **Auto-activation**: Automatically activates when WiFi disconnects
- **Auto-return**: Automatically returns to IDLE when WiFi reconnects

## WiFi Behavior

- **WiFi Connected**: Crown operates in normal IDLE mode
- **WiFi Disconnected**: Automatically switches to WiFi Fallback mode
- **WiFi Reconnected**: Automatically returns to IDLE mode
- **Button Press**: Overrides WiFi status and activates party mode
- **Recovery**: Continuous WiFi reconnection attempts in background

## Testing

Use the included test script to verify all endpoints:

```bash
# Update ESP_IP in test_endpoints.py with your device's IP
python3 test_endpoints.py
```

The test script will:
1. Check device status
2. Test idle mode (halo)
3. Test button mode (party)
4. Return to idle mode
5. Verify final status

## Troubleshooting

### Common Issues
- **WiFi Connection Failed**: Crown will automatically enter WiFi Fallback mode
- **LEDs Not Responding**: Check power supply and data pin connection
- **Patterns Too Fast/Slow**: Adjust timing values in `led_plans.cpp`

### Serial Monitor
Connect via USB and monitor serial output at 115200 baud for debugging information.
