#pragma once

#include <Arduino.h>
#include "esp_lcd_panel_io.h"
#include "esp_lcd_io_i80.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "config.h"

// Common 16-bit RGB565 Colors
#define COLOR565_BLACK          0x0000
#define COLOR565_WHITE          0xFFFF
#define COLOR565_RED            0xF800
#define COLOR565_GREEN          0x07E0
#define COLOR565_BLUE           0x001F
#define COLOR565_CYAN           0x07FF
#define COLOR565_MAGENTA        0xF81F
#define COLOR565_YELLOW         0xFFE0
#define COLOR565_ORANGE         0xFD20
#define COLOR565_GOLD           0xFDB0  // SaintCon Gold (~245, 179, 0)
#define COLOR565_CYBER_CYAN     0x077F  // Cyber Cyan (~0, 240, 255)
#define COLOR565_DARK_NAVY      0x08A5  // Rich dark background
#define COLOR565_CARD_BG        0x110B  // Card background
#define COLOR565_BORDER         0x2190  // Subtle border
#define COLOR565_ACCENT_BLUE    0x1B38  // Card selection highlight
#define COLOR565_TEXT_MUTED     0x8410  // Grey text
#define COLOR565_NEON_GREEN     0x07E8  // Bright radar green
#define COLOR565_HOT_ORANGE     0xFA40  // Hot RSSI orange
#define COLOR565_FIRE_RED       0xF900  // Close target red

class DisplayDriver {
public:
    DisplayDriver();

    bool begin();
    void setBacklight(uint8_t brightness);

    void sleep();
    void wakeup();
    bool isSleeping() const { return _sleeping; }

    // Framebuffer operations
    void clear(uint16_t color = COLOR565_BLACK);
    void flush();

    // 2D Drawing Primitives
    void drawPixel(int x, int y, uint16_t color);
    void drawFastHLine(int x, int y, int w, uint16_t color);
    void drawFastVLine(int x, int y, int h, uint16_t color);
    void fillRect(int x, int y, int w, int h, uint16_t color);
    void drawRect(int x, int y, int w, int h, uint16_t color);
    void fillRoundRect(int x, int y, int w, int h, int r, uint16_t color);
    void drawRoundRect(int x, int y, int w, int h, int r, uint16_t color);
    void drawLine(int x0, int y0, int x1, int y1, uint16_t color);
    void drawCircle(int x0, int y0, int r, uint16_t color);
    void fillCircle(int x0, int y0, int r, uint16_t color);

    // Text Rendering (5x7 Embedded ASCII Font)
    void drawChar(int x, int y, char c, uint16_t color, uint16_t bg = COLOR565_BLACK, uint8_t size = 1);
    void drawString(const char *text, int x, int y, uint16_t color, uint16_t bg = 0, uint8_t size = 1);
    void drawStringCenter(const char *text, int y, uint16_t color, uint16_t bg = 0, uint8_t size = 1);

    // Color conversion
    static uint16_t rgbTo565(uint8_t r, uint8_t g, uint8_t b) {
        return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
    }

private:
    esp_lcd_i80_bus_handle_t _i80_bus;
    esp_lcd_panel_io_handle_t _io_handle;
    esp_lcd_panel_handle_t _panel_handle;
    uint16_t *_framebuffer;
    bool _initialized;
    bool _sleeping;

    static const uint8_t FONT5X7[96][5];
};

extern DisplayDriver Display;
