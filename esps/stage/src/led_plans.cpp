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
    idleHue = 0;
    idleBrightness = 50;
    skipStartTime = 0;
    skipActive = false;
    showPattern = 0;
    showSpeed = 0;
    specialEffect = 0;
    specialStartTime = 0;
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
            idleHue = 0;
            idleBrightness = 50;
            break;
        case PLAN_SKIP:
            skipStartTime = millis();
            skipActive = true;
            break;
        case PLAN_SHOW:
            showPattern = 0;
            showSpeed = 0;
            break;
        case PLAN_SPECIAL:
            specialEffect = 0;
            specialStartTime = millis();
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
        case PLAN_SKIP:
            updateSkip();
            break;
        case PLAN_SHOW:
            updateShow();
            break;
        case PLAN_SPECIAL:
            updateSpecial();
            break;
    }
    
    lastUpdate = currentTime;
}

void LEDPlans::clearAll() {
    setAllLeds(CRGB::Black);
}

// IDLE PLAN: Pulsing green pattern
void LEDPlans::updateIdle() {
    static uint8_t pulseStep = 0;
    pulseStep++;
    
    // Create a pulsing green pattern from off to full brightness
    uint8_t brightness = sin8(pulseStep * 2);  // Pulse from 0-255 brightness
    setAllLeds(CRGB(0, brightness, 0));  // Pure green with pulsing brightness
    
    // Medium pulse speed for idle mode
    if (pulseStep > 127) {
        pulseStep = 0;  // Reset to prevent overflow
    }
}

// SKIP PLAN: White flashes
void LEDPlans::updateSkip() {
    unsigned long elapsed = millis() - skipStartTime;
    
    if (elapsed < 150) {
        // Bright white flash
        setAllLeds(CRGB::White);
    } else if (elapsed < 300) {
        // Quick fade to black
        setAllLeds(CRGB::Black);
    } else if (elapsed < 450) {
        // Second white flash
        setAllLeds(CRGB::White);
    } else if (elapsed < 600) {
        // Fade to black
        setAllLeds(CRGB::Black);
    } else {
        // Return to idle
        currentPlan = PLAN_IDLE;
        setPlan(PLAN_IDLE);
    }
}

// SHOW PLAN: Pulsing red pattern
void LEDPlans::updateShow() {
    static uint8_t pulseStep = 0;
    pulseStep++;
    
    // Create a pulsing red pattern from off to full brightness
    uint8_t brightness = sin8(pulseStep * 3);  // Pulse from 0-255 brightness
    setAllLeds(CRGB(brightness, 0, 0));  // Pure red with pulsing brightness
    
    // Fast pulse speed for show mode
    if (pulseStep > 85) {
        pulseStep = 0;  // Reset to prevent overflow
    }
}

// SPECIAL PLAN: Pulsing blue pattern
void LEDPlans::updateSpecial() {
    static uint8_t pulseStep = 0;
    pulseStep++;
    
    // Create a pulsing blue pattern from off to full brightness
    uint8_t brightness = sin8(pulseStep * 2);  // Pulse from 0-255 brightness
    setAllLeds(CRGB(0, 0, brightness));  // Pure blue with pulsing brightness
    
    // Slow pulse speed for special mode
    if (pulseStep > 127) {
        pulseStep = 0;  // Reset to prevent overflow
    }
}

// Helper functions
void LEDPlans::setAllLeds(CRGB color) {
    for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = color;
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
