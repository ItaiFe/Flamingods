#ifndef LED_PLANS_H
#define LED_PLANS_H

#include <FastLED.h>

// LED Configuration
#define NUM_LEDS 500  // Total LEDs in strip
#define FIRST_LED 3  // Skip first 3 LEDs
#define ACTIVE_LEDS 453  // Active range ends here (last 47 are spare)
#define BRIGHTNESS 100
#define MAX_BRIGHTNESS 255

// Pin definition for single LED strip
#define LED_STRIP_PIN 4

// Relay control pin
#define RELAY_PIN 23

// LED array for single long strip
extern CRGB leds[NUM_LEDS];

// Lighting plan enumeration
enum LightingPlan {
    PLAN_IDLE,
    PLAN_SKIP,
    PLAN_SHOW,
    PLAN_SPECIAL
};

// LED Plan class to manage different lighting patterns
class LEDPlans {
private:
    LightingPlan currentPlan;
    unsigned long lastUpdate;
    uint8_t animationStep;
    uint8_t hue;
    uint8_t brightness;
    
    // Idle plan variables
    uint8_t idleHue;
    uint8_t idleBrightness;
    uint16_t cometPositions[3];  // Track 3 comets
    bool cometActive[3];  // Track which comets are active
    uint8_t cometTrailColors[6];
    uint8_t cometColorIndex;
    uint8_t cometSpeed;
    uint8_t cometRounds;
    uint16_t cometDelayCounter;

    // Sparkle effect variables
    struct Sparkle {
        uint16_t position;
        uint8_t brightness;
        uint8_t fadeSpeed;
        bool active;
    };
    static const uint8_t MAX_SPARKLES = 8;
    Sparkle sparkles[8];
    
    // Skip plan variables
    unsigned long skipStartTime;
    bool skipActive;
    
    // Show plan variables
    uint8_t showPattern;
    uint8_t showSpeed;

    // Color splash variables
    struct ColorSplash {
        uint16_t position;
        uint8_t hue;
        uint8_t radius;
        uint8_t maxRadius;
        bool active;
        int8_t dripPos;  // For dripping effect
        uint8_t speed;  // Individual splash speed
    };
    static const uint8_t MAX_SPLASHES = 12;  // More splashes!
    ColorSplash splashes[12];
    
    // Special plan variables
    uint8_t specialEffect;
    unsigned long specialStartTime;
    uint8_t partyHue;
    uint16_t partyWavePos;
    int8_t partyWaveDir;
    uint8_t partyBurstTimer;
    uint16_t partyBurstPos;

public:
    LEDPlans();
    void begin();
    void setPlan(LightingPlan plan);
    LightingPlan getCurrentPlan();
    void update();
    void clearAll();
    
private:
    void updateIdle();
    void updateSkip();
    void updateShow();
    void updateSpecial();
    
    // Helper functions
    void setAllLeds(CRGB color);
    void fadeToBlack(uint8_t amount);
    void addGlitter(fract8 chanceOfGlitter);
};

#endif
