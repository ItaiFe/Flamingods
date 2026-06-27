#!/bin/bash

# Tasmota Device Discovery Script
# This script helps find Tasmota devices on your local network

set -e  # Exit on any error

echo "🔍 Tasmota Device Discovery"
echo "=========================="

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

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

# Get local network range
print_status "Detecting local network range..."

# Try to get the network range from the default route
NETWORK=$(route -n get default 2>/dev/null | grep interface | awk '{print $2}' | head -1)
if [ -z "$NETWORK" ]; then
    # Fallback method
    NETWORK=$(ifconfig | grep "inet " | grep -v 127.0.0.1 | head -1 | awk '{print $2}' | cut -d. -f1-3)
fi

if [ -z "$NETWORK" ]; then
    print_error "Could not detect network range. Please specify manually."
    echo "Usage: $0 <network_range>"
    echo "Example: $0 192.168.1"
    exit 1
fi

NETWORK_RANGE="${NETWORK}.0/24"
print_success "Detected network: $NETWORK_RANGE"

# Check if nmap is available
if command -v nmap >/dev/null 2>&1; then
    print_status "Using nmap to scan for Tasmota devices..."
    
    # Scan for devices with common Tasmota ports
    TASMOTA_DEVICES=$(nmap -p 80,8080,8081 --open -oG - $NETWORK_RANGE 2>/dev/null | grep -E "(80|8080|8081)/open" | awk '{print $2}' | sort -u)
    
    if [ -n "$TASMOTA_DEVICES" ]; then
        print_success "Found potential Tasmota devices:"
        for device in $TASMOTA_DEVICES; do
            echo "  - http://$device"
            echo "  - http://$device:8080"
            echo "  - http://$device:8081"
        done
    else
        print_warning "No devices found with common Tasmota ports"
    fi
else
    print_warning "nmap not available, using ping scan instead..."
fi

# Ping scan for active devices
print_status "Scanning for active devices on $NETWORK_RANGE..."

ACTIVE_DEVICES=()
for i in {1..254}; do
    IP="${NETWORK}.${i}"
    if ping -c 1 -W 1000 "$IP" >/dev/null 2>&1; then
        ACTIVE_DEVICES+=("$IP")
    fi
done

if [ ${#ACTIVE_DEVICES[@]} -eq 0 ]; then
    print_error "No active devices found on the network"
    exit 1
fi

print_success "Found ${#ACTIVE_DEVICES[@]} active devices"

# Check each active device for Tasmota
print_status "Checking active devices for Tasmota web interface..."

TASMOTA_FOUND=()
for device in "${ACTIVE_DEVICES[@]}"; do
    print_status "Checking $device..."
    
    # Check common Tasmota ports
    for port in 80 8080 8081; do
        if curl -s --connect-timeout 2 --max-time 5 "http://$device:$port" >/dev/null 2>&1; then
            # Try to get the page title to confirm it's Tasmota
            TITLE=$(curl -s --connect-timeout 2 --max-time 5 "http://$device:$port" 2>/dev/null | grep -i "<title>" | head -1 | sed 's/.*<title>\(.*\)<\/title>.*/\1/' | tr -d '\n\r')
            
            if [[ "$TITLE" == *"Tasmota"* ]] || [[ "$TITLE" == *"Sonoff"* ]] || [[ "$TITLE" == *"ESP"* ]]; then
                print_success "Found Tasmota device at http://$device:$port"
                echo "  Title: $TITLE"
                TASMOTA_FOUND+=("http://$device:$port")
            fi
        fi
    done
done

if [ ${#TASMOTA_FOUND[@]} -eq 0 ]; then
    print_warning "No Tasmota devices found, but here are the active devices:"
    for device in "${ACTIVE_DEVICES[@]}"; do
        echo "  - $device"
    done
    echo ""
    echo "You can manually check these devices by opening them in a browser:"
    for device in "${ACTIVE_DEVICES[@]}"; do
        echo "  - http://$device"
        echo "  - http://$device:8080"
        echo "  - http://$device:8081"
    done
else
    print_success "Found ${#TASMOTA_FOUND[@]} Tasmota device(s):"
    for device in "${TASMOTA_FOUND[@]}"; do
        echo "  - $device"
    done
    echo ""
    echo "You can now access your Tasmota device(s) using the URLs above"
    echo "Default credentials are usually admin/admin or admin/password"
fi

# Additional discovery methods
echo ""
print_status "Additional discovery methods:"

# Check for mDNS/Bonjour services
if command -v dns-sd >/dev/null 2>&1; then
    print_status "Checking for mDNS services..."
    dns-sd -B _http._tcp local. 2>/dev/null | grep -E "(Tasmota|Sonoff|ESP)" || true
fi

# Check ARP table for recently active devices
print_status "Recently active devices (from ARP table):"
arp -a | grep -E "(${NETWORK//./\.})" | awk '{print $2}' | sed 's/[()]//g' | sort -u

echo ""
print_success "Discovery complete!"
