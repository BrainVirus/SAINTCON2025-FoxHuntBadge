#include <Arduino.h>
#include "config.h"
#include "display_driver.h"
#include "joystick.h"
#include "led_meter.h"
#include "scanner.h"
#include "foxhunt_ui.h"

void setup() {
    Serial.begin(SERIAL_BAUD_RATE);
    Serial.setTxTimeoutMs(0);

    Serial.println("\n========================================================");
    Serial.println("  SaintCon 2025 Wrench Badge - Wireless Foxhunter Suite ");
    Serial.println("  Wi-Fi 2.4GHz & Bluetooth LE Signal Tracker & RSSI Meter");
    Serial.println("========================================================");

    // 1. Initialize Display
    if (!Display.begin()) {
        Serial.println("[ERR] Display initialization failed!");
    } else {
        Serial.println("[OK] ST7789 240x320 Display initialized.");
    }

    // 2. Initialize Hardware Joystick
    Joystick.begin();
    Serial.println("[OK] 5-Way Joystick initialized.");

    // 3. Initialize LED Wrench Jaw Meter (20 WS2812B LEDs on GPIO 3)
    Meter.begin();
    Serial.println("[OK] 20 WS2812B Wrench Jaw LEDs initialized.");

    // 4. Initialize Scanner Engine
    RadioScanner.begin();
    Serial.printf("[OK] Radio scanner initialized (Protocol: %s).\n", RadioScanner.getProtocolName());

    // 5. Initialize Foxhunt UI
    UI.begin();
    Serial.println("[Ready] Foxhunter is online and scanning!");
}

void handleSerialCommands() {
    static char lineBuf[64];
    static uint8_t linePos = 0;

    while (Serial.available()) {
        char c = Serial.read();
        if (c == '\r') continue;
        if (c == '\n') {
            lineBuf[linePos] = '\0';
            if (linePos > 0) {
                char *token = strtok(lineBuf, " ");
                if (token) {
                    if (strcasecmp(token, "help") == 0) {
                        Serial.println("--- SaintCon 2025 Foxhunter CLI ---");
                        Serial.println("  scan               - Trigger new target scan");
                        Serial.println("  list               - Print discovered targets");
                        Serial.println("  track <index>      - Track target by index");
                        Serial.println("  track <mac>        - Track target by MAC / BSSID");
                        Serial.println("  stop               - Stop tracking and return to list");
                        Serial.println("  proto <wifi|ble>   - Switch wireless protocol");
                        Serial.println("  theme <0-7|name>   - Set LED color palette theme");
                        Serial.println("  themes             - List available LED color themes");
                        Serial.println("  bright <0-255>     - Adjust LED meter brightness");
                        Serial.println("  options            - Show current LED theme & brightness");
                        Serial.println("  status             - Show current tracking status");
                    } else if (strcasecmp(token, "status") == 0) {
                        Serial.printf("Protocol: %s, State: %s\n",
                                      RadioScanner.getProtocolName(),
                                      RadioScanner.isTracking() ? "TRACKING" : (RadioScanner.isScanning() ? "SCANNING" : "IDLE"));
                        Serial.printf("LED Theme: %s, Brightness: %d (%d%%)\n",
                                      Meter.getCurrentThemeName(),
                                      Meter.getBrightness(),
                                      (Meter.getBrightness() * 100) / 255);
                        if (RadioScanner.isTracking()) {
                            Serial.printf("Target: %s (%s), RSSI: %d dBm, Peak: %d dBm, Signal: %.1f%%\n",
                                          RadioScanner.getTrackedTarget().name.c_str(),
                                          RadioScanner.getTrackedTarget().address.c_str(),
                                          RadioScanner.getCurrentRssi(),
                                          RadioScanner.getPeakRssi(),
                                          RadioScanner.getSignalPercent());
                        } else {
                            Serial.printf("Discovered targets: %d\n", RadioScanner.getTargetCount());
                        }
                    } else if (strcasecmp(token, "options") == 0) {
                        Serial.printf("LED Color Theme: %s (ID: %d)\n", Meter.getCurrentThemeName(), (int)Meter.getTheme());
                        Serial.printf("LED Brightness:  %d / 255 (%d%%)\n", Meter.getBrightness(), (Meter.getBrightness() * 100) / 255);
                    } else if (strcasecmp(token, "themes") == 0) {
                        Serial.println("Available LED Color Themes:");
                        for (int i = 0; i < THEME_COUNT; i++) {
                            Serial.printf(" [%d] %s%s\n", i, Meter.getThemeName((ColorTheme)i),
                                          (Meter.getTheme() == (ColorTheme)i) ? "  <- ACTIVE" : "");
                        }
                    } else if (strcasecmp(token, "theme") == 0) {
                        char *arg = strtok(NULL, " ");
                        if (arg) {
                            int thIdx = -1;
                            if (isdigit(arg[0])) {
                                thIdx = atoi(arg);
                            } else {
                                for (int i = 0; i < THEME_COUNT; i++) {
                                    if (strcasestr(Meter.getThemeName((ColorTheme)i), arg)) {
                                        thIdx = i;
                                        break;
                                    }
                                }
                            }
                            if (thIdx >= 0 && thIdx < THEME_COUNT) {
                                Meter.setTheme((ColorTheme)thIdx);
                                Meter.saveSettings();
                                Serial.printf("Theme set to: %s\n", Meter.getCurrentThemeName());
                            } else {
                                Serial.println("Invalid theme. Type 'themes' to list options.");
                            }
                        } else {
                            Serial.printf("Current Theme: %s. Type 'themes' to list options.\n", Meter.getCurrentThemeName());
                        }
                    } else if (strcasecmp(token, "scan") == 0) {
                        RadioScanner.startScan();
                        Serial.printf("Scanning for %s targets...\n", RadioScanner.getProtocolName());
                    } else if (strcasecmp(token, "list") == 0) {
                        int count = RadioScanner.getTargetCount();
                        Serial.printf("Discovered %d %s targets:\n", count, RadioScanner.getProtocolName());
                        for (int i = 0; i < count; i++) {
                            TargetDevice d;
                            if (RadioScanner.getTarget(i, d)) {
                                Serial.printf(" [%2d] RSSI: %3d dBm  MAC: %s  Ch: %2d  Name: %s\n",
                                              i, d.rssi, d.address.c_str(), d.channel, d.name.c_str());
                            }
                        }
                    } else if (strcasecmp(token, "proto") == 0) {
                        char *arg = strtok(NULL, " ");
                        if (arg) {
                            if (strcasecmp(arg, "wifi") == 0) {
                                RadioScanner.setProtocol(PROTO_WIFI);
                                Serial.println("Switched protocol to Wi-Fi 2.4GHz");
                            } else if (strcasecmp(arg, "ble") == 0) {
                                RadioScanner.setProtocol(PROTO_BLE);
                                Serial.println("Switched protocol to Bluetooth LE");
                            }
                        }
                    } else if (strcasecmp(token, "track") == 0) {
                        char *arg = strtok(NULL, " ");
                        if (arg) {
                            if (strchr(arg, ':')) {
                                RadioScanner.selectTargetByAddress(arg);
                                UI.setScreen(SCREEN_TRACKING);
                                Serial.printf("Tracking MAC: %s\n", arg);
                            } else {
                                int idx = atoi(arg);
                                RadioScanner.selectTarget(idx);
                                UI.setScreen(SCREEN_TRACKING);
                                Serial.printf("Tracking target index: %d\n", idx);
                            }
                        }
                    } else if (strcasecmp(token, "stop") == 0) {
                        RadioScanner.stopTracking();
                        UI.setScreen(SCREEN_TARGET_LIST);
                        Serial.println("Stopped tracking.");
                    } else if (strcasecmp(token, "bright") == 0) {
                        char *arg = strtok(NULL, " ");
                        if (arg) {
                            Meter.setBrightness(atoi(arg));
                            Meter.saveSettings();
                            Serial.printf("Brightness set to %d\n", Meter.getBrightness());
                        }
                    }
                }
            }
            linePos = 0;
        } else {
            if (linePos < sizeof(lineBuf) - 1) {
                lineBuf[linePos++] = c;
            }
        }
    }
}

void loop() {
    // 1. Process Hardware Joystick Inputs
    JoystickEvent event = Joystick.update();
    if (event != JOY_NONE) {
        UI.handleInput(event);
    }

    // 2. Process USB Serial Commands (CLI)
    handleSerialCommands();

    // 3. Advance Radio Scanner & RSSI Tracking
    RadioScanner.update();

    // 4. Update 20 WS2812B LEDs on the Wrench Jaws
    Meter.update();

    // 5. Update ST7789 Color Display
    UI.update();

    // 6. Diagnostics Heartbeat
    static unsigned long lastHb = 0;
    if (millis() - lastHb > 4000) {
        lastHb = millis();
        if (RadioScanner.isTracking()) {
            Serial.printf("[Foxhunt] Target: %s, RSSI: %d dBm, Signal: %d%%, Rate: %.1f pkts/s\n",
                          RadioScanner.getTrackedTarget().name.c_str(),
                          RadioScanner.getCurrentRssi(),
                          (int)RadioScanner.getSignalPercent(),
                          RadioScanner.getPacketsPerSec());
        }
    }

    // 7. Cooperative RTOS yield
    delay(1);
}
