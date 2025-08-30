#!/usr/bin/env python3
"""
Test script for the main Sonoff server with specific IP scanning.
This script starts the server and tests the discovery endpoint.
"""

import asyncio
import sys
import os
import time

# Add the current directory to Python path
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from main import app
from config import get_config
import httpx

async def test_server():
    """Test the main server with specific IP scanning"""
    
    print("🚀 Testing Sonoff Server with Specific IP Scanning")
    print("=" * 55)
    
    # Get configuration
    config = get_config()
    print(f"📋 Configuration:")
    print(f"   - Server: {config.server.host}:{config.server.port}")
    print(f"   - Use specific IPs only: {config.network.use_specific_ips_only}")
    print(f"   - Specific device IPs: {config.network.specific_device_ips}")
    print(f"   - Network range: {config.network.local_network}")
    print()
    
    # Start server in background
    print("🔌 Starting server...")
    
    # Test server endpoints
    base_url = f"http://{config.server.host}:{config.server.port}"
    
    try:
        async with httpx.AsyncClient() as client:
            # Test health endpoint
            print("🏥 Testing health endpoint...")
            response = await client.get(f"{base_url}/health")
            if response.status_code == 200:
                print("   ✅ Health check passed")
                health_data = response.json()
                print(f"   📊 Status: {health_data.get('status')}")
                print(f"   ⏱️  Uptime: {health_data.get('uptime')}")
            else:
                print(f"   ❌ Health check failed: {response.status_code}")
            
            print()
            
            # Test device discovery
            print("🔍 Testing device discovery...")
            start_time = time.time()
            
            response = await client.post(f"{base_url}/discover", json={"force_refresh": True})
            if response.status_code == 200:
                discovery_data = response.json()
                duration = time.time() - start_time
                
                print(f"   ✅ Discovery successful in {duration:.2f} seconds")
                print(f"   📱 Total devices: {discovery_data.get('total_count', 0)}")
                print(f"   🟢 Online devices: {discovery_data.get('online_count', 0)}")
                print(f"   🌐 Network range: {discovery_data.get('network_range', 'Unknown')}")
                
                # Show discovered devices
                devices = discovery_data.get('devices', [])
                if devices:
                    print("\n   📱 Discovered Devices:")
                    for i, device in enumerate(devices, 1):
                        print(f"      {i}. {device.get('name', 'Unknown')}")
                        print(f"         - IP: {device.get('ip_address', 'Unknown')}")
                        print(f"         - Type: {device.get('type', 'Unknown')}")
                        print(f"         - Status: {device.get('status', 'Unknown')}")
                        print(f"         - Power: {device.get('power_state', 'Unknown')}")
                else:
                    print("   ❌ No devices discovered")
                    print("      This is expected in development environment")
            else:
                print(f"   ❌ Discovery failed: {response.status_code}")
                print(f"      Response: {response.text}")
            
            print()
            
            # Test devices list endpoint
            print("📋 Testing devices list endpoint...")
            response = await client.get(f"{base_url}/devices")
            if response.status_code == 200:
                devices = response.json()
                print(f"   ✅ Devices list: {len(devices)} devices")
            else:
                print(f"   ❌ Devices list failed: {response.status_code}")
            
            print()
            
            # Test system status
            print("⚙️  Testing system status...")
            response = await client.get(f"{base_url}/system/status")
            if response.status_code == 200:
                status_data = response.json()
                print("   ✅ System status:")
                print(f"      - Server: {status_data.get('server', {}).get('status', 'Unknown')}")
                print(f"      - Devices: {status_data.get('devices', {}).get('total', 0)} total, {status_data.get('devices', {}).get('online', 0)} online")
                print(f"      - WebSocket clients: {status_data.get('websocket', {}).get('clients', 0)}")
            else:
                print(f"   ❌ System status failed: {response.status_code}")
    
    except Exception as e:
        print(f"❌ Error during testing: {e}")
        import traceback
        traceback.print_exc()
    
    print("\n✅ Server testing completed")

def main():
    """Main function"""
    print("🔌 Sonoff Server Test with Specific IP Scanning")
    print("=" * 50)
    print()
    
    # Run the async test
    asyncio.run(test_server())

if __name__ == "__main__":
    main()
