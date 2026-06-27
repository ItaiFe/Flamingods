#include "led_plans.h"

// Single LED array for long strip (extern declaration)
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

    // Initialize 3 comets
    for (uint8_t i = 0; i < 3; i++) {
        cometPositions[i] = 0;
        cometActive[i] = false;
    }
    cometActive[0] = true;  // Start with first comet active

    cometColorIndex = 0;
    cometSpeed = random8(1, 4);  // Random speed between 1-3
    cometRounds = 0;
    cometDelayCounter = 0;

    // Initialize comet trail colors (pink shades only)
    cometTrailColors[0] = 224;  // Deep pink/magenta
    cometTrailColors[1] = 230;  // Pink-magenta
    cometTrailColors[2] = 236;  // Hot pink
    cometTrailColors[3] = 240;  // Pink
    cometTrailColors[4] = 245;  // Light pink
    cometTrailColors[5] = 235;  // Pink

    // Initialize sparkles
    for (uint8_t i = 0; i < MAX_SPARKLES; i++) {
        sparkles[i].active = false;
        sparkles[i].position = 0;
        sparkles[i].brightness = 0;
        sparkles[i].fadeSpeed = 0;
    }

    skipStartTime = 0;
    skipActive = false;
    showPattern = 0;
    showSpeed = 0;

    // Initialize color splashes
    for (uint8_t i = 0; i < MAX_SPLASHES; i++) {
        splashes[i].active = false;
        splashes[i].position = 0;
        splashes[i].hue = 0;
        splashes[i].radius = 0;
        splashes[i].maxRadius = 0;
        splashes[i].dripPos = 0;
        splashes[i].speed = 0;
    }

    specialEffect = 0;
    specialStartTime = 0;
    partyHue = 0;
    partyWavePos = 0;
    partyWaveDir = 1;
    partyBurstTimer = 0;
    partyBurstPos = 0;
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
            specialStartTime = millis();  // Track show start time
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

// IDLE PLAN: 3 dual warm comets splitting from position 227 outward with sparkles
void LEDPlans::updateIdle() {
    const uint16_t SPLIT_POINT = 228;
    const uint8_t COMET_LENGTH = 8;  // Much smaller (was 15)
    const uint8_t COMET_STAGGER = 20;  // Faster launches (was 30)

    // Fade all LEDs slightly for trail effect
    fadeToBlack(40);

    // Keep first 3 LEDs always off
    for (uint16_t i = 0; i < FIRST_LED; i++) {
        leds[i] = CRGB::Black;
    }

    // Keep last spare LEDs always off
    for (uint16_t i = ACTIVE_LEDS; i < NUM_LEDS; i++) {
        leds[i] = CRGB::Black;
    }

    // Update sparkles first (so they appear behind/get swallowed by comets)
    for (uint8_t i = 0; i < MAX_SPARKLES; i++) {
        if (sparkles[i].active) {
            // Fade the sparkle
            if (sparkles[i].brightness > sparkles[i].fadeSpeed) {
                sparkles[i].brightness -= sparkles[i].fadeSpeed;
            } else {
                sparkles[i].brightness = 0;
                sparkles[i].active = false;
            }

            // Draw sparkle (bluish-white) - only in active range
            if (sparkles[i].brightness > 0 &&
                sparkles[i].position >= FIRST_LED &&
                sparkles[i].position < ACTIVE_LEDS) {
                leds[sparkles[i].position] = CRGB(
                    sparkles[i].brightness,
                    sparkles[i].brightness,
                    255
                );
            }
        }
    }

    // Randomly create new sparkles (only in active LED range) - less frequent for smaller strip
    if (random8() < 10) {  // Reduced frequency
        for (uint8_t i = 0; i < MAX_SPARKLES; i++) {
            if (!sparkles[i].active) {
                sparkles[i].active = true;
                sparkles[i].position = random16(FIRST_LED, ACTIVE_LEDS);
                sparkles[i].brightness = random8(150, 255);
                sparkles[i].fadeSpeed = random8(5, 15);
                break;
            }
        }
    }

    // Launch comets with staggered timing
    cometDelayCounter++;
    if (cometDelayCounter >= COMET_STAGGER && !cometActive[1]) {
        cometActive[1] = true;
    }
    if (cometDelayCounter >= COMET_STAGGER * 2 && !cometActive[2]) {
        cometActive[2] = true;
    }

    // Draw all active comets
    for (uint8_t c = 0; c < 3; c++) {
        if (!cometActive[c]) continue;

        for (uint8_t i = 0; i < COMET_LENGTH; i++) {
            int16_t trailOffset = i;

            // Calculate brightness falloff for trail
            uint8_t brightness = 255 - (i * (255 / COMET_LENGTH));

            // Right-moving comet (from 227 towards 452)
            int16_t rightPos = SPLIT_POINT + cometPositions[c] - trailOffset;
            if (rightPos >= FIRST_LED && rightPos < ACTIVE_LEDS) {
                if (i == 0) {
                    // Lead comet: bright pink
                    CRGB cometColor = CHSV(240, 255, 255);  // Pure bright pink
                    cometColor.nscale8(brightness);
                    leds[rightPos] += cometColor;
                } else {
                    // Trail: varying pink shades
                    uint8_t colorIndex = (cometColorIndex + (i / 3) + c) % 6;
                    CRGB trailColor = CHSV(cometTrailColors[colorIndex], 255, brightness);
                    leds[rightPos] += trailColor;
                }
            }

            // Left-moving comet (from 227 towards 3)
            int16_t leftPos = SPLIT_POINT - cometPositions[c] + trailOffset;
            if (leftPos >= FIRST_LED && leftPos < ACTIVE_LEDS) {
                if (i == 0) {
                    // Lead comet: bright pink
                    CRGB cometColor = CHSV(240, 255, 255);  // Pure bright pink
                    cometColor.nscale8(brightness);
                    leds[leftPos] += cometColor;
                } else {
                    // Trail: varying pink shades
                    uint8_t colorIndex = (cometColorIndex + (i / 3) + c) % 6;
                    CRGB trailColor = CHSV(cometTrailColors[colorIndex], 255, brightness);
                    leds[leftPos] += trailColor;
                }
            }
        }

        // Move this comet forward using dynamic speed
        cometPositions[c] += cometSpeed;
    }

    // Check if all comets have completed (left comet reaches position 3)
    bool allComplete = true;
    for (uint8_t c = 0; c < 3; c++) {
        if (cometActive[c] && SPLIT_POINT - cometPositions[c] > FIRST_LED - COMET_LENGTH) {
            allComplete = false;
            break;
        }
    }

    // Reset when all comets complete
    if (allComplete) {
        // Reset all comets
        for (uint8_t i = 0; i < 3; i++) {
            cometPositions[i] = 0;
            cometActive[i] = false;
        }
        cometActive[0] = true;  // Start first comet again
        cometDelayCounter = 0;
        cometRounds++;

        // Change trail colors slightly for next cycle (keep within pink range: 220-250)
        cometColorIndex = (cometColorIndex + 1) % 6;
        for (uint8_t i = 0; i < 6; i++) {
            // Vary within pink hue range (220-250)
            cometTrailColors[i] = 220 + random8(0, 30);
        }

        // Randomize speed every 3-5 rounds
        if (cometRounds >= random8(3, 6)) {
            cometSpeed = random8(1, 4);  // Random speed between 1-3
            cometRounds = 0;
        }
    }
}

// SKIP PLAN: White flashes (2 times) then move to show
void LEDPlans::updateSkip() {
    unsigned long elapsed = millis() - skipStartTime;

    // Flash pattern: 2 white flashes with quick fades, then go to SHOW
    if (elapsed < 150) {
        // First white flash
        setAllLeds(CRGB::White);
    } else if (elapsed < 300) {
        // Quick fade to black
        setAllLeds(CRGB::Black);
    } else if (elapsed < 450) {
        // Second white flash
        setAllLeds(CRGB::White);
    } else if (elapsed < 600) {
        // Final fade to black
        setAllLeds(CRGB::Black);
    } else {
        // Move to show mode
        currentPlan = PLAN_SHOW;
        setPlan(PLAN_SHOW);
    }
}

// SHOW PLAN: RAINBOW MOVING BARS WITH PARTY SPARKLES! 🌈✨🎉
void LEDPlans::updateShow() {
    // Keep first 3 and last spare LEDs off
    for (uint16_t i = 0; i < FIRST_LED; i++) {
        leds[i] = CRGB::Black;
    }
    for (uint16_t i = ACTIVE_LEDS; i < NUM_LEDS; i++) {
        leds[i] = CRGB::Black;
    }

    // Clear for fresh bars each frame
    for (uint16_t i = FIRST_LED; i < ACTIVE_LEDS; i++) {
        leds[i] = CRGB::Black;
    }

    static uint16_t barPosition = 0;
    static uint8_t rainbowOffset = 0;

    // === RAINBOW MOVING BARS ===
    const uint8_t NUM_BARS = 8;  // 8 rainbow bars
    const uint8_t BAR_WIDTH = 30;  // Width of each bar
    const uint8_t BAR_SPACING = 10;  // Gap between bars

    rainbowOffset += 2;  // Rotate rainbow colors
    barPosition += 3;  // Move bars across strip

    for (uint8_t bar = 0; bar < NUM_BARS; bar++) {
        // Calculate bar color (rotating rainbow)
        uint8_t barHue = (bar * 256 / NUM_BARS) + rainbowOffset;

        // Calculate bar position (with wrapping)
        int16_t barStart = (barPosition + bar * (BAR_WIDTH + BAR_SPACING)) % (ACTIVE_LEDS - FIRST_LED + BAR_WIDTH + BAR_SPACING);
        barStart = barStart + FIRST_LED - (BAR_WIDTH + BAR_SPACING);

        // Draw the bar
        for (uint8_t w = 0; w < BAR_WIDTH; w++) {
            int16_t pos = barStart + w;

            // Wrap around
            while (pos < FIRST_LED) pos += (ACTIVE_LEDS - FIRST_LED);
            while (pos >= ACTIVE_LEDS) pos -= (ACTIVE_LEDS - FIRST_LED);

            if (pos >= FIRST_LED && pos < ACTIVE_LEDS) {
                // Gradient within bar for depth
                uint8_t brightness = 255 - (abs((int)w - (int)(BAR_WIDTH / 2)) * 10);
                leds[pos] = CHSV(barHue, 255, brightness);
            }
        }
    }

    // === PARTY SPARKLES - LOTS OF THEM! ===
    for (uint8_t i = 0; i < 30; i++) {  // 30 sparkles per frame!
        if (random8() < 200) {  // 78% chance each
            uint16_t sparklePos = random16(FIRST_LED, ACTIVE_LEDS);
            uint8_t sparkleHue = random8();  // Random rainbow color
            uint8_t sparkleBright = random8(150, 255);

            // Bright sparkles that stand out
            leds[sparklePos] += CHSV(sparkleHue, random8(200, 255), sparkleBright);
        }
    }

    // === GLITTER ACCENTS ===
    // Extra bright white sparkles for extra party feel
    for (uint8_t i = 0; i < 10; i++) {
        if (random8() < 120) {
            uint16_t glitterPos = random16(FIRST_LED, ACTIVE_LEDS);
            leds[glitterPos] += CRGB(200, 200, 200);  // Bright white
        }
    }

    // === SHIMMER WAVES ===
    // Pulsing shimmer across the strip
    static uint8_t shimmerPhase = 0;
    shimmerPhase += 5;
    for (uint16_t i = FIRST_LED; i < ACTIVE_LEDS; i += 15) {
        uint8_t shimmerVal = sin8(shimmerPhase + (i * 4));
        if (shimmerVal > 220) {
            leds[i] += CRGB(shimmerVal, shimmerVal, shimmerVal);
        }
    }
}

// SPECIAL PLAN: MAXIMUM IMPACT STROBE PARTY MODE! ⚡💥🔥
// WARNING: EXTREME FLASHING - SHORT INTENSE BURSTS!
void LEDPlans::updateSpecial() {
    // Keep first 3 and last spare LEDs off
    for (uint16_t i = 0; i < FIRST_LED; i++) {
        leds[i] = CRGB::Black;
    }
    for (uint16_t i = ACTIVE_LEDS; i < NUM_LEDS; i++) {
        leds[i] = CRGB::Black;
    }

    // Minimal fade for MAXIMUM hard flashing
    fadeToBlack(50);

    // Ultra-fast spinning rainbow hue
    partyHue += 8;

    // === EFFECT 0: EXTREME FULL-STRIP STROBE (Primary Impact) ===
    static uint8_t strobeCounter = 0;
    strobeCounter++;

    // HARD STROBE: On/Off every 2 frames
    if (strobeCounter % 2 == 0) {
        uint8_t strobeHue = random8();
        CRGB strobeColor = CHSV(strobeHue, 255, 255);
        for (uint16_t i = FIRST_LED; i < ACTIVE_LEDS; i++) {
            leds[i] = strobeColor;  // FULL BRIGHTNESS INSTANT
        }
    }

    // === EFFECT 1: Rapid Fire Color Snaps ===
    if (random8() < 80) {  // 31% chance - very frequent!
        uint8_t snapHue = random8();
        for (uint16_t i = FIRST_LED; i < ACTIVE_LEDS; i++) {
            leds[i] = CHSV(snapHue, 255, 255);
        }
    }

    // === EFFECT 2: Ultra-Intense Throbbing ===
    uint8_t throb = beatsin8(50, 0, 255);  // Extreme fast throbbing
    for (uint16_t i = FIRST_LED; i < ACTIVE_LEDS; i++) {
        uint8_t bgHue = partyHue + (i / 2);
        leds[i] += CHSV(bgHue, 255, throb);
    }

    // === EFFECT 2: Bouncing Rainbow Wave ===
    uint8_t waveWidth = 50;
    for (uint16_t i = 0; i < waveWidth; i++) {
        int16_t pos = partyWavePos + i;
        if (pos >= FIRST_LED && pos < ACTIVE_LEDS) {
            uint8_t waveHue = partyHue + (i * 5);
            uint8_t waveBrightness = beatsin8(40, 150, 255);  // Pulsing wave
            leds[pos] += CHSV(waveHue, 255, waveBrightness);
        }
    }

    // Move wave faster and bounce at edges
    partyWavePos += partyWaveDir * 6;
    if (partyWavePos >= ACTIVE_LEDS - waveWidth) {
        partyWavePos = ACTIVE_LEDS - waveWidth;
        partyWaveDir = -1;
    }
    if (partyWavePos <= FIRST_LED) {
        partyWavePos = FIRST_LED;
        partyWaveDir = 1;
    }

    // === EFFECT 3: Multiple Random Color Bursts ===
    partyBurstTimer++;
    if (partyBurstTimer >= 15) {  // More frequent bursts!
        partyBurstTimer = 0;
        partyBurstPos = random16(FIRST_LED, ACTIVE_LEDS);
    }

    // Draw expanding burst with throbbing
    if (partyBurstTimer < 20) {
        uint8_t burstSize = partyBurstTimer * 3;
        uint8_t burstHue = random8();
        uint8_t burstThrob = beatsin8(50, 150, 255);
        for (uint8_t i = 0; i < burstSize; i++) {
            int16_t leftPos = partyBurstPos - i;
            int16_t rightPos = partyBurstPos + i;
            uint8_t burstBright = (255 - (i * 8)) * burstThrob / 255;

            if (leftPos >= FIRST_LED && leftPos < ACTIVE_LEDS) {
                leds[leftPos] += CHSV(burstHue, 255, burstBright);
            }
            if (rightPos >= FIRST_LED && rightPos < ACTIVE_LEDS) {
                leds[rightPos] += CHSV(burstHue, 255, burstBright);
            }
        }
    }

    // === EFFECT 4: INSANE Strobing Color Sections ===
    static uint8_t strobeTimer = 0;
    strobeTimer++;
    if (strobeTimer % 2 == 0) {  // EVERY OTHER FRAME - MAXIMUM STROBE!
        uint8_t sectionSize = (ACTIVE_LEDS - FIRST_LED) / 10;  // 10 sections!
        for (uint8_t s = 0; s < 10; s++) {
            uint16_t sectionStart = FIRST_LED + (s * sectionSize);
            uint8_t sectionHue = partyHue + (s * 25);
            // Alternate sections full bright or off
            uint8_t sectionBright = (s % 2 == 0) ? 255 : 0;
            for (uint16_t i = 0; i < sectionSize; i++) {
                uint16_t pos = sectionStart + i;
                if (pos < ACTIVE_LEDS) {
                    leds[pos] = CHSV(sectionHue, 255, sectionBright);
                }
            }
        }
    }

    // === EFFECT 5: EXTREME Confetti Sparkles ===
    for (uint8_t i = 0; i < 25; i++) {  // 25 SPARKLES PER FRAME!!!
        if (random8() < 250) {  // 98% chance!
            uint16_t pos = random16(FIRST_LED, ACTIVE_LEDS);
            uint8_t sparkleHue = random8();
            leds[pos] = CHSV(sparkleHue, 255, 255);  // FULL BRIGHTNESS
        }
    }

    // === EFFECT 6: Multiple Pulsing Rainbow Chases ===
    static uint8_t chasePos = 0;
    chasePos += 3;
    for (uint16_t i = FIRST_LED; i < ACTIVE_LEDS; i += 15) {  // More chase dots!
        uint16_t pos = (i + chasePos) % (ACTIVE_LEDS - FIRST_LED) + FIRST_LED;
        if (pos < ACTIVE_LEDS) {
            uint8_t chaseHue = partyHue + (pos * 3);
            uint8_t chaseBright = beatsin8(40, 150, 255);
            leds[pos] += CHSV(chaseHue, 255, chaseBright);
        }
    }

    // === EFFECT 7: CONSTANT Random Color Flashes ===
    if (random8() < 100) {  // 39% chance - VERY frequent!
        uint16_t flashPos = random16(FIRST_LED, ACTIVE_LEDS);
        uint16_t flashWidth = random8(15, 50);
        CRGB flashColor = CHSV(random8(), 255, 255);
        for (uint16_t i = 0; i < flashWidth; i++) {
            if (flashPos + i < ACTIVE_LEDS) {
                leds[flashPos + i] = flashColor;  // Full replace!
            }
        }
    }

    // === EFFECT 8: MEGA Lightning-Like White Flashes ===
    if (random8() < 25) {  // Much more frequent!
        for (uint16_t i = FIRST_LED; i < ACTIVE_LEDS; i++) {
            leds[i] = CRGB(255, 255, 255);  // FULL WHITE BLAST
        }
    }

    // === EFFECT 9: HARD Binary Color Snaps ===
    static uint8_t binaryTimer = 0;
    binaryTimer++;
    if (binaryTimer % 3 == 0) {  // Every 3 frames
        for (uint16_t i = FIRST_LED; i < ACTIVE_LEDS; i += 10) {
            uint8_t snapHue = random8();
            leds[i] = CHSV(snapHue, 255, 255);
            if (i + 1 < ACTIVE_LEDS) leds[i + 1] = CHSV(snapHue, 255, 255);
            if (i + 2 < ACTIVE_LEDS) leds[i + 2] = CHSV(snapHue, 255, 255);
        }
    }

    // === EFFECT 10: Epileptic Alternating Colors ===
    if (strobeCounter % 4 < 2) {
        uint8_t altColor1 = partyHue;
        uint8_t altColor2 = partyHue + 128;
        for (uint16_t i = FIRST_LED; i < ACTIVE_LEDS; i++) {
            if (i % 4 < 2) {
                leds[i] = CHSV(altColor1, 255, 255);
            } else {
                leds[i] = CHSV(altColor2, 255, 255);
            }
        }
    }

    // === EFFECT 11: Random Full-Strip Color BLASTS ===
    if (random8() < 40) {  // 15% chance
        uint8_t blastHue = random8();
        for (uint16_t i = FIRST_LED; i < ACTIVE_LEDS; i++) {
            leds[i] = CHSV(blastHue, 255, 255);
        }
    }

    // === EFFECT 12: Strobe Kill-Switch (random blackouts) ===
    if (random8() < 20) {  // 8% chance for dramatic blackout
        for (uint16_t i = FIRST_LED; i < ACTIVE_LEDS; i++) {
            leds[i] = CRGB::Black;
        }
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
        int pos = random16(NUM_LEDS);
        leds[pos] += CRGB::White;
    }
}
