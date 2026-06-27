#!/bin/bash
# Test all 4 station-specific special modes

IP="${1:-192.168.1.200}"

echo "========================================="
echo "  Testing All 4 Station Special Modes"
echo "  Flamingo IP: $IP"
echo "========================================="
echo ""

echo "1️⃣  Station 1: FIRE WAVE 🔥"
echo "   Colors: Red, Orange, Yellow"
echo "   Effect: Flickering fire waves"
curl -X POST "http://$IP/station1-special" | jq
echo "   ⏳ Watch the animation for 10 seconds..."
sleep 11
echo ""

echo "2️⃣  Station 2: OCEAN STORM ⚡🌊"
echo "   Colors: Cyan, Blue, Purple"
echo "   Effect: Turbulent waves with lightning"
curl -X POST "http://$IP/station2-special" | jq
echo "   ⏳ Watch the animation for 10 seconds..."
sleep 11
echo ""

echo "3️⃣  Station 3: NORTHERN LIGHTS 🌌"
echo "   Colors: Green, Teal, Purple"
echo "   Effect: Aurora with shimmer"
curl -X POST "http://$IP/station3-special" | jq
echo "   ⏳ Watch the animation for 10 seconds..."
sleep 11
echo ""

echo "4️⃣  Station 4: ELECTRIC PULSE ⚡💜"
echo "   Colors: Magenta, Cyan"
echo "   Effect: Racing pulses"
curl -X POST "http://$IP/station4-special" | jq
echo "   ⏳ Watch the animation for 10 seconds..."
sleep 11
echo ""

echo "========================================="
echo "  ✅ All 4 Station Modes Tested!"
echo "========================================="
echo ""
echo "Bonus: Want to see the MEGA SPECIAL?"
echo "Run: curl -X POST http://$IP/mega-special"
