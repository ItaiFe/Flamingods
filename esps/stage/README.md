# ESP32 FastLED Project

This is an ESP32 project using the FastLED library to control addressable LED strips.

## Features

- **Button Control**: Single, double, multi-press, and long-press detection
- **LED Effects**: Multiple color patterns based on button presses
- **WiFi Connectivity**: HTTP requests to webhooks
- **OTA Updates**: Over-the-air firmware updates via WiFi
- **Visual Feedback**: LED patterns show OTA progress and status

## Hardware Requirements

- ESP32 development board
- WS2812B LED strip (or compatible)
- Power supply (5V for most LED strips)
- Jumper wires

## Wiring

Connect your LED strip to the ESP32:
- LED Data Pin → GPIO 2 (configurable in code)
- LED VCC → 5V (or appropriate voltage for your strip)
- LED GND → GND

## Software Requirements

- PlatformIO IDE (recommended) or Arduino IDE
- FastLED library

## Installation

### Using PlatformIO (Recommended)

1. Install PlatformIO IDE
2. Open this project folder
3. Build and upload to your ESP32

### Using Arduino IDE

1. Install ESP32 board support
2. Install FastLED library from Library Manager
3. Copy the code from `src/main.cpp`
4. Select your ESP32 board and upload

## Configuration

Edit `src/main.cpp` to configure:

- `LED_PIN`: GPIO pin connected to LED data line (default: 2)
- `NUM_LEDS`: Number of LEDs in your strip (default: 50)
- `LED_TYPE`: Type of LED strip (default: WS2812B)
- `COLOR_ORDER`: Color order for your LEDs (default: GRB)

## Usage

1. Connect your ESP32 to your computer
2. Upload the code
3. Connect the LED strip according to the wiring diagram
4. Power on and enjoy the light show!

## OTA (Over-The-Air) Updates

This project includes OTA functionality, allowing you to update the firmware wirelessly:

### First Time Setup
1. Flash the code normally via USB (this enables OTA)
2. The ESP32 will connect to WiFi and enable OTA
3. Note the IP address shown in serial monitor

### Updating Firmware Wirelessly

#### Method 1: Using PlatformIO CLI
```bash
# Build and upload via OTA
pio run --target upload --upload-port ota://<ESP32_IP_ADDRESS>
```

#### Method 2: Using the Python Script
```bash
# Auto-detect ESP32 IP
python upload_ota.py

# Or specify IP manually
python upload_ota.py 192.168.1.100
```

#### Method 3: Using Arduino IDE
1. Go to Tools → Port → Network Ports
2. Select your ESP32's IP address
3. Upload as normal

### OTA Security
- Hostname: `ESP32-Flamingo-Button`
- Password: `flamingo123` (change in code if needed)
- Port: 3232 (default)

### Visual OTA Feedback
- **Yellow LEDs**: OTA update in progress
- **Green LEDs**: Update successful
- **Red LEDs**: Update failed
- **Brightness**: Shows upload progress

## Effects Included

- **Rainbow Wave**: Continuously scrolling rainbow effect
- **Breathing**: Pulsing blue light effect
- **Color Wipe**: Sequential color filling
- **Twinkle**: Random twinkling stars effect

To switch between effects, uncomment the desired effect function in the `loop()` function and comment out the others.

## Troubleshooting

- If LEDs don't light up, check wiring and power supply
- If colors are wrong, try changing `COLOR_ORDER` (GRB, RGB, BGR, etc.)
- If flickering occurs, add a capacitor (1000µF) across the power supply
- Ensure adequate power supply for your LED count (60mA per LED for full brightness)

## Power Requirements

- Each LED can draw up to 60mA at full brightness
- For 50 LEDs: up to 3A at full brightness
- Use appropriate power supply and consider external power for large strips
