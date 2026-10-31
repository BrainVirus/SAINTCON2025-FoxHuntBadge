#pragma once

#include <Arduino.h>
#include "display_driver.h"
#include "joystick.h"
#include "scanner.h"
#include "led_meter.h"

enum UIScreen {
    SCREEN_PROTO_SELECT = 0,
    SCREEN_TARGET_LIST,
    SCREEN_TRACKING,
    SCREEN_OPTIONS
};

class FoxhuntUI {
public:
    FoxhuntUI();

    void begin();
    void update();
    void handleInput(JoystickEvent event);

    void setScreen(UIScreen screen);
    UIScreen getScreen() const { return _currentScreen; }

private:
    UIScreen _currentScreen;
    int _protoCursor;        // 0 = Wi-Fi, 1 = BLE, 2 = LED Options
    int _listCursor;         // Selected target in list
    int _scrollOffset;       // Top visible row
    int _optCursor;          // 0 = Theme, 1 = Brightness, 2 = Back

    bool _trackingPaused;
    unsigned long _lastRenderMs;
    bool _needsRedraw;

    // Mini Wrench coordinates on the tracking screen
    static const int WRENCH_X[NUM_LEDS];
    static const int WRENCH_Y[NUM_LEDS];

    // Screen draw routines
    void drawProtoSelectScreen();
    void drawTargetListScreen();
    void drawTrackingScreen();
    void drawOptionsScreen();

    // Tracking HUD sub-draws
    void drawTrackingHeader();
    void drawTargetCard();
    void drawGaugeSection();
    void drawRssiGraph();
    void drawMiniWrenchHUD();
    void drawStatsAndFooter();
};

extern FoxhuntUI UI;
