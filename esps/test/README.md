# ESP32 Test Firmware

A comprehensive test firmware designed to verify FTDI flashing, OTA updates, and basic ESP32 functionality. This project helps diagnose boot loop issues and verifies that the ESP32 hardware is working correctly.

## Features

- **Minimal Code**: Simple, lightweight firmware with minimal external dependencies
- **Serial Output**: Clear status messages to verify the ESP32 is working
- **LED Blinking**: Built-in LED blinks every 2 seconds to show activity
- **System Info**: Displays chip information, memory usage, and uptime
- **Dual Network Support**: Ethernet (primary) and WiFi (fallback) connectivity
- **OTA Updates**: Over-the-air firmware updates via ArduinoOTA
- **Dual Mode**: Supports both FTDI cable and OTA flashing

## Hardware Requirements

- ESP32 development board
- FTDI cable for initial programming
- Ethernet shield/module (W5500 or similar) for Ethernet connectivity
- WiFi network access for OTA updates (fallback)
- USB power supply

## Building and Flashing

### Prerequisites
- PlatformIO CLI installed
- FTDI cable connected to ESP32 (for initial setup)
- Ethernet shield connected to ESP32 (CS pin 5)
- WiFi network with SSID "DiMax Residency 2.4Ghz" and password "33355555DM" (fallback)

### Quick Start
```bash
# Navigate to the test directory
cd esps/test

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

# Upload via OTA (ESP32 must be connected to network)
pio run -e esp32dev-ota --target upload
```

## Network Configuration

### Ethernet Settings
The firmware is configured to use Ethernet as the primary connection method:
- **MAC Address**: `DE:AD:BE:EF:FE:ED`
- **Static IP**: `192.168.1.100` (if DHCP fails)
- **Gateway**: `192.168.1.1`
- **Subnet**: `255.255.255.0`
- **DNS**: `8.8.8.8`
- **CS Pin**: GPIO 5

### WiFi Settings (Fallback)
If Ethernet connection fails, the firmware will attempt WiFi connection:
- **SSID**: `DiMax Residency 2.4Ghz`
- **Password**: `33355555DM`

### Manual Upload (if PlatformIO fails)
```bash
# Build first
pio run -e esp32dev

# Upload using esptool directly
esptool --chip esp32 --port /dev/cu.usbserial-A5069RR4 --baud 460800 --before default-reset --after hard-reset write-flash --flash-mode qio --flash-freq 80m --flash-size 4MB 0x1000 .pio/build/esp32dev/firmware.bin
```

## Expected Output

### Ethernet Connection (Primary)
If the ESP32 is working correctly with Ethernet, you should see:

```
========================================
    ESP32 Test Firmware
========================================
Firmware Version: 1.0.0
Chip ID: 4C3F820C
Free heap: 327680 bytes
CPU Frequency: 240 MHz
========================================
If you see this message, the ESP32 is working!
========================================
Built-in LED initialized
Initializing Ethernet...
Ethernet connected!
IP address: 192.168.1.100
Initializing OTA...
OTA ready
Starting main loop...

[2] Counter: 1, LED: ON, Free heap: 327680 bytes, Ethernet: 192.168.1.100
[4] Counter: 2, LED: OFF, Free heap: 327680 bytes, Ethernet: 192.168.1.100
[6] Counter: 3, LED: ON, Free heap: 327680 bytes, Ethernet: 192.168.1.100
...
```

### WiFi Connection (Fallback)
If Ethernet fails and WiFi is used as fallback:

```
========================================
    ESP32 Test Firmware
========================================
Firmware Version: 1.0.0
Chip ID: 4C3F820C
Free heap: 327680 bytes
CPU Frequency: 240 MHz
========================================
If you see this message, the ESP32 is working!
========================================
Built-in LED initialized
Initializing Ethernet...
Ethernet connection failed, trying WiFi...
..........
WiFi connected!
IP address: 192.168.1.234
Initializing OTA...
OTA ready
Starting main loop...

[2] Counter: 1, LED: ON, Free heap: 327680 bytes, WiFi: 192.168.1.234
[4] Counter: 2, LED: OFF, Free heap: 327680 bytes, WiFi: 192.168.1.234
[6] Counter: 3, LED: ON, Free heap: 327680 bytes, WiFi: 192.168.1.234
...
```

### OTA Update Process

When updating via OTA, you'll see:
```
OTA Start updating sketch
OTA Progress: 25%
OTA Progress: 50%
OTA Progress: 75%
OTA Progress: 100%
OTA End
```

## Troubleshooting

### If you see boot loop messages:
```
rst:0x10 (RTCWDT_RTC_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
load:0x3f400020,len:134256
ets Jul 29 2019 12:21:46
```

This indicates a hardware or power issue:
1. **Check power supply** - Ensure stable 3.3V power
2. **Check connections** - Verify all pins are properly connected
3. **Try different ESP32** - Test with a known working board
4. **Check FTDI cable** - Verify cable is working with another device

### If you see no output:
1. **Check serial port** - Verify correct port (e.g., `/dev/cu.usbserial-A5069RR4`)
2. **Check baud rate** - Should be 115200
3. **Check connections** - TX/RX pins properly connected
4. **Try different cable** - FTDI cable might be faulty

### If upload fails:
1. **Check boot mode** - ESP32 needs to be in download mode
2. **Check connections** - All required pins connected
3. **Try different baud rate** - Use 115200 instead of 460800
4. **Check power** - Ensure stable power during upload

## Success Criteria

The test is successful if:
- ✅ Firmware uploads without errors
- ✅ Serial output shows the startup message
- ✅ LED blinks every 2 seconds
- ✅ Counter increments continuously
- ✅ No boot loops or resets

## Next Steps

If this test firmware works:
1. The ESP32 hardware is functional
2. The FTDI cable is working
3. The issue is likely in the main firmware code

If this test firmware fails:
1. There's a hardware or power issue
2. Need to check ESP32 board and connections
3. Need to verify FTDI cable functionality
