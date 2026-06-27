# Flamingo ESP32 LED Pattern Controller

A simple ESP32-based LED pattern controller for WS2812B LED strips. This project provides clean, focused LED pattern functionality without complex game logic or network features.

## Features

- **Simple LED Patterns**: Clean, efficient LED animations
- **4 LED Strips**: Control up to 4 separate LED strips independently
- **FastLED Library**: Built on the reliable FastLED library for smooth animations
- **PlatformIO**: Modern development environment with easy build and upload

## Hardware Requirements

- ESP32 development board
- 4x WS2812B LED strips (100 LEDs each recommended)
- 5V power supply (8A recommended for full brightness)
- Connecting wires

## Pin Configuration

| LED Strip | GPIO Pin | Color | Notes |
|-----------|----------|-------|-------|
| Red       | 2        | Red   | Connected to green's pin |
| Green     | 4        | Green | Connected to red's pin |
| Blue      | 5        | Blue  | Standard connection |
| Yellow    | 18       | Yellow| Standard connection |

**Note**: Red and Green are swapped due to pin configuration.

## LED Patterns

### 1. Moving Pattern (Default)
- Simple blue dot that moves across all LED strips
- Same pattern displayed on pins 2, 4, and 5
- Updates every 100ms for smooth animation
- Automatically loops from start to end

### 2. Idle Pattern
- Flowing rainbow pattern across all LED strips
- Dynamic wave motion with varying brightness
- Smooth color transitions
- Updates every 20ms for fluid animation

## Project Structure

```
esps/flamingo/
├── src/
│   ├── main.cpp              # Main Arduino sketch
│   └── led_plans.cpp         # LED pattern implementations
├── include/
│   └── led_plans.h           # LED pattern header
├── platformio.ini            # PlatformIO configuration
├── wiring_diagram.txt        # Hardware connection guide
├── README.md                 # This file
├── requirements.txt          # Python dependencies
├── test_patterns.py          # Test script
├── Makefile                  # Build automation
├── lib/                      # Library files
├── .pio/                     # PlatformIO build files
└── .venv/                    # Python virtual environment
```

## Quick Start

### 1. Install Dependencies
```bash
# Install PlatformIO
pip install platformio

# Install Python dependencies
pip install -r requirements.txt
```

### 2. Build and Upload
```bash
# Build the project
make build

# Upload to ESP32
make upload

# Monitor serial output
make monitor
```

### 3. Test Patterns
```bash
# Test LED patterns (replace PORT with your serial port)
python test_patterns.py /dev/ttyUSB0  # Linux/Mac
python test_patterns.py COM3           # Windows
```

## Development

### Building
```bash
make build          # Build the project
make clean          # Clean build files
```

### Uploading
```bash
make upload         # Build and upload
make deploy         # Same as upload
```

### Monitoring
```bash
make monitor        # Monitor serial output
make deploy-monitor # Build, upload, and monitor
```

## Configuration

### LED Settings
- `NUM_LEDS_PER_STRIP`: Number of LEDs per strip (default: 100)
- `BRIGHTNESS`: LED brightness 0-255 (default: 100)
- `IDLE_UPDATE_INTERVAL`: Idle animation update rate (default: 20ms)
- `MOVING_PATTERN_UPDATE_INTERVAL`: Moving pattern update rate (default: 100ms)

### Pin Configuration
Modify the pin definitions in `include/led_plans.h`:
```cpp
#define LED_RED_PIN    4
#define LED_GREEN_PIN  2
#define LED_BLUE_PIN   5
#define LED_YELLOW_PIN 18
```

## Troubleshooting

### Common Issues

1. **LEDs not lighting up**
   - Check power supply (5V, sufficient current)
   - Verify pin connections
   - Check LED strip orientation (data flow direction)

2. **Patterns not working**
   - Ensure FastLED library is installed
   - Check serial monitor for error messages
   - Verify LED strip type (WS2812B)

3. **Upload failures**
   - Check USB connection
   - Verify correct board selection
   - Try pressing reset button on ESP32

### Debug Information
Enable debug output by checking the serial monitor:
```bash
make monitor
```

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test thoroughly
5. Submit a pull request

## License

This project is open source. Feel free to modify and distribute.

## Support

For issues and questions:
1. Check the troubleshooting section
2. Review the wiring diagram
3. Check serial monitor output
4. Open an issue with detailed information 