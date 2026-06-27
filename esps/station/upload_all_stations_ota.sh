#!/bin/bash

# OTA Upload Script for All Stations
# This script builds and uploads firmware to all 4 stations via OTA

set -e  # Exit on error

# Configuration
OTA_PASSWORD="flamingods2024"
export ESP_OTA_PASSWORD=$OTA_PASSWORD

# Station IP addresses (more reliable than mDNS hostnames)
STATION_1="192.168.1.235"
STATION_2="station-2.local"  # IP not provided, using hostname
STATION_3="192.168.1.154"
STATION_4="192.168.1.228"

# Function to get station host
get_station_host() {
    case $1 in
        1) echo "$STATION_1" ;;
        2) echo "$STATION_2" ;;
        3) echo "$STATION_3" ;;
        4) echo "$STATION_4" ;;
        *) echo "" ;;
    esac
}

# Colors for output
GREEN='\033[0;32m'
BLUE='\033[0;34m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${BLUE}╔════════════════════════════════════════╗${NC}"
echo -e "${BLUE}║  Station OTA Upload Script             ║${NC}"
echo -e "${BLUE}║  Updating All 4 Stations               ║${NC}"
echo -e "${BLUE}╔════════════════════════════════════════╗${NC}"
echo ""

# Function to upload to a specific station
upload_station() {
    local station_num=$1
    local station_host=$2

    echo -e "${YELLOW}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    echo -e "${BLUE}Station $station_num${NC}: Preparing upload to ${station_host}"
    echo -e "${YELLOW}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"

    # Step 1: Copy correct station config for this station
    echo -e "${GREEN}[1/4]${NC} Copying station_${station_num}_config.h to station_config.h..."
    if [ ! -f "station_config_examples/station_${station_num}_config.h" ]; then
        echo -e "${RED}✗${NC} Config file station_config_examples/station_${station_num}_config.h not found!"
        return 1
    fi

    cp "station_config_examples/station_${station_num}_config.h" station_config.h
    # Force update timestamp to ensure compiler sees the change
    touch station_config.h

    if [ $? -eq 0 ]; then
        echo -e "${GREEN}✓${NC} Config copied: Station ${station_num} ($(grep 'STATION_NAME' station_config.h | cut -d'"' -f2))"
    else
        echo -e "${RED}✗${NC} Failed to copy config"
        return 1
    fi

    # Step 2: Clean previous build and remove cached headers
    echo -e "${GREEN}[2/4]${NC} Cleaning previous build and cache..."
    pio run -t clean
    # Force remove build cache to ensure config changes are picked up
    rm -rf .pio/build/*/
    echo -e "${GREEN}✓${NC} Build cache cleared"

    # Step 3: Build firmware with correct config
    echo -e "${GREEN}[3/4]${NC} Building firmware for Station $station_num..."
    pio run -e esp32dev

    if [ $? -eq 0 ]; then
        echo -e "${GREEN}✓${NC} Build successful for Station $station_num config"
    else
        echo -e "${RED}✗${NC} Build failed"
        return 1
    fi

    # Step 4: Upload via OTA (with timeout)
    echo -e "${GREEN}[4/4]${NC} Uploading to $station_host via OTA..."
    echo -e "${YELLOW}Checking if station is reachable...${NC}"

    # Ping test to check if station is reachable (1 second timeout)
    if ping -c 1 -W 1 "$station_host" > /dev/null 2>&1; then
        echo -e "${GREEN}✓${NC} Station is reachable"

        # Upload via OTA (use gtimeout if available, otherwise no timeout)
        if command -v gtimeout &> /dev/null; then
            # macOS with coreutils installed
            gtimeout 120 pio run -e esp32dev-ota --target upload --upload-port "$station_host"
        elif command -v timeout &> /dev/null; then
            # Linux or macOS with timeout installed
            timeout 120 pio run -e esp32dev-ota --target upload --upload-port "$station_host"
        else
            # No timeout command available - run without timeout
            pio run -e esp32dev-ota --target upload --upload-port "$station_host"
        fi

        if [ $? -eq 0 ]; then
            echo -e "${GREEN}✓${NC} Upload successful!"
            echo -e "${GREEN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
            echo -e "${GREEN}✓ Station $station_num updated successfully!${NC}"
            echo -e "${GREEN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
            return 0
        elif [ $? -eq 124 ]; then
            echo -e "${YELLOW}⚠${NC} Upload timed out after 120 seconds"
            echo -e "${YELLOW}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
            echo -e "${YELLOW}⚠ Station $station_num upload timed out - skipping${NC}"
            echo -e "${YELLOW}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
            return 1
        else
            echo -e "${RED}✗${NC} Upload failed"
            echo -e "${RED}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
            echo -e "${RED}✗ Station $station_num update failed - skipping${NC}"
            echo -e "${RED}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
            return 1
        fi
    else
        echo -e "${YELLOW}⚠${NC} Station is not reachable (offline or not on network)"
        echo -e "${YELLOW}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
        echo -e "${YELLOW}⚠ Station $station_num is offline - skipping${NC}"
        echo -e "${YELLOW}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
        return 1
    fi
}

# Main execution
SUCCESS_COUNT=0
FAIL_COUNT=0

# Option to upload to specific station or all
if [ "$1" != "" ]; then
    # Upload to specific station
    station_num=$1
    station_host=$(get_station_host $station_num)
    if [ -n "$station_host" ]; then
        echo -e "${BLUE}Uploading to Station $station_num only...${NC}"
        echo ""
        upload_station $station_num $station_host
        if [ $? -eq 0 ]; then
            SUCCESS_COUNT=1
        else
            FAIL_COUNT=1
        fi
    else
        echo -e "${RED}Error: Invalid station number. Use 1-4${NC}"
        exit 1
    fi
else
    # Upload to all stations
    echo -e "${BLUE}Uploading to all stations...${NC}"
    echo ""

    for station_num in 1 2 3 4; do
        station_host=$(get_station_host $station_num)
        upload_station $station_num $station_host
        if [ $? -eq 0 ]; then
            SUCCESS_COUNT=$((SUCCESS_COUNT + 1))
        else
            FAIL_COUNT=$((FAIL_COUNT + 1))
        fi
        echo ""
        sleep 2  # Brief pause between uploads
    done
fi

# Summary
echo ""
echo -e "${BLUE}╔════════════════════════════════════════╗${NC}"
echo -e "${BLUE}║           Upload Summary               ║${NC}"
echo -e "${BLUE}╔════════════════════════════════════════╗${NC}"
echo -e "${GREEN}  Successful: $SUCCESS_COUNT${NC}"
echo -e "${RED}  Failed/Skipped: $FAIL_COUNT${NC}"
echo -e "${BLUE}╚════════════════════════════════════════╝${NC}"

if [ $FAIL_COUNT -eq 0 ]; then
    echo -e "${GREEN}All stations updated successfully! 🎉${NC}"
    exit 0
else
    echo -e "${YELLOW}Some stations were skipped (offline) or failed. Check the output above.${NC}"
    echo -e "${YELLOW}Offline stations will be updated when they come back online.${NC}"
    exit 1
fi
