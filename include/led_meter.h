#pragma once

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <Preferences.h>
#include "config.h"

struct RGBColor {
    uint8_t r, g, b;
    RGBColor() : r(0), g(0), b(0) {}
    RGBColor(uint8_t red, uint8_t green, uint8_t blue) : r(red), g(green), b(blue) {}
};

enum MeterMode {
    METER_MODE_IDLE = 0,
    METER_MODE_SCANNING,
    METER_MODE_TRACKING,
    METER_MODE_OPTIONS
};

enum ColorTheme {
    THEME_CYBER_THERMAL = 0,
    THEME_SAINTCON_GOLD,
    THEME_TOXIC_MATRIX,
    THEME_CYBERPUNK_NEON,
    THEME_LAVA_MAGMA,
    THEME_ICE_FROST,
    THEME_BLOODHOUND_RED,
    THEME_RAINBOW,
    THEME_COUNT
};

class LEDMeter {
public:
    LEDMeter();

    void begin();
    void update();
    void show();

    void setMode(MeterMode mode);
    MeterMode getMode() const { return _mode; }

    // RSSI Tracking Input
    void setSignal(int rssi, float qualityPercent);

    // Brightness Control
    void setBrightness(uint8_t brightness);
    uint8_t getBrightness() const { return _brightness; }
    void adjustBrightness(int delta);

    // Color Theme Control
    void setTheme(ColorTheme theme);
    ColorTheme getTheme() const { return _currentTheme; }
    const char* getThemeName(ColorTheme theme) const;
    const char* getCurrentThemeName() const { return getThemeName(_currentTheme); }
    void nextTheme();
    void prevTheme();

    // Readout for on-screen mini wrench mirror & swatches
    RGBColor getLed(uint8_t index) const;
    int getActiveLevels() const { return _activeLevels; }
    int getPeakLevel() const { return _peakLevel; }

    RGBColor getLevelColor(int level) const;
    static RGBColor getThemeColor(ColorTheme theme, int level);

    // NVS Persistence
    void loadSettings();
    void saveSettings();

private:
    Adafruit_NeoPixel _strip;
    MeterMode _mode;
    uint8_t _brightness;
    ColorTheme _currentTheme;

    int _currentRssi;
    float _signalPercent;
    int _activeLevels;       // 0 to 10
    int _peakLevel;          // Floating peak indicator 0 to 10
    unsigned long _peakHoldMs;

    unsigned long _lastUpdateMs;
    uint32_t _animStep;

    RGBColor _frameBuffer[NUM_LEDS];

    void renderIdleAnimation();
    void renderScanningAnimation();
    void renderTrackingMeter();
    void renderOptionsAnimation();
};

extern LEDMeter Meter;
