# Stage ESP32 - LED Lighting Controller

A WiFi-enabled ESP32 device that controls 1 LED strip with different lighting plans triggered via HTTP endpoints. Designed for the Midburn art installation project.

## Features

### 4 LED Lighting Plans
1. **IDLE Plan** - Subtle, ambient lighting with slow color transitions and gentle waves
2. **SKIP Plan** - Quick, bright flashes for transitions between scenes
3. **SHOW Plan** - Dynamic, performance-focused patterns with multiple sub-patterns
4. **SPECIAL Plan** - Unique effects including fire, aurora, and matrix patterns

### HTTP API Endpoints
- `POST /idle` - Switch to idle lighting plan
- `POST /skip` - Switch to skip lighting plan  
- `POST /show` - Switch to show lighting plan
- `POST /special` - Switch to special lighting plan
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
- **SSID**: Flamingods
- **Password**: Aa123456!
- **IP**: Automatically assigned via DHCP
- **Port**: 80 (HTTP)

## Building and Flashing

### Prerequisites
- PlatformIO IDE or CLI
- ESP32 development board
- USB-C cable for programming

### Build Commands
```bash
# Navigate to the stage directory
cd esps/stage

# Build the project
pio run

# Upload to ESP32
pio run --target upload

# Monitor serial output
pio device monitor
```

### Dependencies
- FastLED library (v3.5.0+)
- Arduino_JSON library (v0.1.0+)
- ESP32 Arduino framework

## Usage Examples

### Switch to Show Mode
```bash
curl -X POST http://[ESP32_IP]/show
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

### IDLE Plan
- **Purpose**: Ambient background lighting
- **Pattern**: Gentle color waves with subtle glitter across the LED strip
- **Speed**: Slow, relaxing transitions
- **Brightness**: Low to medium (50-100)

### SKIP Plan
- **Purpose**: Quick scene transitions
- **Pattern**: Bright white flash → fade → color flash → fade across the strip
- **Duration**: ~400ms total
- **Auto-return**: Automatically returns to IDLE after completion

### SHOW Plan
- **Purpose**: Main performance lighting
- **Patterns**: 
  - Rainbow wave across the LED strip
  - Pulsing circles from center of strip
  - Running lights animation along the strip
  - Strobe effect on entire strip
- **Rotation**: Automatically cycles through patterns every 5 seconds
- **Brightness**: High (200-255)

### SPECIAL Plan
- **Purpose**: Special effects and unique lighting
- **Effects**:
  - Fire effect with heat colors across the strip
  - Aurora borealis simulation along the strip
  - Matrix-style green rain effect
- **Rotation**: Changes effects every 5 seconds
- **Brightness**: Variable based on effect

## Integration

### Main Control Application
The stage ESP integrates with the main control system via HTTP API calls. The main application can:
- Trigger lighting plan changes
- Monitor current status
- Check system health
- Coordinate with other ESP devices

### Raspberry Pi Hub
Communicates with the Raspberry Pi control hub for:
- Centralized lighting control
- Performance coordination
- System monitoring

## Troubleshooting

### Common Issues

1. **WiFi Connection Failed**
   - Check SSID and password in main.cpp
   - Verify network availability
   - Check signal strength

2. **LEDs Not Responding**
   - Verify power supply (5V, sufficient current - 8A recommended)
   - Check pin connection to GPIO 4
   - Confirm LED strip type (WS2812B)
   - Verify single LED strip connection

3. **HTTP Endpoints Not Responding**
   - Check WiFi connection
   - Verify IP address assignment
   - Check serial monitor for error messages

4. **LED Patterns Not Working**
   - Verify single LED strip connection
   - Check GPIO 4 connection
   - Ensure proper power supply capacity
   - Check for loose connections

### Debug Information
The ESP32 outputs detailed debug information via serial monitor:
- WiFi connection status
- HTTP request handling
- LED plan changes
- System status updates

## Development

### Adding New Lighting Plans
1. Add new plan to `LightingPlan` enum in `led_plans.h`
2. Implement update function in `led_plans.cpp`
3. Add HTTP handler in `main.cpp`
4. Update documentation

### Customizing LED Patterns
- Modify pattern algorithms in the respective `update*()` functions
- Adjust timing and color parameters
- Add new effects using FastLED functions
- All patterns work on the single LED strip

### Performance Optimization
- Monitor memory usage with LED count
- Optimize animation timing for smooth performance
- Use efficient FastLED functions for complex patterns
- Single strip reduces complexity and improves performance

## License

This project is part of the Midburn art installation and follows the project's licensing terms.

## Support

For technical support or questions about the stage ESP:
- Check the serial monitor output
- Review the main project documentation
- Contact the development team
