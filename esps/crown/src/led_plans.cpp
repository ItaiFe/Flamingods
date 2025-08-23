#include "led_plans.h"

// LED array (extern declaration)
CRGB leds[NUM_LEDS];

LEDPlans::LEDPlans() {
    currentPlan = PLAN_IDLE;
    lastUpdate = 0;
    animationStep = 0;
    hue = 0;
    brightness = BRIGHTNESS;
    
    // Initialize plan-specific variables
    idleHue = 32;  // Warm yellow/amber
    idleBrightness = 50;
    haloPulseStep = 0;
    
    buttonStartTime = 0;
    buttonActive = false;
    partyHue = 0;
    partySpeed = 0;
    partyPattern = 0;
    
    fallbackHue = 32;  // Same warm yellow/amber
    fallbackBrightness = 50;
    fallbackPulseStep = 0;
    runningPixelPos = 0;
    runningPixelHue = 0;
}

void LEDPlans::begin() {
    // Clear all LEDs
    clearAll();
    FastLED.show();
}

void LEDPlans::setPlan(LightingPlan plan) {
    currentPlan = plan;
    animationStep = 0;
    lastUpdate = millis();
    
    // Plan-specific initialization
    switch (plan) {
        case PLAN_IDLE:
            idleHue = 32;  // Warm yellow/amber
            idleBrightness = 50;
            haloPulseStep = 0;
            break;
        case PLAN_BUTTON:
            buttonStartTime = millis();
            buttonActive = true;
            partyHue = 0;
            partySpeed = 0;
            partyPattern = 0;
            break;
        case PLAN_WIFI_FALLBACK:
            fallbackHue = 32;  // Same warm yellow/amber
            fallbackBrightness = 50;
            fallbackPulseStep = 0;
            runningPixelPos = 0;
            runningPixelHue = 0;
            break;
    }
    
    Serial.printf("Switched to plan %d\n", plan);
}

LightingPlan LEDPlans::getCurrentPlan() {
    return currentPlan;
}

void LEDPlans::update() {
    unsigned long currentTime = millis();
    
    // Update based on current plan
    switch (currentPlan) {
        case PLAN_IDLE:
            updateIdle();
            break;
        case PLAN_BUTTON:
            updateButton();
            break;
        case PLAN_WIFI_FALLBACK:
            updateWiFiFallback();
            break;
    }
    
    lastUpdate = currentTime;
}

void LEDPlans::clearAll() {
    setAllLeds(CRGB::Black);
}

// IDLE PLAN: Test with Pure Blue
void LEDPlans::updateIdle() {
    haloPulseStep++;
    
    // Test with pure blue to debug color order
    uint8_t pulseBrightness = sin8(haloPulseStep * 2);  // Slow, gentle pulse
    setAllLeds(CRGB(0, 0, pulseBrightness));  // Pure blue (Blue only, no Red, no Green)
    
    // Reset pulse step to prevent overflow
    if (haloPulseStep > 127) {
        haloPulseStep = 0;
    }
}

// BUTTON PLAN: Crazy Colors Party Mode
void LEDPlans::updateButton() {
    unsigned long elapsed = millis() - buttonStartTime;
    
    // Auto-return to idle after 10 seconds
    if (elapsed > 10000) {
        currentPlan = PLAN_IDLE;
        setPlan(PLAN_IDLE);
        return;
    }
    
    partySpeed++;
    partyHue += 3;  // Fast color cycling
    
    // Different party patterns
    partyPattern = (elapsed / 1000) % 4;  // Change pattern every second
    
    switch (partyPattern) {
        case 0:  // Rainbow wave
            for (int i = 0; i < NUM_LEDS; i++) {
                uint8_t pixelHue = partyHue + (i * 3);
                leds[i] = getPartyColor(pixelHue, 255);
            }
            break;
            
        case 1:  // Color explosion
            for (int i = 0; i < NUM_LEDS; i++) {
                if (random8() < 128) {
                    leds[i] = getPartyColor(partyHue + random8(64), 255);
                } else {
                    leds[i] = CRGB::Black;
                }
            }
            break;
            
        case 2:  // Alternating colors
            for (int i = 0; i < NUM_LEDS; i++) {
                if (i % 2 == 0) {
                    leds[i] = getPartyColor(partyHue, 255);
                } else {
                    leds[i] = getPartyColor(partyHue + 128, 255);
                }
            }
            break;
            
        case 3:  // Sparkle effect
            setAllLeds(CRGB::Black);
            for (int i = 0; i < NUM_LEDS / 4; i++) {
                int pixel = random16(NUM_LEDS);
                leds[pixel] = getPartyColor(partyHue + random8(64), 255);
            }
            break;
    }
    
    // Add some glitter for extra party effect
    addGlitter(80);
}

// WIFI FALLBACK PLAN: Halo + Running Colorful Pixels
void LEDPlans::updateWiFiFallback() {
    fallbackPulseStep++;
    runningPixelPos++;
    runningPixelHue += 5;
    
    // Create the soft yellow pulsating halo background
    uint8_t pulseBrightness = sin8(fallbackPulseStep * 2);
    CRGB haloColor = getHaloColor(pulseBrightness);
    setAllLeds(haloColor);
    
    // Add running colorful pixels
    for (int i = 0; i < 3; i++) {  // 3 running pixels
        int pixelPos = (runningPixelPos + i * 30) % NUM_LEDS;
        uint8_t pixelHue = runningPixelHue + (i * 85);  // Different colors for each pixel
        
        // Create a bright, colorful running pixel
        CRGB pixelColor = getPartyColor(pixelHue, 255);
        
        // Add some fade effect
        if (pixelPos < NUM_LEDS) {
            leds[pixelPos] = pixelColor;
            // Fade the next few pixels for a trail effect
            for (int j = 1; j <= 3; j++) {
                if (pixelPos + j < NUM_LEDS) {
                    leds[pixelPos + j] = pixelColor;
                    leds[pixelPos + j].fadeToBlackBy(j * 50);
                }
            }
        }
    }
    
    // Reset counters to prevent overflow
    if (fallbackPulseStep > 127) {
        fallbackPulseStep = 0;
    }
    if (runningPixelPos >= NUM_LEDS) {
        runningPixelPos = 0;
    }
    if (runningPixelHue > 255) {
        runningPixelHue = 0;
    }
}

// Helper functions
void LEDPlans::setAllLeds(CRGB color) {
    for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = color;
    }
}

void LEDPlans::setPixel(uint16_t pixel, CRGB color) {
    if (pixel < NUM_LEDS) {
        leds[pixel] = color;
    }
}

void LEDPlans::fadeToBlack(uint8_t amount) {
    for (int i = 0; i < NUM_LEDS; i++) {
        leds[i].fadeToBlackBy(amount);
    }
}

void LEDPlans::addGlitter(fract8 chanceOfGlitter) {
    if (random8() < chanceOfGlitter) {
        leds[random16(NUM_LEDS)] += CRGB::White;
    }
}

// Get halo color (warm yellow/amber tones)
CRGB LEDPlans::getHaloColor(uint8_t brightness) {
    // Warm yellow/amber color with adjustable brightness
    return CRGB(brightness, brightness * 0.7, brightness * 0.3);
}

// Get party color (vibrant, saturated colors)
CRGB LEDPlans::getPartyColor(uint8_t hue, uint8_t brightness) {
    return CHSV(hue, 255, brightness);
}
