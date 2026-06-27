#!/usr/bin/env python3
"""
Test script for Flamingo ESP32 LED Pattern Controller
Tests the LED patterns and basic functionality
"""

import time
import serial
import argparse

def test_serial_connection(port, baudrate=115200):
    """Test basic serial connection to the ESP32"""
    try:
        ser = serial.Serial(port, baudrate, timeout=5)
        time.sleep(2)  # Wait for ESP32 to boot
        
        print(f"✅ Connected to ESP32 on {port}")
        print("📡 Testing serial communication...")
        
        # Send a test command or just read what's available
        if ser.in_waiting:
            data = ser.readline().decode('utf-8').strip()
            print(f"📥 Received: {data}")
        
        ser.close()
        return True
        
    except serial.SerialException as e:
        print(f"❌ Failed to connect to ESP32: {e}")
        return False
    except Exception as e:
        print(f"❌ Unexpected error: {e}")
        return False

def test_led_patterns(port, baudrate=115200):
    """Test LED pattern functionality"""
    try:
        ser = serial.Serial(port, baudrate, timeout=5)
        time.sleep(2)  # Wait for ESP32 to boot
        
        print("🎨 Testing LED patterns...")
        print("📱 The ESP32 should be running the moving pattern on pins 2, 4, 5")
        print("⏱️  Waiting 10 seconds to observe patterns...")
        
        # Monitor serial output for pattern information
        start_time = time.time()
        while time.time() - start_time < 10:
            if ser.in_waiting:
                data = ser.readline().decode('utf-8').strip()
                if data:
                    print(f"📥 {data}")
            time.sleep(0.1)
        
        ser.close()
        print("✅ LED pattern test completed")
        return True
        
    except Exception as e:
        print(f"❌ LED pattern test failed: {e}")
        return False

def main():
    parser = argparse.ArgumentParser(description='Test Flamingo ESP32 LED Patterns')
    parser.add_argument('port', help='Serial port (e.g., /dev/ttyUSB0, COM3)')
    parser.add_argument('--baudrate', type=int, default=115200, help='Baud rate (default: 115200)')
    parser.add_argument('--test', choices=['serial', 'patterns', 'all'], default='all', 
                       help='Test type (default: all)')
    
    args = parser.parse_args()
    
    print("🚀 Flamingo ESP32 LED Pattern Controller Test")
    print("=" * 50)
    
    if args.test in ['serial', 'all']:
        print("\n🔌 Testing Serial Connection...")
        if not test_serial_connection(args.port, args.baudrate):
            print("❌ Serial connection test failed. Exiting.")
            return
    
    if args.test in ['patterns', 'all']:
        print("\n🎨 Testing LED Patterns...")
        test_led_patterns(args.port, args.baudrate)
    
    print("\n✅ All tests completed!")
    print("\n💡 Tips:")
    print("   - Make sure the ESP32 is powered and connected")
    print("   - Check that the correct serial port is specified")
    print("   - Verify that the LED strips are properly connected")
    print("   - Use 'make monitor' to see real-time output")

if __name__ == "__main__":
    main()
