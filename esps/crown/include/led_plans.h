#ifndef LED_PLANS_H
#define LED_PLANS_H

#include <FastLED.h>

// LED Configuration
#define NUM_LEDS 200
#define BRIGHTNESS 100
#define MAX_BRIGHTNESS 255

// Pin definition for single LED strip
#define LED_STRIP_PIN 2

// LED array for single strip
extern CRGB leds[NUM_LEDS];

// Lighting plan enumeration
enum LightingPlan {
    PLAN_IDLE,
    PLAN_BUTTON,
    PLAN_WIFI_FALLBACK
};

// LED Plan class to manage different lighting patterns
class LEDPlans {
private:
    LightingPlan currentPlan;
    unsigned long lastUpdate;
    uint8_t animationStep;
    uint8_t hue;
    uint8_t brightness;
    
    // Idle plan variables (Halo Mode)
    uint8_t idleHue;
    uint8_t idleBrightness;
    uint8_t haloPulseStep;
    
    // Button plan variables (Party Mode)
    unsigned long buttonStartTime;
    bool buttonActive;
    uint8_t partyHue;
    uint8_t partySpeed;
    uint8_t partyPattern;
    
    // WiFi fallback variables (Halo + Running Pixels)
    uint8_t fallbackHue;
    uint8_t fallbackBrightness;
    uint8_t fallbackPulseStep;
    uint8_t runningPixelPos;
    uint8_t runningPixelHue;

public:
    LEDPlans();
    void begin();
    void setPlan(LightingPlan plan);
    LightingPlan getCurrentPlan();
    void update();
    void clearAll();
    
private:
    void updateIdle();
    void updateButton();
    void updateWiFiFallback();
    
    // Helper functions
    void setAllLeds(CRGB color);
    void fadeToBlack(uint8_t amount);
    void addGlitter(fract8 chanceOfGlitter);
    void setPixel(uint16_t pixel, CRGB color);
    CRGB getHaloColor(uint8_t brightness);
    CRGB getPartyColor(uint8_t hue, uint8_t brightness);
};

#endif
