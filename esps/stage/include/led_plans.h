#ifndef LED_PLANS_H
#define LED_PLANS_H

#include <FastLED.h>

// LED Configuration
#define NUM_LEDS 100
#define BRIGHTNESS 100
#define MAX_BRIGHTNESS 255

// Pin definition for single LED strip
#define LED_STRIP_PIN 4

// LED array for single strip
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
    
    // Skip plan variables
    unsigned long skipStartTime;
    bool skipActive;
    
    // Show plan variables
    uint8_t showPattern;
    uint8_t showSpeed;
    
    // Special plan variables
    uint8_t specialEffect;
    unsigned long specialStartTime;

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
