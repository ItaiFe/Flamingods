#!/usr/bin/env python3
"""
Test script for Flamingo ESP32 Mega Special Mode

This script triggers the mega special animation sequence:
1. Malfunction (2s) - Random flickering/glitching
2. Drain (3s) - Color drains from center to edges
3. Wait (3s) - All LEDs off
4. Pulsate (5s) - Vivid pink pulsating
5. Confetti (continuous) - Colorful confetti explosion

Usage:
    python test_mega_special.py <IP_ADDRESS>
    python test_mega_special.py  # Uses default 192.168.1.200
"""

import requests
import sys
import time

# Default IP (change if needed)
DEFAULT_IP = "192.168.1.200"

def trigger_mega_special(ip):
    """Trigger the mega special animation"""
    url = f"http://{ip}/mega-special"

    print(f"Triggering Mega Special Mode on {ip}...")
    print("Animation sequence (23 seconds total):")
    print("  0-2s:    Malfunction (flickering/glitching)")
    print("  2-5s:    Drain (color drains from center to edges)")
    print("  5-8s:    Wait (all LEDs off)")
    print("  8-13s:   Pink Pulsate (vivid pink pulsating)")
    print("  13-23s:  CRAZY PARTY MODE! (rapid rainbow, strobe, sparkles)")
    print("  23s+:    Confetti (colorful explosion)")
    print()

    try:
        response = requests.post(url, timeout=5)
        if response.status_code == 200:
            data = response.json()
            print(f"✅ Success! {data.get('message', '')}")
            print(f"Duration: {data.get('duration_seconds', 13)} seconds")
            print()
            print("Watch the flamingo LEDs for the animation!")
        else:
            print(f"❌ Error: HTTP {response.status_code}")
            print(response.text)
    except requests.exceptions.ConnectionError:
        print(f"❌ Connection Error: Could not connect to {ip}")
        print("Make sure the Flamingo ESP is powered on and connected to the network")
    except requests.exceptions.Timeout:
        print(f"❌ Timeout: No response from {ip}")
    except Exception as e:
        print(f"❌ Error: {e}")

def check_status(ip):
    """Check current status of flamingo"""
    url = f"http://{ip}/status"

    try:
        response = requests.get(url, timeout=2)
        if response.status_code == 200:
            data = response.json()
            plan_names = ["IDLE", "SKIP", "SHOW", "SPECIAL", "MIXED_COLORS", "PARTY", "MEGA_SPECIAL"]
            current_plan = data.get('current_plan', 0)
            plan_name = plan_names[current_plan] if current_plan < len(plan_names) else "UNKNOWN"

            print(f"Current Status:")
            print(f"  Plan: {plan_name} ({current_plan})")
            print(f"  IP: {data.get('ip_address', 'N/A')}")
            print(f"  Uptime: {data.get('uptime', 0)}s")
            print()
    except:
        pass

if __name__ == "__main__":
    # Get IP from command line or use default
    ip = sys.argv[1] if len(sys.argv) > 1 else DEFAULT_IP

    # Check status first
    check_status(ip)

    # Trigger mega special
    trigger_mega_special(ip)
