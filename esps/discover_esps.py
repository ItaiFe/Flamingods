#!/usr/bin/env python3
"""
ESP32 Network Discovery Script
Scans the local network to find ESP32 devices running the Flamingods project
"""

import socket
import requests
import threading
import time
import ipaddress
from concurrent.futures import ThreadPoolExecutor, as_completed

class ESPDiscoverer:
    def __init__(self):
        self.discovered_esps = []
        self.local_ip = self.get_local_ip()
        self.network = self.get_network()
        
    def get_local_ip(self):
        """Get the local IP address"""
        try:
            # Create a socket to get local IP
            s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            s.connect(("8.8.8.8", 80))
            local_ip = s.getsockname()[0]
            s.close()
            return local_ip
        except:
            return "192.168.1.100"  # Fallback
    
    def get_network(self):
        """Get the local network range"""
        try:
            ip_parts = self.local_ip.split('.')
            return f"{ip_parts[0]}.{ip_parts[1]}.{ip_parts[2]}.0/24"
        except:
            return "192.168.1.0/24"  # Fallback
    
    def scan_port(self, ip, port=80, timeout=1):
        """Check if a port is open on an IP address"""
        try:
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.settimeout(timeout)
            result = sock.connect_ex((ip, port))
            sock.close()
            return result == 0
        except:
            return False
    
    def test_esp_endpoints(self, ip):
        """Test if an IP is running an ESP32 with our endpoints"""
        try:
            # Test health endpoint first (fastest)
            health_url = f"http://{ip}/health"
            response = requests.get(health_url, timeout=2)
            if response.status_code == 200 and response.text.strip() == "OK":
                return True
        except:
            pass
        
        try:
            # Test status endpoint
            status_url = f"http://{ip}/status"
            response = requests.get(status_url, timeout=2)
            if response.status_code == 200:
                data = response.json()
                if 'current_plan' in data and 'wifi_connected' in data:
                    return True
        except:
            pass
        
        return False
    
    def identify_esp_type(self, ip):
        """Identify whether this is a stage or crown ESP"""
        try:
            # Test crown-specific endpoint
            button_url = f"http://{ip}/button"
            response = requests.post(button_url, timeout=2)
            if response.status_code == 200:
                data = response.json()
                if data.get('plan') == 'button':
                    return "Crown ESP"
            
            # Test stage-specific endpoint
            skip_url = f"http://{ip}/skip"
            response = requests.post(skip_url, timeout=2)
            if response.status_code == 200:
                data = response.json()
                if data.get('plan') == 'skip':
                    return "Stage ESP"
            
            # If we can't determine, check what endpoints exist
            endpoints = []
            for endpoint in ['/idle', '/show', '/special', '/status', '/health']:
                try:
                    response = requests.get(f"http://{ip}{endpoint}", timeout=1)
                    if response.status_code in [200, 405]:  # 405 = method not allowed
                        endpoints.append(endpoint)
                except:
                    pass
            
            if '/skip' in endpoints:
                return "Stage ESP"
            elif '/button' in endpoints:
                return "Crown ESP"
            else:
                return "Unknown ESP"
                
        except:
            return "Unknown ESP"
    
    def scan_ip(self, ip):
        """Scan a single IP address"""
        if self.scan_port(ip):
            if self.test_esp_endpoints(ip):
                esp_type = self.identify_esp_type(ip)
                return {
                    'ip': ip,
                    'type': esp_type,
                    'port_80_open': True,
                    'endpoints_working': True
                }
            else:
                return {
                    'ip': ip,
                    'type': 'Unknown',
                    'port_80_open': True,
                    'endpoints_working': False
                }
        return None
    
    def scan_network(self):
        """Scan the entire network for ESP devices"""
        print(f"🔍 Scanning network: {self.network}")
        print(f"📍 Local IP: {self.local_ip}")
        print("⏳ This may take a few minutes...\n")
        
        network = ipaddress.IPv4Network(self.network, strict=False)
        total_ips = network.num_addresses
        
        with ThreadPoolExecutor(max_workers=50) as executor:
            # Submit all IPs for scanning
            future_to_ip = {
                executor.submit(self.scan_port, str(ip)): str(ip) 
                for ip in network.hosts()
            }
            
            # Process results as they complete
            for future in as_completed(future_to_ip):
                ip = future_to_ip[future]
                try:
                    port_open = future.result()
                    if port_open:
                        print(f"🔓 Found open port 80 on {ip}")
                        # Test if it's an ESP
                        esp_info = self.scan_ip(ip)
                        if esp_info:
                            self.discovered_esps.append(esp_info)
                            print(f"✅ Found {esp_info['type']} at {ip}")
                        else:
                            print(f"❌ {ip} has port 80 open but is not an ESP")
                except Exception as e:
                    pass
        
        return self.discovered_esps
    
    def print_results(self):
        """Print the discovery results"""
        print("\n" + "="*50)
        print("🎯 ESP DISCOVERY RESULTS")
        print("="*50)
        
        if not self.discovered_esps:
            print("❌ No ESP devices found!")
            print("\nPossible reasons:")
            print("- ESPs are not powered on")
            print("- ESPs are not connected to WiFi")
            print("- ESPs are on a different network")
            print("- Firewall is blocking port 80")
            return
        
        print(f"✅ Found {len(self.discovered_esps)} ESP device(s):\n")
        
        for esp in self.discovered_esps:
            print(f"📍 {esp['type']}")
            print(f"   IP Address: {esp['ip']}")
            print(f"   Port 80: {'✅ Open' if esp['port_80_open'] else '❌ Closed'}")
            print(f"   Endpoints: {'✅ Working' if esp['endpoints_working'] else '❌ Not Working'}")
            print()
        
        print("🌐 Test the ESPs:")
        for esp in self.discovered_esps:
            if esp['type'] == 'Crown ESP':
                print(f"   Crown: curl -X POST http://{esp['ip']}/button")
            elif esp['type'] == 'Stage ESP':
                print(f"   Stage: curl -X POST http://{esp['ip']}/show")
            print(f"   Status: curl http://{esp['ip']}/status")

def main():
    """Main discovery function"""
    print("🚀 ESP32 Network Discovery Tool")
    print("="*40)
    
    discoverer = ESPDiscoverer()
    
    try:
        # Scan the network
        results = discoverer.scan_network()
        
        # Print results
        discoverer.print_results()
        
    except KeyboardInterrupt:
        print("\n\n⏹️  Scan interrupted by user")
    except Exception as e:
        print(f"\n❌ Error during scan: {e}")

if __name__ == "__main__":
    main()
