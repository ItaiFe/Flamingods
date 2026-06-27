#!/bin/bash

# ESP32 Test Flash Script
# This script builds and flashes the test firmware to help diagnose boot loop issues
# Supports both FTDI cable flashing and OTA updates

set -e  # Exit on any error

echo "🔧 ESP32 Test Flash Script"
echo "=========================="

# Check if we're in the right directory
if [ ! -f "platformio.ini" ]; then
    echo "❌ Error: Not in the test project directory"
    echo "Please run this script from esps/test/"
    exit 1
fi

# Check if PlatformIO is available
if ! command -v pio &> /dev/null; then
    echo "❌ Error: PlatformIO not found"
    echo "Please install PlatformIO CLI"
    exit 1
fi

# Parse command line arguments
MODE="ftdi"
SERIAL_PORT="/dev/cu.usbserial-A5069RR4"

while [[ $# -gt 0 ]]; do
    case $1 in
        --ota)
            MODE="ota"
            shift
            ;;
        --port)
            SERIAL_PORT="$2"
            shift 2
            ;;
        --help)
            echo "Usage: $0 [--ota] [--port PORT]"
            echo "  --ota    Use OTA update instead of FTDI cable"
            echo "  --port   Specify serial port for FTDI mode (default: /dev/cu.usbserial-A5069RR4)"
            echo "  --help   Show this help message"
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            echo "Use --help for usage information"
            exit 1
            ;;
    esac
done

echo "📡 Mode: $MODE"
if [ "$MODE" = "ftdi" ]; then
    echo "📡 Serial Port: $SERIAL_PORT"
fi
echo ""

# Check if serial port exists (only for FTDI mode)
if [ "$MODE" = "ftdi" ] && [ ! -e "$SERIAL_PORT" ]; then
    echo "❌ Error: Serial port $SERIAL_PORT not found"
    echo "Available ports:"
    ls /dev/cu.* 2>/dev/null || echo "No serial ports found"
    exit 1
fi

# Select environment based on mode
if [ "$MODE" = "ota" ]; then
    ENV="esp32dev-ota"
    echo "🔨 Building test firmware for OTA..."
else
    ENV="esp32dev"
    echo "🔨 Building test firmware for FTDI..."
fi

pio run -e $ENV

if [ $? -ne 0 ]; then
    echo "❌ Build failed"
    exit 1
fi

echo "✅ Build successful"
echo ""

if [ "$MODE" = "ota" ]; then
    echo "📤 Uploading firmware via OTA..."
    echo "🔍 Looking for ESP32 at test-esp32.local..."
    
    # Check if ESP32 is reachable
    if ping -c 1 test-esp32.local > /dev/null 2>&1; then
        echo "✅ ESP32 found at test-esp32.local"
        pio run -e $ENV --target upload
        
        if [ $? -ne 0 ]; then
            echo "❌ OTA upload failed"
            exit 1
        fi
        
        echo "✅ OTA upload successful"
        echo ""
        echo "📡 ESP32 should be running the new firmware"
        echo "Check the serial monitor to see the output"
    else
        echo "❌ ESP32 not found at test-esp32.local"
        echo "Make sure the ESP32 is connected to WiFi and running"
        echo "You may need to flash via FTDI first to set up WiFi"
        exit 1
    fi
else
    echo "📤 Uploading firmware to ESP32 via FTDI..."
    pio run -e $ENV --target upload --upload-port "$SERIAL_PORT"

    if [ $? -ne 0 ]; then
        echo "❌ Upload failed"
        echo ""
        echo "🔄 Trying alternative upload method..."
        esptool --chip esp32 --port "$SERIAL_PORT" --baud 460800 --before default-reset --after hard-reset write-flash --flash-mode qio --flash-freq 80m --flash-size 4MB 0x1000 .pio/build/$ENV/firmware.bin
        
        if [ $? -ne 0 ]; then
            echo "❌ Alternative upload also failed"
            exit 1
        fi
    fi

    echo "✅ Upload successful"
    echo ""

    echo "📡 Starting serial monitor..."
    echo "Press Ctrl+C to exit"
    echo "================================"
    echo ""

    # Wait a moment for the ESP32 to boot
    sleep 2

    # Start serial monitor
    pio device monitor --port "$SERIAL_PORT" --baud 115200
fi
