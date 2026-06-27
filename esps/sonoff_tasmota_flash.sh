#!/bin/bash

# Sonoff S26R2 Tasmota Flashing Script
# This script helps flash Tasmota firmware on Sonoff S26R2 smart sockets
# 
# Prerequisites:
# - esptool.py installed (pip install esptool)
# - Tasmota firmware binary downloaded
# - Sonoff S26R2 in programming mode
# - FTDI cable or USB-to-Serial adapter

set -e  # Exit on any error

echo "🔌 Sonoff S26R2 Tasmota Flashing Script"
echo "========================================"

# Configuration
FIRMWARE_URL="https://github.com/arendst/Tasmota/releases/latest/download/tasmota.bin"
FIRMWARE_FILE="$HOME/Downloads/tasmota.bin"  # Look in Downloads folder first
FALLBACK_FIRMWARE="tasmota.bin"  # Fallback to current directory
SERIAL_PORT="/dev/cu.usbserial-A5069RR4"  # Adjust for your system
BAUD_RATE="115200"
FLASH_MODE="dout"
FLASH_SIZE="1MB"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Function to check if command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Check prerequisites
print_status "Checking prerequisites..."

if ! command_exists esptool.py; then
    print_error "esptool.py not found. Please install it with: pip install esptool"
    exit 1
fi

if ! command_exists curl; then
    print_error "curl not found. Please install curl to download firmware."
    exit 1
fi

print_success "Prerequisites check passed"

# Check if serial port exists
if [ ! -e "$SERIAL_PORT" ]; then
    print_error "Serial port $SERIAL_PORT not found"
    echo "Available serial ports:"
    ls /dev/cu.* 2>/dev/null || echo "No serial ports found"
    echo ""
    echo "Please update the SERIAL_PORT variable in this script with the correct port"
    exit 1
fi

print_success "Serial port $SERIAL_PORT found"

# Check for firmware file in Downloads first, then current directory
if [ -f "$FIRMWARE_FILE" ]; then
    print_success "Found firmware in Downloads: $FIRMWARE_FILE"
elif [ -f "$FALLBACK_FIRMWARE" ]; then
    print_success "Found firmware in current directory: $FALLBACK_FIRMWARE"
    FIRMWARE_FILE="$FALLBACK_FIRMWARE"
else
    print_status "No firmware found, downloading latest Tasmota firmware..."
    curl -L -o "$FIRMWARE_FILE" "$FIRMWARE_URL"
    
    if [ $? -ne 0 ]; then
        print_error "Failed to download Tasmota firmware"
        exit 1
    fi
    
    print_success "Tasmota firmware downloaded: $FIRMWARE_FILE"
fi

# Check firmware file size
FIRMWARE_SIZE=$(stat -f%z "$FIRMWARE_FILE" 2>/dev/null || stat -c%s "$FIRMWARE_FILE" 2>/dev/null)
if [ "$FIRMWARE_SIZE" -lt 100000 ]; then
    print_error "Firmware file seems too small ($FIRMWARE_SIZE bytes). Please check the download."
    exit 1
fi

print_success "Firmware file size: $FIRMWARE_SIZE bytes"

# Instructions for putting Sonoff in programming mode
echo ""
print_warning "IMPORTANT: Put your Sonoff S26R2 in programming mode:"
echo "1. Disconnect power from the Sonoff"
echo "2. Hold the button on the Sonoff"
echo "3. Connect power while holding the button"
echo "4. Keep holding the button for 3-5 seconds"
echo "5. Release the button"
echo "6. The LED should be solid (not blinking)"
echo ""
read -p "Press Enter when the Sonoff is in programming mode..."

# Test connection to Sonoff
print_status "Testing connection to Sonoff..."
if ! esptool.py --port "$SERIAL_PORT" --baud $BAUD_RATE chip_id > /dev/null 2>&1; then
    print_error "Cannot connect to Sonoff. Please check:"
    echo "- Sonoff is in programming mode"
    echo "- Serial port is correct: $SERIAL_PORT"
    echo "- FTDI cable is properly connected"
    echo "- Baud rate is correct: $BAUD_RATE"
    exit 1
fi

print_success "Connection to Sonoff established"

# Read chip info
print_status "Reading chip information..."
CHIP_INFO=$(esptool.py --port "$SERIAL_PORT" --baud $BAUD_RATE chip_id 2>/dev/null)
print_success "Chip ID: $CHIP_INFO"

# Erase flash
print_status "Erasing flash memory..."
esptool.py --port "$SERIAL_PORT" --baud $BAUD_RATE erase_flash

if [ $? -ne 0 ]; then
    print_error "Failed to erase flash"
    exit 1
fi

print_success "Flash memory erased"

# Flash Tasmota firmware
print_status "Flashing Tasmota firmware..."
esptool.py --port "$SERIAL_PORT" --baud $BAUD_RATE write_flash \
    --flash_mode $FLASH_MODE \
    --flash_size $FLASH_SIZE \
    0x00000 "$FIRMWARE_FILE"

if [ $? -ne 0 ]; then
    print_error "Failed to flash firmware"
    exit 1
fi

print_success "Tasmota firmware flashed successfully!"

# Verify flash
print_status "Verifying flash..."
esptool.py --port "$SERIAL_PORT" --baud $BAUD_RATE verify_flash \
    --flash_mode $FLASH_MODE \
    --flash_size $FLASH_SIZE \
    0x00000 "$FIRMWARE_FILE"

if [ $? -ne 0 ]; then
    print_warning "Flash verification failed, but firmware might still work"
else
    print_success "Flash verification passed"
fi

# Reset the device
print_status "Resetting Sonoff..."
esptool.py --port "$SERIAL_PORT" --baud $BAUD_RATE run

print_success "Sonoff S26R2 has been flashed with Tasmota!"
echo ""
echo "Next steps:"
echo "1. Disconnect the FTDI cable"
echo "2. Power cycle the Sonoff (unplug and plug back in)"
echo "3. Look for a WiFi network named 'sonoff-XXXX'"
echo "4. Connect to that network and configure Tasmota"
echo "5. Set up your WiFi credentials in the Tasmota web interface"
echo ""
echo "Default Tasmota web interface: http://192.168.4.1"
echo "Default credentials: admin/admin"
echo ""
print_success "Flashing completed successfully!"
