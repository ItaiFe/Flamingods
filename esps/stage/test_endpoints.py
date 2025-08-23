#!/usr/bin/env python3
"""
Test script for Stage ESP32 HTTP endpoints
Tests all lighting plan endpoints and status functions
"""

import requests
import json
import time
import sys

class StageESPTester:
    def __init__(self, esp_ip):
        self.esp_ip = esp_ip
        self.base_url = f"http://{esp_ip}"
        
    def test_health(self):
        """Test health endpoint"""
        try:
            response = requests.get(f"{self.base_url}/health", timeout=5)
            if response.status_code == 200 and response.text == "OK":
                print("✅ Health check: PASSED")
                return True
            else:
                print(f"❌ Health check: FAILED - Status: {response.status_code}, Response: {response.text}")
                return False
        except Exception as e:
            print(f"❌ Health check: ERROR - {e}")
            return False
    
    def test_status(self):
        """Test status endpoint"""
        try:
            response = requests.get(f"{self.base_url}/status", timeout=5)
            if response.status_code == 200:
                data = response.json()
                print("✅ Status check: PASSED")
                print(f"   Current plan: {data.get('current_plan', 'unknown')}")
                print(f"   WiFi connected: {data.get('wifi_connected', 'unknown')}")
                print(f"   IP address: {data.get('ip_address', 'unknown')}")
                print(f"   Uptime: {data.get('uptime', 'unknown')} seconds")
                return True
            else:
                print(f"❌ Status check: FAILED - Status: {response.status_code}")
                return False
        except Exception as e:
            print(f"❌ Status check: ERROR - {e}")
            return False
    
    def test_lighting_plans(self):
        """Test all lighting plan endpoints"""
        plans = [
            ("idle", "IDLE"),
            ("skip", "SKIP"), 
            ("show", "SHOW"),
            ("special", "SPECIAL")
        ]
        
        results = []
        
        for endpoint, plan_name in plans:
            try:
                print(f"\n🔄 Testing {plan_name} plan...")
                
                # Send POST request
                response = requests.post(f"{self.base_url}/{endpoint}", timeout=5)
                
                if response.status_code == 200:
                    data = response.json()
                    if data.get('status') == 'success' and data.get('plan') == endpoint:
                        print(f"✅ {plan_name} plan: PASSED")
                        results.append(True)
                    else:
                        print(f"❌ {plan_name} plan: FAILED - Invalid response: {data}")
                        results.append(False)
                else:
                    print(f"❌ {plan_name} plan: FAILED - Status: {response.status_code}")
                    results.append(False)
                
                # Wait a bit to see the effect
                time.sleep(2)
                
                # Check status to confirm plan change
                status_response = requests.get(f"{self.base_url}/status", timeout=5)
                if status_response.status_code == 200:
                    status_data = status_response.json()
                    current_plan = status_data.get('current_plan', -1)
                    plan_mapping = {0: 'idle', 1: 'skip', 2: 'show', 3: 'special'}
                    actual_plan = plan_mapping.get(current_plan, 'unknown')
                    
                    if actual_plan == endpoint:
                        print(f"   ✅ Plan change confirmed: {actual_plan}")
                    else:
                        print(f"   ⚠️  Plan change not confirmed. Expected: {endpoint}, Got: {actual_plan}")
                
            except Exception as e:
                print(f"❌ {plan_name} plan: ERROR - {e}")
                results.append(False)
        
        return all(results)
    
    def run_full_test(self):
        """Run complete test suite"""
        print("🎭 Stage ESP32 Endpoint Test Suite")
        print("=" * 40)
        print(f"Testing ESP at: {self.esp_ip}")
        print()
        
        # Test health first
        if not self.test_health():
            print("\n❌ Health check failed. ESP may not be responding.")
            return False
        
        print()
        
        # Test status
        if not self.test_status():
            print("\n❌ Status check failed.")
            return False
        
        print()
        
        # Test lighting plans
        print("🎨 Testing Lighting Plans...")
        if not self.test_lighting_plans():
            print("\n❌ Some lighting plan tests failed.")
            return False
        
        print("\n" + "=" * 40)
        print("🎉 All tests completed successfully!")
        return True

def main():
    if len(sys.argv) != 2:
        print("Usage: python3 test_endpoints.py <ESP_IP_ADDRESS>")
        print("Example: python3 test_endpoints.py 192.168.1.100")
        sys.exit(1)
    
    esp_ip = sys.argv[1]
    tester = StageESPTester(esp_ip)
    
    try:
        success = tester.run_full_test()
        sys.exit(0 if success else 1)
    except KeyboardInterrupt:
        print("\n\n⏹️  Test interrupted by user")
        sys.exit(1)
    except Exception as e:
        print(f"\n❌ Test suite error: {e}")
        sys.exit(1)

if __name__ == "__main__":
    main()
