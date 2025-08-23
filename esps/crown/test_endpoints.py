#!/usr/bin/env python3
"""
Test script for Crown ESP32 LED endpoints
Tests the idle, button, and status endpoints
"""

import requests
import time
import json

# Configuration
ESP_IP = "192.168.1.100"  # Update this to your ESP32's IP address
BASE_URL = f"http://{ESP_IP}"

def test_endpoint(endpoint, method="POST"):
    """Test a specific endpoint"""
    url = f"{BASE_URL}{endpoint}"
    
    try:
        if method == "POST":
            response = requests.post(url, timeout=5)
        else:
            response = requests.get(url, timeout=5)
            
        print(f"{method} {endpoint}: {response.status_code}")
        if response.status_code == 200:
            try:
                data = response.json()
                print(f"  Response: {json.dumps(data, indent=2)}")
            except:
                print(f"  Response: {response.text}")
        return response.status_code == 200
    except requests.exceptions.RequestException as e:
        print(f"  Error: {e}")
        return False

def main():
    """Main test sequence"""
    print("=== Crown ESP32 LED Endpoint Test ===\n")
    
    # Test status first
    print("1. Testing status endpoint...")
    test_endpoint("/status", "GET")
    print()
    
    # Test idle mode
    print("2. Testing idle endpoint (halo mode)...")
    test_endpoint("/idle")
    print("   Crown should now show soft yellow pulsating halo")
    print()
    
    # Wait a bit to see the effect
    time.sleep(3)
    
    # Test button mode (party mode)
    print("3. Testing button endpoint (party mode)...")
    test_endpoint("/button")
    print("   Crown should now show crazy colors party mode")
    print()
    
    # Wait to see party mode
    time.sleep(5)
    
    # Return to idle
    print("4. Returning to idle mode...")
    test_endpoint("/idle")
    print("   Crown should return to soft yellow pulsating halo")
    print()
    
    # Final status check
    print("5. Final status check...")
    test_endpoint("/status", "GET")
    print()
    
    print("Test complete!")

if __name__ == "__main__":
    main()
