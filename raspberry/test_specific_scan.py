#!/usr/bin/env python3
"""
Test script for specific IP Sonoff device scanning.
This script tests the modified SonoffDeviceManager that scans only specific IPs.
"""

import asyncio
import sys
import os

# Add the current directory to Python path
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from sonoff_manager import SonoffDeviceManager
from config import get_config

async def test_specific_ip_scanning():
    """Test the specific IP scanning functionality"""
    
    print("🔍 Testing Specific IP Sonoff Device Scanning")
    print("=" * 50)
    
    # Get configuration
    config = get_config()
    print(f"📋 Configuration:")
    print(f"   - Use specific IPs only: {config.network.use_specific_ips_only}")
    print(f"   - Specific device IPs: {config.network.specific_device_ips}")
    print(f"   - Network range: {config.network.local_network}")
    print()
    
    # Create device manager
    device_manager = SonoffDeviceManager()
    
    try:
        print("🚀 Starting Sonoff Device Manager...")
        await device_manager.start()
        
        print("🔍 Triggering device discovery...")
        start_time = asyncio.get_event_loop().time()
        
        # Discover devices
        discovered_devices = await device_manager.discover_devices(force_refresh=True)
        
        end_time = asyncio.get_event_loop().time()
        duration = end_time - start_time
        
        print(f"✅ Discovery completed in {duration:.2f} seconds")
        print(f"📱 Found {len(discovered_devices)} devices")
        print()
        
        # Display discovered devices
        if discovered_devices:
            print("📱 Discovered Devices:")
            print("-" * 30)
            for i, device in enumerate(discovered_devices, 1):
                print(f"{i}. {device.name}")
                print(f"   - IP: {device.ip_address}")
                print(f"   - Type: {device.type}")
                print(f"   - Model: {device.model}")
                print(f"   - Status: {device.status}")
                print(f"   - Power: {device.power_state}")
                print()
        else:
            print("❌ No devices discovered")
            print("   This could mean:")
            print("   - Devices are not powered on")
            print("   - Devices are not connected to network")
            print("   - Network configuration is incorrect")
            print("   - Devices are not responding on port 80")
        
        # Test device control if devices were found
        if discovered_devices:
            print("🎛️  Testing device control...")
            test_device = discovered_devices[0]
            print(f"   Testing with device: {test_device.name} ({test_device.ip_address})")
            
            # Get current status
            current_status = await device_manager.get_device_status(test_device.id)
            if current_status:
                print(f"   Current power state: {current_status.power_state}")
                
                # Test toggle (if supported)
                try:
                    from models import DeviceControl, PowerState
                    control = DeviceControl(power=PowerState.TOGGLE)
                    response = await device_manager.control_device(test_device.id, control)
                    print(f"   Toggle response: {response.success} - {response.message}")
                except Exception as e:
                    print(f"   Control test failed: {e}")
        
    except Exception as e:
        print(f"❌ Error during testing: {e}")
        import traceback
        traceback.print_exc()
    
    finally:
        print("🛑 Stopping device manager...")
        await device_manager.stop()
        print("✅ Test completed")

def main():
    """Main function"""
    print("🔌 Sonoff Specific IP Scanning Test")
    print("=" * 40)
    print()
    
    # Run the async test
    asyncio.run(test_specific_ip_scanning())

if __name__ == "__main__":
    main()
