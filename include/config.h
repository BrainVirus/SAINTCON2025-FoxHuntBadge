#pragma once

#include <Arduino.h>

// =============================================================================
// SaintCon 2025 Badge Hardware Pinout Configuration - Foxhunter Edition
// =============================================================================

// Addressable RGB LEDs (WS2812B-2020)
#define LED_PIN             3      // GPIO connected to WS2812B DIN
#define NUM_LEDS            20     // 20 LEDs total along the wrench jaws
#define LEDS_PER_JAW        10     // 10 LEDs per jaw (Left: 9->0, Right: 10->19)

// 5-Way Joystick Navigation
#define JOY_UP_PIN          41     // Active LOW, internal pull-up
#define JOY_DOWN_PIN        43     // Active LOW, internal pull-up
#define JOY_LEFT_PIN        2      // Active LOW, internal pull-up
#define JOY_RIGHT_PIN       42     // Active LOW, internal pull-up
#define JOY_CENTER_PIN      44     // Active LOW, internal pull-up

#define JOY_PIN_UP          JOY_UP_PIN
#define JOY_PIN_DOWN        JOY_DOWN_PIN
#define JOY_PIN_LEFT        JOY_LEFT_PIN
#define JOY_PIN_RIGHT       JOY_RIGHT_PIN
#define JOY_PIN_CENTER      JOY_CENTER_PIN

// ST7789 240x320 Display (Intel 8080 8-Bit Parallel Interface)
#define LCD_WIDTH           240
#define LCD_HEIGHT          320

#define LCD_PIN_DATA0       35
#define LCD_PIN_DATA1       48
#define LCD_PIN_DATA2       47
#define LCD_PIN_DATA3       21
#define LCD_PIN_DATA4       14
#define LCD_PIN_DATA5       13
#define LCD_PIN_DATA6       12
#define LCD_PIN_DATA7       11

#define LCD_PIN_DC          38     // Data / Command select
#define LCD_PIN_CS          39     // Chip Select (active LOW)
#define LCD_PIN_RD          36     // Read strobe (pull HIGH for write-only)
#define LCD_PIN_WR          37     // Write strobe clock
#define LCD_PIN_RST         40     // Hardware Reset (active LOW)
#define LCD_PIN_BL          18     // Backlight control (HIGH = On)

// Serial Baud Rate
#define SERIAL_BAUD_RATE    115200

// Foxhunter RSSI Range Calibration (dBm)
#define FOXHUNT_MIN_RSSI    -95    // Weakest threshold (0% / 0 levels)
#define FOXHUNT_MAX_RSSI    -28    // Strongest threshold (100% / 10 levels)
