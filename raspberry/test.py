import asyncio
import sys
import os

# Add the current directory to Python path
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

async def test_ewelink():
    """Test ewelink cloud-based functionality"""
    
    print("🔌 Testing EWeLink Cloud Library v0.2.1")
    print("=" * 45)
    
    try:
        from ewelink import EWeLink
        from ewelink.types import AppCredentials, EmailUserCredentials
        
        print("📋 EWeLink Library Information:")
        print("   - This library requires cloud-based authentication")
        print("   - You need to register an app with EWeLink/Sonoff")
        print("   - App credentials are required for API access")
        print()
        
        # Check if credentials are provided
        app_id = os.getenv('EWELINK_APP_ID')
        app_secret = os.getenv('EWELINK_APP_SECRET')
        
        if not app_id or not app_secret:
            print("❌ Missing EWeLink App Credentials")
            print("   To use EWeLink cloud API, you need:")
            print("   1. Register at https://dev.ewelink.cc/")
            print("   2. Create an app to get APP_ID and APP_SECRET")
            print("   3. Set environment variables:")
            print("      export EWELINK_APP_ID='your_app_id'")
            print("      export EWELINK_APP_SECRET='your_app_secret'")
            print()
            print("   Note: This requires internet connection and cloud service")
            return False
        
        # Create credentials
        app_creds = AppCredentials(
            app_id=app_id,
            app_secret=app_secret
        )
        
        user_creds = EmailUserCredentials(
            email="tolkachovd@email.com",
            password="Ditomi1!"
        )
        
        # Create EWeLink client
        print("🔐 Creating EWeLink client...")
        client = EWeLink(app_creds, user_creds)
        
        # Login to the service
        print("🔑 Logging in...")
        login_response = await client.login(region='eu')  # or 'us', 'cn', 'as'
        print(f"✅ Login successful: {login_response}")
        
        # Get device list
        print("📱 Getting device list...")
        devices = await client.get_thing_list()
        print(f"📱 Found {len(devices) if devices else 0} devices")
        
        if devices:
            print("\n📱 Device Details:")
            for i, device in enumerate(devices, 1):
                print(f"   {i}. Device ID: {device.deviceid}")
                print(f"      Name: {device.name}")
                print(f"      Type: {device.device_type}")
                print(f"      Online: {device.online}")
                print(f"      Brand: {device.brand_name}")
                print()
            
            # Test with first device if available
            if devices:
                test_device = devices[0]
                print(f"🎛️  Testing device: {test_device.name} ({test_device.deviceid})")
                
                # Try to update device status (turn on)
                try:
                    print("   🔌 Attempting to turn device ON...")
                    # Note: The exact method depends on device type
                    # You may need to check device.params for available controls
                    print(f"   📊 Device params: {test_device.params}")
                    print(f"   📊 Device config: {test_device.config}")
                    
                except Exception as e:
                    print(f"   ❌ Error controlling device: {e}")
        
        # Close the client
        await client.close()
        print("✅ Client closed successfully")
        return True
        
    except ImportError as e:
        print(f"❌ Import error: {e}")
        return False
    except Exception as e:
        print(f"❌ Error: {e}")
        import traceback
        traceback.print_exc()
        return False

async def test_local_sonoff():
    """Test our local Sonoff implementation as alternative"""
    
    print("\n" + "="*60)
    print("🏠 Testing Local Sonoff Implementation (Alternative)")
    print("="*60)
    
    try:
        from sonoff_manager import SonoffDeviceManager
        from config import get_config
        
        print("📋 Local Sonoff System Information:")
        print("   - Works completely offline (no internet required)")
        print("   - Scans only specific IPs: 192.168.1.216, 192.168.1.217")
        print("   - Fast discovery (< 1 second)")
        print("   - Direct local network communication")
        print()
        
        # Get configuration
        config = get_config()
        print(f"📋 Configuration:")
        print(f"   - Use specific IPs only: {config.network.use_specific_ips_only}")
        print(f"   - Specific device IPs: {config.network.specific_device_ips}")
        print(f"   - Network range: {config.network.local_network}")
        print()
        
        # Create device manager
        device_manager = SonoffDeviceManager()
        
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
        
        # Stop device manager
        await device_manager.stop()
        print("✅ Local Sonoff test completed")
        return True
        
    except Exception as e:
        print(f"❌ Error in local Sonoff test: {e}")
        import traceback
        traceback.print_exc()
        return False

async def main():
    """Main function to test both ewelink and local Sonoff"""
    
    print("🔌 Sonoff Device Testing Suite")
    print("=" * 40)
    print()
    
    # Test ewelink first
    ewelink_success = await test_ewelink()
    
    # Test local Sonoff as alternative
    local_success = await test_local_sonoff()
    
    # Summary
    print("\n" + "="*60)
    print("📊 TEST SUMMARY")
    print("="*60)
    print(f"🔌 EWeLink Cloud API: {'✅ SUCCESS' if ewelink_success else '❌ FAILED'}")
    print(f"🏠 Local Sonoff System: {'✅ SUCCESS' if local_success else '❌ FAILED'}")
    print()
    
    if not ewelink_success and local_success:
        print("💡 RECOMMENDATION:")
        print("   Use the Local Sonoff System - it's working and doesn't require internet!")
        print("   Start the server: uv run python main.py")
        print("   Test discovery: curl -X POST http://localhost:8000/discover")
    
    print("\n✅ Testing completed")

if __name__ == "__main__":
    # Run the async function
    asyncio.run(main())