#!/usr/bin/env python3
"""
OTA Manager for Flamingods ESP32 Devices
Manages over-the-air firmware updates for all ESP devices
"""

import requests
import json
import time
import os
import sys
from pathlib import Path
import subprocess

class OTAManager:
    def __init__(self):
        self.esps = {}
        self.firmware_dir = Path(__file__).parent
        
    def discover_esps(self):
        """Discover ESP devices on the network"""
        print("🔍 Discovering ESP devices...")
        
        # Common ESP hostnames
        hostnames = [
            "crown-esp32",
            "stage-esp32", 
            "flamingo-esp32",
            "button-esp32",
            "station-esp32"
        ]
        
        discovered = []
        
        # Try to find ESPs by hostname first
        for hostname in hostnames:
            try:
                # Try to resolve hostname to IP
                import socket
                ip = socket.gethostbyname(hostname)
                if self.test_esp_endpoint(ip):
                    esp_info = self.get_esp_info(ip)
                    if esp_info:
                        self.esps[ip] = esp_info
                        discovered.append(esp_info)
                        print(f"✅ Found {esp_info['device']} at {ip}")
            except:
                pass
        
        # If no ESPs found by hostname, scan network
        if not discovered:
            print("No ESPs found by hostname, scanning network...")
            discovered = self.scan_network()
        
        return discovered
    
    def scan_network(self):
        """Scan network for ESP devices"""
        discovered = []
        
        # Get local network range
        try:
            import socket
            s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            s.connect(("8.8.8.8", 80))
            local_ip = s.getsockname()[0]
            s.close()
            
            network_base = ".".join(local_ip.split(".")[:3])
            
            print(f"Scanning network: {network_base}.0/24")
            
            # Scan common IP ranges
            for i in range(200, 255):
                ip = f"{network_base}.{i}"
                if self.test_esp_endpoint(ip):
                    esp_info = self.get_esp_info(ip)
                    if esp_info:
                        self.esps[ip] = esp_info
                        discovered.append(esp_info)
                        print(f"✅ Found {esp_info['device']} at {ip}")
                        
        except Exception as e:
            print(f"Network scan error: {e}")
        
        return discovered
    
    def test_esp_endpoint(self, ip):
        """Test if an IP is running an ESP32"""
        try:
            response = requests.get(f"http://{ip}/health", timeout=2)
            return response.status_code == 200 and response.text.strip() == "OK"
        except:
            return False
    
    def get_esp_info(self, ip):
        """Get information about an ESP device"""
        try:
            response = requests.get(f"http://{ip}/status", timeout=1)
            if response.status_code == 200:
                data = response.json()
                return {
                    'ip': ip,
                    'device': data.get('device', 'unknown'),
                    'firmware_version': data.get('firmware_version', 'unknown'),
                    'current_plan': data.get('current_plan', 'unknown'),
                    'wifi_connected': data.get('wifi_connected', False),
                    'uptime': data.get('uptime', 0)
                }
        except:
            pass
        return None
    
    def list_esps(self):
        """List all discovered ESP devices"""
        if not self.esps:
            print("❌ No ESP devices discovered. Run discovery first.")
            return
        
        print("\n" + "="*60)
        print("📱 DISCOVERED ESP DEVICES")
        print("="*60)
        
        for ip, esp in self.esps.items():
            print(f"\n📍 {esp['device'].upper()}")
            print(f"   IP Address: {ip}")
            print(f"   Firmware: {esp['firmware_version']}")
            print(f"   Current Plan: {esp['current_plan']}")
            print(f"   WiFi: {'✅ Connected' if esp['wifi_connected'] else '❌ Disconnected'}")
            print(f"   Uptime: {esp['uptime']} seconds")
    
    def check_versions(self):
        """Check firmware versions of all ESPs"""
        print("\n🔍 Checking firmware versions...")
        
        for ip, esp in self.esps.items():
            try:
                response = requests.get(f"http://{ip}/version", timeout=2)
                if response.status_code == 200:
                    data = response.json()
                    current_version = data.get('firmware_version', 'unknown')
                    print(f"   {esp['device']}: {current_version}")
                else:
                    print(f"   {esp['device']}: Error getting version")
            except Exception as e:
                print(f"   {esp['device']}: Connection failed - {e}")
    
    def upload_firmware(self, target_ip, firmware_path):
        """Upload firmware to a specific ESP"""
        if target_ip not in self.esps:
            print(f"❌ ESP at {target_ip} not found in discovered devices")
            return False
        
        esp = self.esps[target_ip]
        print(f"\n📤 Uploading firmware to {esp['device']} at {target_ip}")
        
        # Check if firmware file exists
        if not os.path.exists(firmware_path):
            print(f"❌ Firmware file not found: {firmware_path}")
            return False
        
        # Check OTA status
        try:
            response = requests.get(f"http://{target_ip}/ota-status", timeout=2)
            if response.status_code == 200:
                data = response.json()
                if data.get('ota_in_progress'):
                    print("❌ OTA update already in progress")
                    return False
        except:
            print("⚠️  Could not check OTA status")
        
        # Use esptool for OTA upload
        try:
            print("🚀 Starting OTA upload...")
            
            # Build esptool command
            cmd = [
                "esptool.py",
                "--chip", "esp32",
                "--port", f"http://{target_ip}:3232",
                "--baud", "115200",
                "write_flash",
                "0x10000", firmware_path
            ]
            
            print(f"Running: {' '.join(cmd)}")
            
            # Run esptool
            result = subprocess.run(cmd, capture_output=True, text=True)
            
            if result.returncode == 0:
                print("✅ Firmware upload successful!")
                return True
            else:
                print(f"❌ Firmware upload failed:")
                print(result.stderr)
                return False
                
        except FileNotFoundError:
            print("❌ esptool.py not found. Please install esptool:")
            print("   pip install esptool")
            return False
        except Exception as e:
            print(f"❌ Upload error: {e}")
            return False
    
    def upload_to_all(self, firmware_path):
        """Upload firmware to all discovered ESPs"""
        if not self.esps:
            print("❌ No ESP devices discovered. Run discovery first.")
            return
        
        print(f"\n📤 Uploading firmware to all ESPs...")
        
        success_count = 0
        total_count = len(self.esps)
        
        for ip, esp in self.esps.items():
            print(f"\n--- Uploading to {esp['device']} ---")
            if self.upload_firmware(ip, firmware_path):
                success_count += 1
            time.sleep(2)  # Wait between uploads
        
        print(f"\n📊 Upload Summary: {success_count}/{total_count} successful")
    
    def interactive_menu(self):
        """Interactive menu for OTA management"""
        while True:
            print("\n" + "="*60)
            print("🚀 FLAMINGODS OTA MANAGER")
            print("="*60)
            print("1. Discover ESP devices")
            print("2. List discovered ESPs")
            print("3. Check firmware versions")
            print("4. Upload firmware to specific ESP")
            print("5. Upload firmware to all ESPs")
            print("6. Exit")
            
            choice = input("\nSelect option (1-6): ").strip()
            
            if choice == "1":
                self.discover_esps()
                
            elif choice == "2":
                self.list_esps()
                
            elif choice == "3":
                self.check_versions()
                
            elif choice == "4":
                if not self.esps:
                    print("❌ No ESP devices discovered. Run discovery first.")
                    continue
                
                print("\nAvailable ESPs:")
                for i, (ip, esp) in enumerate(self.esps.items(), 1):
                    print(f"{i}. {esp['device']} at {ip}")
                
                try:
                    esp_choice = int(input("Select ESP (number): ")) - 1
                    if 0 <= esp_choice < len(self.esps):
                        ip = list(self.esps.keys())[esp_choice]
                        firmware_path = input("Enter firmware file path: ").strip()
                        self.upload_firmware(ip, firmware_path)
                    else:
                        print("❌ Invalid selection")
                except ValueError:
                    print("❌ Invalid input")
                    
            elif choice == "5":
                if not self.esps:
                    print("❌ No ESP devices discovered. Run discovery first.")
                    continue
                    
                firmware_path = input("Enter firmware file path: ").strip()
                self.upload_to_all(firmware_path)
                
            elif choice == "6":
                print("👋 Goodbye!")
                break
                
            else:
                print("❌ Invalid option")

def main():
    """Main function"""
    print("🚀 Flamingods OTA Manager")
    print("="*40)
    
    manager = OTAManager()
    
    if len(sys.argv) > 1:
        # Command line mode
        command = sys.argv[1]
        
        if command == "discover":
            manager.discover_esps()
            manager.list_esps()
            
        elif command == "list":
            manager.discover_esps()
            manager.list_esps()
            
        elif command == "versions":
            manager.discover_esps()
            manager.check_versions()
            
        elif command == "upload" and len(sys.argv) > 3:
            target_ip = sys.argv[2]
            firmware_path = sys.argv[3]
            manager.discover_esps()
            manager.upload_firmware(target_ip, firmware_path)
            
        elif command == "upload-all" and len(sys.argv) > 2:
            firmware_path = sys.argv[2]
            manager.discover_esps()
            manager.upload_to_all(firmware_path)
            
        else:
            print("Usage:")
            print("  python ota_manager.py discover")
            print("  python ota_manager.py list")
            print("  python ota_manager.py versions")
            print("  python ota_manager.py upload <IP> <firmware_path>")
            print("  python ota_manager.py upload-all <firmware_path>")
            print("  python ota_manager.py (interactive mode)")
    else:
        # Interactive mode
        manager.interactive_menu()

if __name__ == "__main__":
    main()
