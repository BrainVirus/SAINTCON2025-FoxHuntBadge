#include "led_meter.h"

LEDMeter Meter;

static const char* THEME_NAMES[THEME_COUNT] = {
    "CYBER THERMAL",
    "SAINTCON GOLD",
    "TOXIC MATRIX",
    "CYBERPUNK NEON",
    "LAVA MAGMA",
    "ICE FROST",
    "BLOODHOUND RED",
    "RAINBOW SPECTRUM"
};

LEDMeter::LEDMeter()
    : _strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800),
      _mode(METER_MODE_IDLE),
      _brightness(75),
      _currentTheme(THEME_CYBER_THERMAL),
      _currentRssi(-100),
      _signalPercent(0.0f),
      _activeLevels(0),
      _peakLevel(0),
      _peakHoldMs(0),
      _lastUpdateMs(0),
      _animStep(0)
{
    memset(_frameBuffer, 0, sizeof(_frameBuffer));
}

void LEDMeter::begin() {
    loadSettings();
    _strip.begin();
    _strip.setBrightness(_brightness);
    _strip.clear();
    _strip.show();
}

void LEDMeter::loadSettings() {
    Preferences prefs;
    if (prefs.begin("sc25_fox", true)) {
        _brightness = prefs.getUChar("bright", 75);
        uint8_t th = prefs.getUChar("theme", (uint8_t)THEME_CYBER_THERMAL);
        if (th < THEME_COUNT) {
            _currentTheme = (ColorTheme)th;
        }
        prefs.end();
    }
}

void LEDMeter::saveSettings() {
    Preferences prefs;
    if (prefs.begin("sc25_fox", false)) {
        prefs.putUChar("bright", _brightness);
        prefs.putUChar("theme", (uint8_t)_currentTheme);
        prefs.end();
    }
}

void LEDMeter::setMode(MeterMode mode) {
    _mode = mode;
    _animStep = 0;
    if (_mode == METER_MODE_IDLE) {
        _activeLevels = 0;
        _peakLevel = 0;
    }
}

void LEDMeter::setSignal(int rssi, float qualityPercent) {
    _currentRssi = rssi;
    _signalPercent = constrain(qualityPercent, 0.0f, 100.0f);

    int levels = (int)((_signalPercent * 10.0f) / 100.0f + 0.5f);
    if (levels < 0) levels = 0;
    if (levels > 10) levels = 10;
    _activeLevels = levels;

    if (_activeLevels >= _peakLevel) {
        _peakLevel = _activeLevels;
        _peakHoldMs = millis() + 700;
    }
}

void LEDMeter::setBrightness(uint8_t brightness) {
    _brightness = constrain(brightness, (uint8_t)10, (uint8_t)255);
    _strip.setBrightness(_brightness);
}

void LEDMeter::adjustBrightness(int delta) {
    int val = (int)_brightness + delta;
    if (val < 10) val = 10;
    if (val > 255) val = 255;
    setBrightness((uint8_t)val);
}

void LEDMeter::setTheme(ColorTheme theme) {
    if (theme < THEME_COUNT) {
        _currentTheme = theme;
    }
}

const char* LEDMeter::getThemeName(ColorTheme theme) const {
    if (theme < THEME_COUNT) {
        return THEME_NAMES[theme];
    }
    return "UNKNOWN";
}

void LEDMeter::nextTheme() {
    _currentTheme = (ColorTheme)((_currentTheme + 1) % THEME_COUNT);
}

void LEDMeter::prevTheme() {
    _currentTheme = (ColorTheme)((_currentTheme + THEME_COUNT - 1) % THEME_COUNT);
}

RGBColor LEDMeter::getLed(uint8_t index) const {
    if (index < NUM_LEDS) return _frameBuffer[index];
    return RGBColor(0, 0, 0);
}

RGBColor LEDMeter::getLevelColor(int level) const {
    return getThemeColor(_currentTheme, level);
}

RGBColor LEDMeter::getThemeColor(ColorTheme theme, int level) {
    level = constrain(level, 0, 9);
    switch (theme) {
        case THEME_CYBER_THERMAL:
            switch (level) {
                case 0: return RGBColor(0, 140, 255);    // Deep Cyan
                case 1: return RGBColor(0, 210, 255);    // Bright Cyan
                case 2: return RGBColor(0, 255, 180);    // Aqua Teal
                case 3: return RGBColor(0, 255, 50);     // Radar Green
                case 4: return RGBColor(110, 255, 0);    // Lime Green
                case 5: return RGBColor(245, 190, 0);    // SaintCon Gold
                case 6: return RGBColor(255, 130, 0);    // Amber Orange
                case 7: return RGBColor(255, 60, 0);     // Hot Orange
                case 8: return RGBColor(255, 10, 0);     // Crimson Red
                case 9: return RGBColor(255, 255, 220);  // White-Hot Tip
            }
            break;

        case THEME_SAINTCON_GOLD:
            switch (level) {
                case 0: return RGBColor(120, 60, 0);     // Deep Amber
                case 1: return RGBColor(160, 85, 0);     // Warm Bronze
                case 2: return RGBColor(200, 120, 0);    // Ochre Gold
                case 3: return RGBColor(235, 160, 0);    // SaintCon Gold
                case 4: return RGBColor(255, 190, 0);    // Bright Gold
                case 5: return RGBColor(255, 215, 20);   // Radiant Yellow
                case 6: return RGBColor(255, 235, 60);   // Sunlight
                case 7: return RGBColor(255, 245, 120);  // Incandescent Yellow
                case 8: return RGBColor(255, 250, 180);  // Warm White
                case 9: return RGBColor(255, 255, 255);  // Pure White
            }
            break;

        case THEME_TOXIC_MATRIX:
            switch (level) {
                case 0: return RGBColor(0, 60, 10);      // Deep Terminal Green
                case 1: return RGBColor(0, 100, 20);     // Dark Emerald
                case 2: return RGBColor(0, 150, 30);     // Forest Green
                case 3: return RGBColor(0, 200, 40);     // Radar Green
                case 4: return RGBColor(0, 255, 50);     // Electric Green
                case 5: return RGBColor(60, 255, 0);     // Acid Lime
                case 6: return RGBColor(130, 255, 0);    // Fluorescent Lime
                case 7: return RGBColor(190, 255, 0);    // Toxic Yellow-Green
                case 8: return RGBColor(240, 255, 80);   // Glowing Chartreuse
                case 9: return RGBColor(240, 255, 240);  // Toxic White Core
            }
            break;

        case THEME_CYBERPUNK_NEON:
            switch (level) {
                case 0: return RGBColor(0, 70, 220);     // Neon Blue
                case 1: return RGBColor(40, 10, 220);    // Deep Violet
                case 2: return RGBColor(90, 0, 220);     // Electric Purple
                case 3: return RGBColor(160, 0, 220);    // Neon Indigo
                case 4: return RGBColor(220, 0, 200);    // Magenta
                case 5: return RGBColor(255, 0, 140);    // Hot Pink
                case 6: return RGBColor(255, 0, 70);     // Cyber Rose
                case 7: return RGBColor(255, 80, 0);     // Laser Coral
                case 8: return RGBColor(255, 180, 0);    // Cyber Yellow
                case 9: return RGBColor(255, 255, 255);  // Pure White
            }
            break;

        case THEME_LAVA_MAGMA:
            switch (level) {
                case 0: return RGBColor(70, 0, 0);       // Dark Smolder Red
                case 1: return RGBColor(130, 0, 0);      // Deep Crimson
                case 2: return RGBColor(190, 0, 0);      // Blood Red
                case 3: return RGBColor(230, 20, 0);     // Fiery Red
                case 4: return RGBColor(255, 50, 0);     // Blaze Orange
                case 5: return RGBColor(255, 100, 0);    // Hot Lava Orange
                case 6: return RGBColor(255, 150, 0);    // Glowing Amber
                case 7: return RGBColor(255, 200, 20);   // Molten Gold
                case 8: return RGBColor(255, 235, 100);  // Incandescent Core
                case 9: return RGBColor(255, 255, 230);  // White-Hot Eruption
            }
            break;

        case THEME_ICE_FROST:
            switch (level) {
                case 0: return RGBColor(0, 20, 70);      // Deep Abyss Navy
                case 1: return RGBColor(0, 45, 130);     // Midnight Blue
                case 2: return RGBColor(0, 85, 190);     // Cobalt Blue
                case 3: return RGBColor(0, 140, 225);    // Glacier Blue
                case 4: return RGBColor(0, 190, 240);    // Arctic Cyan
                case 5: return RGBColor(0, 235, 255);    // Crystal Cyan
                case 6: return RGBColor(60, 245, 255);   // Ice Diamond
                case 7: return RGBColor(140, 250, 255);  // Frost Glaze
                case 8: return RGBColor(210, 252, 255);  // Pure Glacial Frost
                case 9: return RGBColor(255, 255, 255);  // Subzero White
            }
            break;

        case THEME_BLOODHOUND_RED:
            switch (level) {
                case 0: return RGBColor(40, 0, 0);       // Dim Ruby
                case 1: return RGBColor(80, 0, 0);       // Low Scent Red
                case 2: return RGBColor(130, 0, 0);      // Medium Ruby
                case 3: return RGBColor(175, 0, 0);      // Warm Crimson
                case 4: return RGBColor(215, 0, 0);      // Hot Scent Red
                case 5: return RGBColor(255, 10, 0);     // High Alert Red
                case 6: return RGBColor(255, 45, 0);     // Warning Scarlet
                case 7: return RGBColor(255, 95, 0);     // Target Lock Vermillion
                case 8: return RGBColor(255, 160, 20);   // Target Acquired Orange
                case 9: return RGBColor(255, 255, 255);  // Killshot White
            }
            break;

        case THEME_RAINBOW:
            switch (level) {
                case 0: return RGBColor(120, 0, 220);    // Violet
                case 1: return RGBColor(0, 40, 230);     // Indigo
                case 2: return RGBColor(0, 160, 240);    // Cyan
                case 3: return RGBColor(0, 220, 140);    // Emerald
                case 4: return RGBColor(40, 240, 0);     // Green
                case 5: return RGBColor(180, 240, 0);    // Lime
                case 6: return RGBColor(255, 210, 0);    // Yellow
                case 7: return RGBColor(255, 110, 0);    // Orange
                case 8: return RGBColor(255, 20, 0);     // Crimson
                case 9: return RGBColor(255, 255, 255);  // Pure White
            }
            break;

        default:
            break;
    }
    return RGBColor(255, 255, 255);
}

void LEDMeter::renderIdleAnimation() {
    // Gentle breathing ambient pulse along the throat / base using theme color
    float breath = (sinf(_animStep * 0.08f) + 1.0f) * 0.5f;
    RGBColor baseCol = getLevelColor(0);
    uint8_t r = (uint8_t)(baseCol.r * breath);
    uint8_t g = (uint8_t)(baseCol.g * breath);
    uint8_t b = (uint8_t)(baseCol.b * breath);

    for (int i = 0; i < NUM_LEDS; i++) {
        _frameBuffer[i] = RGBColor(0, 0, 0);
    }
    // Base throat glow (LEDs 9 and 10)
    _frameBuffer[9] = RGBColor(r, g, b);
    _frameBuffer[10] = RGBColor(r, g, b);
}

void LEDMeter::renderScanningAnimation() {
    // Cyber radar bounce sweeping up and down both jaws simultaneously
    int pos = (_animStep / 2) % 18;
    int lvl = (pos < 10) ? pos : (18 - pos); // 0 to 9 to 0

    for (int i = 0; i < NUM_LEDS; i++) {
        // Fade previous
        _frameBuffer[i].r = (uint8_t)(_frameBuffer[i].r * 0.65f);
        _frameBuffer[i].g = (uint8_t)(_frameBuffer[i].g * 0.65f);
        _frameBuffer[i].b = (uint8_t)(_frameBuffer[i].b * 0.65f);
    }

    int leftLed = 9 - lvl;
    int rightLed = 10 + lvl;
    RGBColor scanColor = getLevelColor(lvl);
    _frameBuffer[leftLed] = scanColor;
    _frameBuffer[rightLed] = scanColor;
}

void LEDMeter::renderTrackingMeter() {
    // Decay floating peak indicator
    unsigned long now = millis();
    static unsigned long lastPeakDecay = 0;
    if (now > _peakHoldMs && (now - lastPeakDecay > 120)) {
        lastPeakDecay = now;
        if (_peakLevel > _activeLevels) {
            _peakLevel--;
        }
    }

    // Render 10 vertical levels on both jaws (0=base, 9=tip)
    for (int lvl = 0; lvl < 10; lvl++) {
        int leftLed = 9 - lvl;
        int rightLed = 10 + lvl;

        if (lvl < _activeLevels) {
            RGBColor c = getLevelColor(lvl);
            _frameBuffer[leftLed] = c;
            _frameBuffer[rightLed] = c;
        } else if (lvl == _peakLevel - 1 && _peakLevel > 0) {
            // Floating peak marker
            _frameBuffer[leftLed] = RGBColor(255, 255, 255);
            _frameBuffer[rightLed] = RGBColor(255, 255, 255);
        } else {
            // Off
            _frameBuffer[leftLed] = RGBColor(0, 0, 0);
            _frameBuffer[rightLed] = RGBColor(0, 0, 0);
        }
    }

    // High signal sparkle / contact burst at level 10
    if (_activeLevels >= 10 && random(0, 100) < 30) {
        _frameBuffer[0] = RGBColor(255, 255, 255);
        _frameBuffer[19] = RGBColor(255, 255, 255);
    }
}

void LEDMeter::renderOptionsAnimation() {
    // Continuous live test preview: display full 10-level theme gradient
    // along both jaws with a traveling highlight pulse
    int highlightLvl = (_animStep / 3) % 10;

    for (int lvl = 0; lvl < 10; lvl++) {
        int leftLed = 9 - lvl;
        int rightLed = 10 + lvl;
        RGBColor c = getLevelColor(lvl);

        if (lvl == highlightLvl) {
            c.r = min(255, (int)(c.r * 1.3f + 40));
            c.g = min(255, (int)(c.g * 1.3f + 40));
            c.b = min(255, (int)(c.b * 1.3f + 40));
        }

        _frameBuffer[leftLed] = c;
        _frameBuffer[rightLed] = c;
    }
}

void LEDMeter::update() {
    unsigned long now = millis();
    if (now - _lastUpdateMs < 30) return; // ~33 FPS
    _lastUpdateMs = now;
    _animStep++;

    switch (_mode) {
        case METER_MODE_IDLE:
            renderIdleAnimation();
            break;
        case METER_MODE_SCANNING:
            renderScanningAnimation();
            break;
        case METER_MODE_TRACKING:
            renderTrackingMeter();
            break;
        case METER_MODE_OPTIONS:
            renderOptionsAnimation();
            break;
    }

    show();
}

void LEDMeter::show() {
    for (int i = 0; i < NUM_LEDS; i++) {
        _strip.setPixelColor(i, _frameBuffer[i].r, _frameBuffer[i].g, _frameBuffer[i].b);
    }
    _strip.show();
}
