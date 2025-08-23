#!/usr/bin/env python3
"""
Simple firmware upload script for ESP devices.
Usage: python upload_firmware.py <device_name>
Example: python upload_firmware.py crown
"""

import sys
import os
import subprocess
import platform

def upload_firmware(device_name):
    """Upload firmware to ESP device using PlatformIO OTA."""
    
    # Map device names to their directories
    devices = {
        'crown': 'crown',
        'stage': 'stage', 
        'flamingo': 'flamingo',
        'button': 'button',
        'station': 'station'
    }
    
    if device_name not in devices:
        print(f"❌ Unknown device: {device_name}")
        print(f"Available devices: {', '.join(devices.keys())}")
        return False
    
    device_dir = devices[device_name]
    project_path = os.path.join(os.path.dirname(__file__), device_dir)
    
    if not os.path.exists(project_path):
        print(f"❌ Device directory not found: {project_path}")
        return False
    
    print(f"🚀 Uploading firmware to {device_name}...")
    print(f"📁 Project path: {project_path}")
    
    # Change to device directory
    os.chdir(project_path)
    
    # Set OTA password environment variable
    os.environ['ESP_OTA_PASSWORD'] = 'flamingods2024'
    
    try:
        # Run PlatformIO OTA upload
        cmd = ['pio', 'run', '-e', 'esp32dev-ota', '--target', 'upload']
        print(f"🔧 Running: {' '.join(cmd)}")
        
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=120)
        
        if result.returncode == 0:
            print(f"✅ Firmware uploaded successfully to {device_name}!")
            return True
        else:
            print(f"❌ Upload failed for {device_name}:")
            print(result.stderr)
            return False
            
    except subprocess.TimeoutExpired:
        print(f"⏰ Upload timed out for {device_name}")
        return False
    except Exception as e:
        print(f"❌ Error uploading to {device_name}: {e}")
        return False

def main():
    if len(sys.argv) != 2:
        print("Usage: python upload_firmware.py <device_name>")
        print("Available devices: crown, stage, flamingo, button, station")
        sys.exit(1)
    
    device_name = sys.argv[1].lower()
    
    if upload_firmware(device_name):
        sys.exit(0)
    else:
        sys.exit(1)

if __name__ == "__main__":
    main()
