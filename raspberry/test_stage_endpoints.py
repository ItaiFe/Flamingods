#!/usr/bin/env python3
"""
Test script for Stage LED endpoints
Tests the simple HTTP endpoints that forward to the stage ESP32
"""

import asyncio
import httpx
import json
import sys

# Test configuration
BASE_URL = "http://localhost:8000"
STAGE_IP = "192.168.1.100"  # Update this to your stage ESP32 IP

class StageEndpointTester:
    def __init__(self, base_url):
        self.base_url = base_url
        
    async def test_endpoint(self, endpoint, method="POST", expected_status=200):
        """Test a specific endpoint"""
        try:
            url = f"{self.base_url}{endpoint}"
            print(f"\n🔄 Testing {method} {endpoint}...")
            
            async with httpx.AsyncClient(timeout=10.0) as client:
                if method == "POST":
                    response = await client.post(url)
                elif method == "GET":
                    response = await client.get(url)
                else:
                    print(f"❌ Unknown method: {method}")
                    return False
                
                if response.status_code == expected_status:
                    print(f"✅ {endpoint}: PASSED (Status: {response.status_code})")
                    try:
                        data = response.json()
                        print(f"   Response: {json.dumps(data, indent=2)}")
                    except:
                        print(f"   Response: {response.text}")
                    return True
                else:
                    print(f"❌ {endpoint}: FAILED - Status: {response.status_code}")
                    print(f"   Response: {response.text}")
                    return False
                    
        except httpx.ConnectError:
            print(f"❌ {endpoint}: FAILED - Cannot connect to server")
            return False
        except Exception as e:
            print(f"❌ {endpoint}: ERROR - {e}")
            return False
    
    async def run_all_tests(self):
        """Run all stage endpoint tests"""
        print("🎭 Stage LED Endpoint Test Suite")
        print("=" * 40)
        print(f"Testing server at: {self.base_url}")
        print(f"Stage ESP32 IP: {STAGE_IP}")
        print()
        
        tests = [
            ("/stage/idle", "POST"),
            ("/stage/skip", "POST"),
            ("/stage/show", "POST"),
            ("/stage/special", "POST"),
            ("/stage/status", "GET"),
            ("/stage/health", "GET"),
        ]
        
        results = []
        for endpoint, method in tests:
            result = await self.test_endpoint(endpoint, method)
            results.append(result)
        
        print("\n" + "=" * 40)
        passed = sum(results)
        total = len(results)
        print(f"🎉 Test Results: {passed}/{total} tests passed")
        
        if passed == total:
            print("✅ All stage endpoints are working!")
        else:
            print("❌ Some tests failed. Check server logs for details.")
        
        return passed == total

async def main():
    if len(sys.argv) > 1:
        base_url = sys.argv[1]
    else:
        base_url = BASE_URL
    
    tester = StageEndpointTester(base_url)
    
    try:
        success = await tester.run_all_tests()
        sys.exit(0 if success else 1)
    except KeyboardInterrupt:
        print("\n\n⏹️  Test interrupted by user")
        sys.exit(1)
    except Exception as e:
        print(f"\n❌ Test suite error: {e}")
        sys.exit(1)

if __name__ == "__main__":
    asyncio.run(main())
