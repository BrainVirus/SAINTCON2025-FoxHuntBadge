#include "foxhunt_ui.h"

FoxhuntUI UI;

// Mini wrench visualizer coordinates shifted to fit HUD area (Y: 204 to 244)
const int FoxhuntUI::WRENCH_X[NUM_LEDS] = {
    65, 56, 50, 48, 50, 56, 65, 78, 93, 109,
    131, 147, 162, 175, 184, 190, 192, 190, 184, 175
};

const int FoxhuntUI::WRENCH_Y[NUM_LEDS] = {
    210, 214, 220, 226, 232, 237, 241, 243, 244, 244,
    244, 244, 243, 241, 237, 232, 226, 220, 214, 210
};

FoxhuntUI::FoxhuntUI()
    : _currentScreen(SCREEN_PROTO_SELECT),
      _protoCursor(0),
      _listCursor(0),
      _scrollOffset(0),
      _optCursor(0),
      _trackingPaused(false),
      _lastRenderMs(0),
      _needsRedraw(true)
{
}

void FoxhuntUI::begin() {
    _currentScreen = SCREEN_PROTO_SELECT;
    _needsRedraw = true;
}

void FoxhuntUI::setScreen(UIScreen screen) {
    _currentScreen = screen;
    _needsRedraw = true;
}

void FoxhuntUI::handleInput(JoystickEvent event) {
    if (event == JOY_NONE) return;

    if (_currentScreen == SCREEN_PROTO_SELECT) {
        switch (event) {
            case JOY_UP:
                _protoCursor = (_protoCursor + 2) % 3;
                _needsRedraw = true;
                break;
            case JOY_DOWN:
                _protoCursor = (_protoCursor + 1) % 3;
                _needsRedraw = true;
                break;

            case JOY_CENTER_CLICK:
                if (_protoCursor == 0) {
                    RadioScanner.setProtocol(PROTO_WIFI);
                    _currentScreen = SCREEN_TARGET_LIST;
                    _listCursor = 0;
                    _scrollOffset = 0;
                    _needsRedraw = true;
                } else if (_protoCursor == 1) {
                    RadioScanner.setProtocol(PROTO_BLE);
                    _currentScreen = SCREEN_TARGET_LIST;
                    _listCursor = 0;
                    _scrollOffset = 0;
                    _needsRedraw = true;
                } else if (_protoCursor == 2) {
                    _currentScreen = SCREEN_OPTIONS;
                    _optCursor = 0;
                    Meter.setMode(METER_MODE_OPTIONS);
                    _needsRedraw = true;
                }
                break;

            default:
                break;
        }
    } else if (_currentScreen == SCREEN_OPTIONS) {
        switch (event) {
            case JOY_UP:
                _optCursor = (_optCursor + 2) % 3;
                _needsRedraw = true;
                break;

            case JOY_DOWN:
                _optCursor = (_optCursor + 1) % 3;
                _needsRedraw = true;
                break;

            case JOY_LEFT:
                if (_optCursor == 0) {
                    Meter.prevTheme();
                    _needsRedraw = true;
                } else if (_optCursor == 1) {
                    Meter.adjustBrightness(-15);
                    _needsRedraw = true;
                } else if (_optCursor == 2) {
                    Meter.saveSettings();
                    _currentScreen = SCREEN_PROTO_SELECT;
                    Meter.setMode(METER_MODE_IDLE);
                    _needsRedraw = true;
                }
                break;

            case JOY_RIGHT:
                if (_optCursor == 0) {
                    Meter.nextTheme();
                    _needsRedraw = true;
                } else if (_optCursor == 1) {
                    Meter.adjustBrightness(+15);
                    _needsRedraw = true;
                }
                break;

            case JOY_CENTER_CLICK:
                if (_optCursor == 0) {
                    Meter.nextTheme();
                    _needsRedraw = true;
                } else if (_optCursor == 1) {
                    Meter.adjustBrightness(+40);
                    _needsRedraw = true;
                } else if (_optCursor == 2) {
                    Meter.saveSettings();
                    _currentScreen = SCREEN_PROTO_SELECT;
                    Meter.setMode(METER_MODE_IDLE);
                    _needsRedraw = true;
                }
                break;

            case JOY_CENTER_HOLD:
                Meter.saveSettings();
                _currentScreen = SCREEN_PROTO_SELECT;
                Meter.setMode(METER_MODE_IDLE);
                _needsRedraw = true;
                break;

            default:
                break;
        }
    } else if (_currentScreen == SCREEN_TARGET_LIST) {
        int count = RadioScanner.getTargetCount();

        switch (event) {
            case JOY_UP:
                if (count > 0) {
                    _listCursor--;
                    if (_listCursor < 0) _listCursor = count - 1;
                    if (_listCursor < _scrollOffset) _scrollOffset = _listCursor;
                    else if (_listCursor >= _scrollOffset + 5) _scrollOffset = _listCursor - 4;
                    _needsRedraw = true;
                }
                break;

            case JOY_DOWN:
                if (count > 0) {
                    _listCursor++;
                    if (_listCursor >= count) _listCursor = 0;
                    if (_listCursor >= _scrollOffset + 5) _scrollOffset = _listCursor - 4;
                    else if (_listCursor < _scrollOffset) _scrollOffset = _listCursor;
                    _needsRedraw = true;
                }
                break;

            case JOY_LEFT:
                // Rescan
                RadioScanner.startScan();
                _listCursor = 0;
                _scrollOffset = 0;
                _needsRedraw = true;
                break;

            case JOY_CENTER_HOLD:
                // Back to protocol select
                RadioScanner.stopScan();
                _currentScreen = SCREEN_PROTO_SELECT;
                _needsRedraw = true;
                break;

            case JOY_CENTER_CLICK:
                if (count > 0 && _listCursor < count) {
                    RadioScanner.selectTarget(_listCursor);
                    _currentScreen = SCREEN_TRACKING;
                    _trackingPaused = false;
                    _needsRedraw = true;
                }
                break;

            default:
                break;
        }
    } else if (_currentScreen == SCREEN_TRACKING) {
        switch (event) {
            case JOY_LEFT:
            case JOY_CENTER_HOLD:
                // Exit tracking mode back to target list
                RadioScanner.stopTracking();
                _currentScreen = SCREEN_TARGET_LIST;
                RadioScanner.startScan();
                _needsRedraw = true;
                break;

            case JOY_CENTER_CLICK:
                // Pause / resume tracking
                _trackingPaused = !_trackingPaused;
                _needsRedraw = true;
                break;

            case JOY_RIGHT:
                // Reset Peak and History
                RadioScanner.resetMetrics();
                _needsRedraw = true;
                break;

            case JOY_UP:
                // Increase sensitivity floor
                RadioScanner.adjustMinRssi(+3);
                _needsRedraw = true;
                break;

            case JOY_DOWN:
                // Decrease sensitivity floor
                RadioScanner.adjustMinRssi(-3);
                _needsRedraw = true;
                break;

            default:
                break;
        }
    }
}

void FoxhuntUI::drawProtoSelectScreen() {
    Display.fillRect(0, 0, LCD_WIDTH, LCD_HEIGHT, COLOR565_DARK_NAVY);

    // Header (Y: 0 to 32)
    Display.fillRect(0, 0, LCD_WIDTH, 32, COLOR565_BLACK);
    Display.drawStringCenter("SAINTCON 2025", 5, COLOR565_GOLD, COLOR565_BLACK, 2);
    Display.drawStringCenter("WIRELESS FOXHUNTER", 21, COLOR565_CYBER_CYAN, COLOR565_BLACK, 1);
    Display.drawFastHLine(0, 32, LCD_WIDTH, COLOR565_CYBER_CYAN);

    // Card 0: Wi-Fi 2.4GHz (Y: 38, H: 66)
    bool selWifi = (_protoCursor == 0);
    Display.fillRoundRect(8, 38, LCD_WIDTH - 16, 66, 5, selWifi ? COLOR565_ACCENT_BLUE : COLOR565_CARD_BG);
    Display.drawRoundRect(8, 38, LCD_WIDTH - 16, 66, 5, selWifi ? COLOR565_CYBER_CYAN : COLOR565_BORDER);
    Display.fillRoundRect(16, 44, 46, 14, 2, COLOR565_CYBER_CYAN);
    Display.drawString("WI-FI", 22, 47, COLOR565_BLACK, COLOR565_CYBER_CYAN, 1);
    Display.drawString("2.4GHz AP Scanner", 68, 46, selWifi ? COLOR565_WHITE : COLOR565_GOLD, 0, 1);
    Display.drawString("Track Access Points, Routers,", 16, 64, COLOR565_TEXT_MUTED, 0, 1);
    Display.drawString("Hotspots, and Hidden SSIDs", 16, 78, COLOR565_TEXT_MUTED, 0, 1);
    if (selWifi) Display.drawString(">", LCD_WIDTH - 22, 60, COLOR565_CYBER_CYAN, 0, 2);

    // Card 1: Bluetooth LE (Y: 108, H: 66)
    bool selBle = (_protoCursor == 1);
    Display.fillRoundRect(8, 108, LCD_WIDTH - 16, 66, 5, selBle ? COLOR565_ACCENT_BLUE : COLOR565_CARD_BG);
    Display.drawRoundRect(8, 108, LCD_WIDTH - 16, 66, 5, selBle ? COLOR565_GOLD : COLOR565_BORDER);
    Display.fillRoundRect(16, 114, 46, 14, 2, COLOR565_GOLD);
    Display.drawString("BLE", 26, 117, COLOR565_BLACK, COLOR565_GOLD, 1);
    Display.drawString("Bluetooth LE Scanner", 68, 116, selBle ? COLOR565_WHITE : COLOR565_GOLD, 0, 1);
    Display.drawString("Track BLE Beacons, Tags,", 16, 134, COLOR565_TEXT_MUTED, 0, 1);
    Display.drawString("AirTags, Trackers, & Gadgets", 16, 148, COLOR565_TEXT_MUTED, 0, 1);
    if (selBle) Display.drawString(">", LCD_WIDTH - 22, 130, COLOR565_GOLD, 0, 2);

    // Card 2: LED Options & Settings (Y: 178, H: 66)
    bool selOpts = (_protoCursor == 2);
    Display.fillRoundRect(8, 178, LCD_WIDTH - 16, 66, 5, selOpts ? COLOR565_ACCENT_BLUE : COLOR565_CARD_BG);
    Display.drawRoundRect(8, 178, LCD_WIDTH - 16, 66, 5, selOpts ? COLOR565_CYBER_CYAN : COLOR565_BORDER);
    Display.fillRoundRect(16, 184, 46, 14, 2, COLOR565_MAGENTA);
    Display.drawString("OPTS", 22, 187, COLOR565_WHITE, COLOR565_MAGENTA, 1);
    Display.drawString("LED Palette & Brightness", 68, 186, selOpts ? COLOR565_WHITE : COLOR565_GOLD, 0, 1);
    char optMeta[48];
    snprintf(optMeta, sizeof(optMeta), "Theme: %s", Meter.getCurrentThemeName());
    Display.drawString(optMeta, 16, 204, COLOR565_CYBER_CYAN, 0, 1);
    char brightMeta[48];
    snprintf(brightMeta, sizeof(brightMeta), "Brightness: %d%% (%d/255)", (Meter.getBrightness() * 100) / 255, Meter.getBrightness());
    Display.drawString(brightMeta, 16, 218, COLOR565_TEXT_MUTED, 0, 1);
    if (selOpts) Display.drawString(">", LCD_WIDTH - 22, 200, COLOR565_CYBER_CYAN, 0, 2);

    // Tip Banner (Y: 248, H: 44)
    Display.fillRoundRect(8, 248, LCD_WIDTH - 16, 44, 4, COLOR565_CARD_BG);
    Display.drawRoundRect(8, 248, LCD_WIDTH - 16, 44, 4, COLOR565_BORDER);
    Display.drawStringCenter("LEDs surge up wrench jaws as", 254, COLOR565_GOLD, COLOR565_CARD_BG, 1);
    Display.drawStringCenter("you close in on the radio target!", 268, COLOR565_WHITE, COLOR565_CARD_BG, 1);

    // Footer (Y: 298 to 320)
    Display.fillRect(0, 298, LCD_WIDTH, 22, COLOR565_BLACK);
    Display.drawFastHLine(0, 298, LCD_WIDTH, COLOR565_BORDER);
    Display.drawStringCenter("^v:Select    O:Confirm / Open", 304, COLOR565_CYBER_CYAN, COLOR565_BLACK, 1);
}

void FoxhuntUI::drawTargetListScreen() {
    Display.fillRect(0, 0, LCD_WIDTH, LCD_HEIGHT, COLOR565_DARK_NAVY);

    // Header
    bool isWifi = (RadioScanner.getProtocol() == PROTO_WIFI);
    Display.fillRect(0, 0, LCD_WIDTH, 26, COLOR565_BLACK);
    Display.drawString(isWifi ? "WI-FI AP SCANNER" : "BLUETOOTH LE SCANNER", 10, 8, isWifi ? COLOR565_CYBER_CYAN : COLOR565_GOLD, COLOR565_BLACK, 1);

    // Scan Status Pill
    if (RadioScanner.isScanning()) {
        Display.fillRoundRect(LCD_WIDTH - 64, 6, 56, 14, 2, COLOR565_GOLD);
        Display.drawString("SCANNING", LCD_WIDTH - 60, 9, COLOR565_BLACK, COLOR565_GOLD, 1);
    } else {
        Display.fillRoundRect(LCD_WIDTH - 54, 6, 46, 14, 2, COLOR565_GREEN);
        Display.drawString("READY", LCD_WIDTH - 46, 9, COLOR565_BLACK, COLOR565_GREEN, 1);
    }
    Display.drawFastHLine(0, 26, LCD_WIDTH, COLOR565_CYBER_CYAN);

    // Status Subheader
    char subBuf[48];
    int count = RadioScanner.getTargetCount();
    snprintf(subBuf, sizeof(subBuf), "Targets: %d (Sorted by Signal)", count);
    Display.drawString(subBuf, 10, 31, COLOR565_TEXT_MUTED, COLOR565_DARK_NAVY, 1);

    // Target List (5 rows max per view, 48px height each)
    int listY = 46;
    int itemH = 48;

    for (int row = 0; row < 5; row++) {
        int idx = _scrollOffset + row;
        if (idx >= count) break;

        TargetDevice dev;
        if (!RadioScanner.getTarget(idx, dev)) break;

        int y = listY + row * itemH;
        bool isSel = (idx == _listCursor);

        // Card box
        Display.fillRoundRect(8, y, LCD_WIDTH - 22, itemH - 2, 4, isSel ? COLOR565_ACCENT_BLUE : COLOR565_CARD_BG);
        Display.drawRoundRect(8, y, LCD_WIDTH - 22, itemH - 2, 4, isSel ? COLOR565_CYBER_CYAN : COLOR565_BORDER);

        // Name
        char nameBuf[26];
        snprintf(nameBuf, sizeof(nameBuf), "%s", dev.name.c_str());
        Display.drawString(nameBuf, 14, y + 5, isSel ? COLOR565_WHITE : COLOR565_GOLD, 0, 1);

        // Address & Channel / Extra
        char metaBuf[40];
        if (dev.proto == PROTO_WIFI) {
            snprintf(metaBuf, sizeof(metaBuf), "%s  Ch:%d", dev.address.c_str(), dev.channel);
        } else {
            snprintf(metaBuf, sizeof(metaBuf), "%s  %s", dev.address.c_str(), dev.extra.c_str());
        }
        Display.drawString(metaBuf, 14, y + 20, COLOR565_TEXT_MUTED, 0, 1);

        // RSSI Value & Mini Bar
        char rssiBuf[16];
        snprintf(rssiBuf, sizeof(rssiBuf), "%d dBm", dev.rssi);
        uint16_t rssiCol = (dev.rssi > -65) ? COLOR565_GREEN : ((dev.rssi > -80) ? COLOR565_GOLD : COLOR565_CYBER_CYAN);
        Display.drawString(rssiBuf, LCD_WIDTH - 76, y + 5, rssiCol, 0, 1);

        // Mini signal bar (40px)
        int barW = map(constrain(dev.rssi, -95, -30), -95, -30, 2, 40);
        Display.fillRect(LCD_WIDTH - 76, y + 20, 40, 4, 0x110B);
        Display.fillRect(LCD_WIDTH - 76, y + 20, barW, 4, rssiCol);

        // Selection pointer
        if (isSel) {
            Display.drawString(">", LCD_WIDTH - 28, y + 12, COLOR565_CYBER_CYAN, 0, 2);
        }
    }

    // Scrollbar
    if (count > 5) {
        int trackX = LCD_WIDTH - 10;
        int trackY = listY;
        int trackH = 5 * itemH - 2;
        Display.fillRect(trackX, trackY, 4, trackH, 0x110B);
        int thumbH = max(14, trackH * 5 / count);
        int thumbY = trackY + (_scrollOffset * (trackH - thumbH) / (count - 5));
        Display.fillRoundRect(trackX, thumbY, 4, thumbH, 2, COLOR565_CYBER_CYAN);
    }

    // Footer
    Display.fillRect(0, 298, LCD_WIDTH, 22, COLOR565_BLACK);
    Display.drawFastHLine(0, 298, LCD_WIDTH, COLOR565_BORDER);
    Display.drawString("^v:Select  O:Track Fox  <:Rescan", 8, 304, COLOR565_CYBER_CYAN, COLOR565_BLACK, 1);
}

void FoxhuntUI::drawTrackingHeader() {
    Display.fillRect(0, 0, LCD_WIDTH, 24, COLOR565_BLACK);
    bool isWifi = (RadioScanner.getProtocol() == PROTO_WIFI);

    Display.fillRoundRect(6, 4, 62, 16, 2, isWifi ? COLOR565_CYBER_CYAN : COLOR565_GOLD);
    Display.drawString(isWifi ? "WI-FI FOX" : "BLE FOX", 10, 8, COLOR565_BLACK, isWifi ? COLOR565_CYBER_CYAN : COLOR565_GOLD, 1);

    if (_trackingPaused) {
        Display.fillRoundRect(LCD_WIDTH - 64, 4, 56, 16, 2, COLOR565_RED);
        Display.drawString("PAUSED", LCD_WIDTH - 58, 8, COLOR565_WHITE, COLOR565_RED, 1);
    } else {
        Display.fillRoundRect(LCD_WIDTH - 72, 4, 64, 16, 2, COLOR565_GREEN);
        Display.drawString("TRACKING", LCD_WIDTH - 68, 8, COLOR565_BLACK, COLOR565_GREEN, 1);
    }
    Display.drawFastHLine(0, 24, LCD_WIDTH, COLOR565_CYBER_CYAN);
}

void FoxhuntUI::drawTargetCard() {
    const TargetDevice &target = RadioScanner.getTrackedTarget();

    Display.fillRoundRect(8, 28, LCD_WIDTH - 16, 32, 4, COLOR565_CARD_BG);
    Display.drawRoundRect(8, 28, LCD_WIDTH - 16, 32, 4, COLOR565_BORDER);

    // Target Name
    char nameBuf[28];
    snprintf(nameBuf, sizeof(nameBuf), "%s", target.name.c_str());
    Display.drawString(nameBuf, 14, 33, COLOR565_WHITE, COLOR565_CARD_BG, 1);

    // Target Address & Channel
    char addrBuf[36];
    if (target.proto == PROTO_WIFI) {
        snprintf(addrBuf, sizeof(addrBuf), "MAC:%s  CH:%d", target.address.c_str(), target.channel);
    } else {
        snprintf(addrBuf, sizeof(addrBuf), "MAC:%s  %s", target.address.c_str(), target.extra.c_str());
    }
    Display.drawString(addrBuf, 14, 46, COLOR565_CYBER_CYAN, COLOR565_CARD_BG, 1);
}

void FoxhuntUI::drawGaugeSection() {
    int rssi = RadioScanner.getCurrentRssi();
    float pct = RadioScanner.getSignalPercent();
    int activeLevels = Meter.getActiveLevels();

    // Large RSSI Readout Box
    Display.fillRoundRect(8, 64, LCD_WIDTH - 16, 56, 4, COLOR565_CARD_BG);
    Display.drawRoundRect(8, 64, LCD_WIDTH - 16, 56, 4, COLOR565_BORDER);

    // Big RSSI Text
    char rssiBuf[16];
    snprintf(rssiBuf, sizeof(rssiBuf), "%d", rssi);
    uint16_t col = (rssi > -45) ? COLOR565_WHITE : ((rssi > -60) ? COLOR565_HOT_ORANGE : ((rssi > -75) ? COLOR565_GOLD : COLOR565_CYBER_CYAN));
    Display.drawString(rssiBuf, 16, 72, col, COLOR565_CARD_BG, 3);
    Display.drawString("dBm", 80, 84, COLOR565_TEXT_MUTED, COLOR565_CARD_BG, 1);

    // Proximity Status Banner
    const char *statusText = "SEARCHING...";
    uint16_t statusBg = COLOR565_CARD_BG;
    uint16_t statusFg = COLOR565_TEXT_MUTED;

    if (rssi > -38) {
        statusText = ">> HOT / CONTACT <<";
        statusBg = COLOR565_RED;
        statusFg = COLOR565_WHITE;
    } else if (rssi > -50) {
        statusText = "^ VERY CLOSE ^";
        statusBg = COLOR565_HOT_ORANGE;
        statusFg = COLOR565_BLACK;
    } else if (rssi > -65) {
        statusText = "^ CLOSING IN ^";
        statusBg = COLOR565_GOLD;
        statusFg = COLOR565_BLACK;
    } else if (rssi > -80) {
        statusText = "> IN RANGE <";
        statusBg = COLOR565_GREEN;
        statusFg = COLOR565_BLACK;
    } else {
        statusText = "v WEAK SIGNAL v";
        statusBg = COLOR565_CARD_BG;
        statusFg = COLOR565_CYBER_CYAN;
    }

    Display.fillRoundRect(120, 72, 102, 16, 2, statusBg);
    Display.drawStringCenter(statusText, 76, statusFg, statusBg, 1);

    // Quality Percentage Pill
    char pctBuf[20];
    snprintf(pctBuf, sizeof(pctBuf), "Signal: %d%%", (int)(pct + 0.5f));
    Display.drawString(pctBuf, 124, 92, COLOR565_TEXT_MUTED, COLOR565_CARD_BG, 1);

    // 10-Segment LED Meter Bar (mirrors the 10 wrench jaw levels)
    int barX = 14;
    int barY = 106;
    int segW = 19;
    int segGap = 2;

    for (int lvl = 0; lvl < 10; lvl++) {
        int x = barX + lvl * (segW + segGap);
        if (lvl < activeLevels) {
            RGBColor c = Meter.getLevelColor(lvl);
            uint16_t c565 = DisplayDriver::rgbTo565(c.r, c.g, c.b);
            Display.fillRect(x, barY, segW, 8, c565);
        } else {
            Display.fillRect(x, barY, segW, 8, 0x110B);
        }
    }
}

void FoxhuntUI::drawRssiGraph() {
    int graphX = 8;
    int graphY = 124;
    int graphW = LCD_WIDTH - 16;
    int graphH = 58;

    Display.fillRoundRect(graphX, graphY, graphW, graphH, 4, COLOR565_BLACK);
    Display.drawRoundRect(graphX, graphY, graphW, graphH, 4, COLOR565_BORDER);

    // Reference Grid Lines
    // -30 dBm (Top), -60 dBm (Mid), -90 dBm (Bottom)
    int y30 = graphY + 12;
    int y60 = graphY + 32;
    int y90 = graphY + 50;

    Display.drawFastHLine(graphX + 2, y30, graphW - 4, 0x18E3);
    Display.drawFastHLine(graphX + 2, y60, graphW - 4, 0x18E3);
    Display.drawFastHLine(graphX + 2, y90, graphW - 4, 0x18E3);

    Display.drawString("-30", graphX + 4, y30 - 8, COLOR565_TEXT_MUTED, 0, 1);
    Display.drawString("-60", graphX + 4, y60 - 8, COLOR565_TEXT_MUTED, 0, 1);
    Display.drawString("-90", graphX + 4, y90 - 8, COLOR565_TEXT_MUTED, 0, 1);

    // Plot 60 History Samples
    float stepX = (float)(graphW - 36) / (float)(Scanner::HISTORY_LEN - 1);
    int prevX = 0, prevY = 0;

    for (int i = 0; i < Scanner::HISTORY_LEN; i++) {
        int rssi = RadioScanner.getHistory(i);
        int px = (graphX + 30) + (int)(i * stepX);
        // Map RSSI (-95 to -25) to Y (graphY + 52 down to graphY + 8)
        int py = map(constrain(rssi, -95, -25), -95, -25, graphY + 52, graphY + 8);

        if (i > 0) {
            uint16_t traceCol = (rssi > -50) ? COLOR565_HOT_ORANGE : ((rssi > -70) ? COLOR565_GOLD : COLOR565_CYBER_CYAN);
            Display.drawLine(prevX, prevY, px, py, traceCol);
        }
        prevX = px;
        prevY = py;
    }
}

void FoxhuntUI::drawMiniWrenchHUD() {
    int hudY = 186;
    Display.fillRect(0, hudY, LCD_WIDTH, 64, COLOR565_DARK_NAVY);

    // Subtle outline connecting the dots
    for (int i = 0; i < 9; i++) {
        Display.drawLine(WRENCH_X[i], WRENCH_Y[i], WRENCH_X[i + 1], WRENCH_Y[i + 1], 0x1A25);
    }
    Display.drawLine(WRENCH_X[9], WRENCH_Y[9], WRENCH_X[10], WRENCH_Y[10], 0x1A25);
    for (int i = 10; i < 19; i++) {
        Display.drawLine(WRENCH_X[i], WRENCH_Y[i], WRENCH_X[i + 1], WRENCH_Y[i + 1], 0x1A25);
    }

    // Draw the 20 glowing LED nodes matching the physical wrench jaws
    for (int i = 0; i < NUM_LEDS; i++) {
        RGBColor c = Meter.getLed(i);
        uint16_t col565 = DisplayDriver::rgbTo565(c.r, c.g, c.b);
        Display.fillCircle(WRENCH_X[i], WRENCH_Y[i], 3, col565);
        Display.drawCircle(WRENCH_X[i], WRENCH_Y[i], 4, COLOR565_BLACK);
    }

    Display.drawStringCenter("PHYSICAL WRENCH JAW SIGNAL METER", 192, COLOR565_TEXT_MUTED, COLOR565_DARK_NAVY, 1);
}

void FoxhuntUI::drawStatsAndFooter() {
    int statsY = 252;
    Display.fillRect(0, statsY, LCD_WIDTH, 44, COLOR565_DARK_NAVY);

    // Card 1: Peak & Min RSSI
    Display.fillRoundRect(8, statsY, 108, 40, 4, COLOR565_CARD_BG);
    Display.drawRoundRect(8, statsY, 108, 40, 4, COLOR565_BORDER);
    char peakBuf[24];
    snprintf(peakBuf, sizeof(peakBuf), "Peak: %d dBm", RadioScanner.getPeakRssi());
    Display.drawString(peakBuf, 14, statsY + 6, COLOR565_GOLD, COLOR565_CARD_BG, 1);
    char minBuf[24];
    snprintf(minBuf, sizeof(minBuf), "Min:  %d dBm", RadioScanner.getMinRssi());
    Display.drawString(minBuf, 14, statsY + 22, COLOR565_TEXT_MUTED, COLOR565_CARD_BG, 1);

    // Card 2: Packet Rate & Sensitivity Range
    Display.fillRoundRect(124, statsY, 108, 40, 4, COLOR565_CARD_BG);
    Display.drawRoundRect(124, statsY, 108, 40, 4, COLOR565_BORDER);
    char rateBuf[24];
    snprintf(rateBuf, sizeof(rateBuf), "Rate: %.1f pkts/s", RadioScanner.getPacketsPerSec());
    Display.drawString(rateBuf, 130, statsY + 6, COLOR565_CYBER_CYAN, COLOR565_CARD_BG, 1);
    char sensBuf[24];
    snprintf(sensBuf, sizeof(sensBuf), "Sens: %ddB", RadioScanner.getMinRssiThreshold());
    Display.drawString(sensBuf, 130, statsY + 22, COLOR565_TEXT_MUTED, COLOR565_CARD_BG, 1);

    // Footer Action Bar
    Display.fillRect(0, 298, LCD_WIDTH, 22, COLOR565_BLACK);
    Display.drawFastHLine(0, 298, LCD_WIDTH, COLOR565_BORDER);
    Display.drawString("<:Back  O:Pause  >:Reset  ^v:Sens", 6, 304, COLOR565_CYBER_CYAN, COLOR565_BLACK, 1);
}

void FoxhuntUI::drawOptionsScreen() {
    Display.fillRect(0, 0, LCD_WIDTH, LCD_HEIGHT, COLOR565_DARK_NAVY);

    // Header (Y: 0 to 30)
    Display.fillRect(0, 0, LCD_WIDTH, 30, COLOR565_BLACK);
    Display.drawStringCenter("FOXHUNTER OPTIONS", 5, COLOR565_GOLD, COLOR565_BLACK, 2);
    Display.drawStringCenter("LED COLOR PALETTE & BRIGHTNESS", 20, COLOR565_CYBER_CYAN, COLOR565_BLACK, 1);
    Display.drawFastHLine(0, 30, LCD_WIDTH, COLOR565_CYBER_CYAN);

    // Card 0: LED Color Theme (Y: 36, H: 82)
    bool selTheme = (_optCursor == 0);
    Display.fillRoundRect(8, 36, LCD_WIDTH - 16, 82, 5, selTheme ? COLOR565_ACCENT_BLUE : COLOR565_CARD_BG);
    Display.drawRoundRect(8, 36, LCD_WIDTH - 16, 82, 5, selTheme ? COLOR565_CYBER_CYAN : COLOR565_BORDER);
    Display.drawString("LED COLOR PALETTE", 16, 42, selTheme ? COLOR565_CYBER_CYAN : COLOR565_TEXT_MUTED, 0, 1);

    char themeBuf[32];
    snprintf(themeBuf, sizeof(themeBuf), "<  %s  >", Meter.getCurrentThemeName());
    Display.drawStringCenter(themeBuf, 56, selTheme ? COLOR565_GOLD : COLOR565_WHITE, 0, 1);

    // 10-level color gradient swatch bar
    int swatchStartX = 18;
    int swatchY = 74;
    int swatchW = 20;
    int swatchH = 16;
    for (int i = 0; i < 10; i++) {
        RGBColor c = Meter.getThemeColor(Meter.getTheme(), i);
        uint16_t c565 = ((c.r & 0xF8) << 8) | ((c.g & 0xFC) << 3) | (c.b >> 3);
        Display.fillRect(swatchStartX + i * swatchW, swatchY, swatchW - 1, swatchH, c565);
    }
    Display.drawRect(swatchStartX - 1, swatchY - 1, 10 * swatchW + 1, swatchH + 2, COLOR565_BORDER);
    Display.drawString("Base", swatchStartX, swatchY + 18, COLOR565_TEXT_MUTED, 0, 1);
    Display.drawString("Tip", swatchStartX + 10 * swatchW - 18, swatchY + 18, COLOR565_TEXT_MUTED, 0, 1);

    // Card 1: LED Brightness (Y: 124, H: 74)
    bool selBright = (_optCursor == 1);
    Display.fillRoundRect(8, 124, LCD_WIDTH - 16, 74, 5, selBright ? COLOR565_ACCENT_BLUE : COLOR565_CARD_BG);
    Display.drawRoundRect(8, 124, LCD_WIDTH - 16, 74, 5, selBright ? COLOR565_CYBER_CYAN : COLOR565_BORDER);
    Display.drawString("LED BRIGHTNESS", 16, 130, selBright ? COLOR565_CYBER_CYAN : COLOR565_TEXT_MUTED, 0, 1);

    uint8_t bright = Meter.getBrightness();
    int brightPercent = (bright * 100) / 255;
    char brightBuf[32];
    snprintf(brightBuf, sizeof(brightBuf), "%d%%  (%d / 255)", brightPercent, bright);
    Display.drawString(brightBuf, LCD_WIDTH - 120, 130, selBright ? COLOR565_GOLD : COLOR565_WHITE, 0, 1);

    // Visual slider
    int barX = 18;
    int barY = 152;
    int barW = 204;
    int barH = 16;
    Display.fillRoundRect(barX, barY, barW, barH, 3, 0x110B);
    Display.drawRoundRect(barX, barY, barW, barH, 3, COLOR565_BORDER);
    int fillW = (barW * bright) / 255;
    if (fillW > 0) {
        Display.fillRoundRect(barX + 1, barY + 1, fillW - 2, barH - 2, 2, selBright ? COLOR565_GOLD : COLOR565_CYBER_CYAN);
    }
    Display.drawString("<:Dim (-15)        >:Bright (+15)", 18, 174, COLOR565_TEXT_MUTED, 0, 1);

    // Card 2: Live Preview Info Card (Y: 204, H: 52)
    Display.fillRoundRect(8, 204, LCD_WIDTH - 16, 52, 5, COLOR565_CARD_BG);
    Display.drawRoundRect(8, 204, LCD_WIDTH - 16, 52, 5, COLOR565_BORDER);
    Display.drawString("LIVE WRENCH PREVIEW", 16, 210, COLOR565_GOLD, COLOR565_CARD_BG, 1);
    Display.drawString("Physical LEDs show full 10-level", 16, 224, COLOR565_WHITE, COLOR565_CARD_BG, 1);
    Display.drawString("palette & brightness in real time!", 16, 238, COLOR565_CYBER_CYAN, COLOR565_CARD_BG, 1);

    // Card 3: Back Button (Y: 262, H: 30)
    bool selBack = (_optCursor == 2);
    Display.fillRoundRect(8, 262, LCD_WIDTH - 16, 30, 4, selBack ? COLOR565_ACCENT_BLUE : COLOR565_CARD_BG);
    Display.drawRoundRect(8, 262, LCD_WIDTH - 16, 30, 4, selBack ? COLOR565_CYBER_CYAN : COLOR565_BORDER);
    Display.drawStringCenter(selBack ? "> [ SAVE & BACK TO MAIN ] <" : "[ SAVE & BACK TO MAIN ]", 271, selBack ? COLOR565_CYBER_CYAN : COLOR565_WHITE, 0, 1);

    // Footer (Y: 298 to 320)
    Display.fillRect(0, 298, LCD_WIDTH, 22, COLOR565_BLACK);
    Display.drawFastHLine(0, 298, LCD_WIDTH, COLOR565_BORDER);
    Display.drawStringCenter("^v:Move    <>:Adjust    O:Confirm", 304, COLOR565_CYBER_CYAN, COLOR565_BLACK, 1);
}

void FoxhuntUI::drawTrackingScreen() {
    drawTrackingHeader();
    drawTargetCard();
    drawGaugeSection();
    drawRssiGraph();
    drawMiniWrenchHUD();
    drawStatsAndFooter();
}

void FoxhuntUI::update() {
    unsigned long now = millis();
    // Render at ~25 FPS
    if (now - _lastRenderMs >= 40 || _needsRedraw) {
        _lastRenderMs = now;
        _needsRedraw = false;

        switch (_currentScreen) {
            case SCREEN_PROTO_SELECT:
                drawProtoSelectScreen();
                break;
            case SCREEN_TARGET_LIST:
                drawTargetListScreen();
                break;
            case SCREEN_TRACKING:
                drawTrackingScreen();
                break;
            case SCREEN_OPTIONS:
                drawOptionsScreen();
                break;
        }

        Display.flush();
    }
}
