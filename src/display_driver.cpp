#include "display_driver.h"
#include <string.h>

DisplayDriver Display;

// Standard 5x7 ASCII font (Characters 32 to 127)
static const uint8_t FONT_5X7[96][5] PROGMEM = {
    {0x00, 0x00, 0x00, 0x00, 0x00}, // ' ' (32)
    {0x00, 0x00, 0x5F, 0x00, 0x00}, // '!'
    {0x00, 0x07, 0x00, 0x07, 0x00}, // '"'
    {0x14, 0x7F, 0x14, 0x7F, 0x14}, // '#'
    {0x24, 0x2A, 0x7F, 0x2A, 0x12}, // '$'
    {0x23, 0x13, 0x08, 0x64, 0x62}, // '%'
    {0x36, 0x49, 0x55, 0x22, 0x50}, // '&'
    {0x00, 0x05, 0x03, 0x00, 0x00}, // '''
    {0x00, 0x1C, 0x22, 0x41, 0x00}, // '('
    {0x00, 0x41, 0x22, 0x1C, 0x00}, // ')'
    {0x14, 0x08, 0x3E, 0x08, 0x14}, // '*'
    {0x08, 0x08, 0x3E, 0x08, 0x08}, // '+'
    {0x00, 0x50, 0x30, 0x00, 0x00}, // ','
    {0x08, 0x08, 0x08, 0x08, 0x08}, // '-'
    {0x00, 0x60, 0x60, 0x00, 0x00}, // '.'
    {0x20, 0x10, 0x08, 0x04, 0x02}, // '/'
    {0x3E, 0x51, 0x49, 0x45, 0x3E}, // '0' (48)
    {0x00, 0x42, 0x7F, 0x40, 0x00}, // '1'
    {0x42, 0x61, 0x51, 0x49, 0x46}, // '2'
    {0x21, 0x41, 0x45, 0x4B, 0x31}, // '3'
    {0x18, 0x14, 0x12, 0x7F, 0x10}, // '4'
    {0x27, 0x45, 0x45, 0x45, 0x39}, // '5'
    {0x3C, 0x4A, 0x49, 0x49, 0x30}, // '6'
    {0x01, 0x71, 0x09, 0x05, 0x03}, // '7'
    {0x36, 0x49, 0x49, 0x49, 0x36}, // '8'
    {0x06, 0x49, 0x49, 0x29, 0x1E}, // '9'
    {0x00, 0x36, 0x36, 0x00, 0x00}, // ':'
    {0x00, 0x56, 0x36, 0x00, 0x00}, // ';'
    {0x08, 0x14, 0x22, 0x41, 0x00}, // '<'
    {0x14, 0x14, 0x14, 0x14, 0x14}, // '='
    {0x00, 0x41, 0x22, 0x14, 0x08}, // '>'
    {0x02, 0x01, 0x51, 0x09, 0x06}, // '?'
    {0x32, 0x49, 0x79, 0x41, 0x3E}, // '@'
    {0x7E, 0x11, 0x11, 0x11, 0x7E}, // 'A' (65)
    {0x7F, 0x49, 0x49, 0x49, 0x36}, // 'B'
    {0x3E, 0x41, 0x41, 0x41, 0x22}, // 'C'
    {0x7F, 0x41, 0x41, 0x22, 0x1C}, // 'D'
    {0x7F, 0x49, 0x49, 0x49, 0x41}, // 'E'
    {0x7F, 0x09, 0x09, 0x09, 0x01}, // 'F'
    {0x3E, 0x41, 0x49, 0x49, 0x7A}, // 'G'
    {0x7F, 0x08, 0x08, 0x08, 0x7F}, // 'H'
    {0x00, 0x41, 0x7F, 0x41, 0x00}, // 'I'
    {0x20, 0x40, 0x41, 0x3F, 0x01}, // 'J'
    {0x7F, 0x08, 0x14, 0x22, 0x41}, // 'K'
    {0x7F, 0x40, 0x40, 0x40, 0x40}, // 'L'
    {0x7F, 0x02, 0x0C, 0x02, 0x7F}, // 'M'
    {0x7F, 0x04, 0x08, 0x10, 0x7F}, // 'N'
    {0x3E, 0x41, 0x41, 0x41, 0x3E}, // 'O'
    {0x7F, 0x09, 0x09, 0x09, 0x06}, // 'P'
    {0x3E, 0x41, 0x51, 0x21, 0x5E}, // 'Q'
    {0x7F, 0x09, 0x19, 0x29, 0x46}, // 'R'
    {0x46, 0x49, 0x49, 0x49, 0x31}, // 'S'
    {0x01, 0x01, 0x7F, 0x01, 0x01}, // 'T'
    {0x3F, 0x40, 0x40, 0x40, 0x3F}, // 'U'
    {0x1F, 0x20, 0x40, 0x20, 0x1F}, // 'V'
    {0x7F, 0x20, 0x18, 0x20, 0x7F}, // 'W'
    {0x63, 0x14, 0x08, 0x14, 0x63}, // 'X'
    {0x07, 0x08, 0x70, 0x08, 0x07}, // 'Y'
    {0x61, 0x51, 0x49, 0x45, 0x43}, // 'Z'
    {0x00, 0x7F, 0x41, 0x41, 0x00}, // '['
    {0x02, 0x04, 0x08, 0x10, 0x20}, // '\'
    {0x00, 0x41, 0x41, 0x7F, 0x00}, // ']'
    {0x04, 0x02, 0x01, 0x02, 0x04}, // '^'
    {0x40, 0x40, 0x40, 0x40, 0x40}, // '_'
    {0x00, 0x01, 0x02, 0x04, 0x00}, // '`'
    {0x20, 0x54, 0x54, 0x54, 0x78}, // 'a' (97)
    {0x7F, 0x48, 0x44, 0x44, 0x38}, // 'b'
    {0x38, 0x44, 0x44, 0x44, 0x20}, // 'c'
    {0x38, 0x44, 0x44, 0x48, 0x7F}, // 'd'
    {0x38, 0x54, 0x54, 0x54, 0x18}, // 'e'
    {0x08, 0x7E, 0x09, 0x01, 0x02}, // 'f'
    {0x0C, 0x52, 0x52, 0x52, 0x3E}, // 'g'
    {0x7F, 0x08, 0x04, 0x04, 0x78}, // 'h'
    {0x00, 0x44, 0x7D, 0x40, 0x00}, // 'i'
    {0x20, 0x40, 0x44, 0x3D, 0x00}, // 'j'
    {0x7F, 0x10, 0x28, 0x44, 0x00}, // 'k'
    {0x00, 0x41, 0x7F, 0x40, 0x00}, // 'l'
    {0x7C, 0x04, 0x18, 0x04, 0x78}, // 'm'
    {0x7C, 0x08, 0x04, 0x04, 0x78}, // 'n'
    {0x38, 0x44, 0x44, 0x44, 0x38}, // 'o'
    {0x7C, 0x14, 0x14, 0x14, 0x08}, // 'p'
    {0x08, 0x14, 0x14, 0x18, 0x7C}, // 'q'
    {0x7C, 0x08, 0x04, 0x04, 0x08}, // 'r'
    {0x48, 0x54, 0x54, 0x54, 0x20}, // 's'
    {0x04, 0x3F, 0x44, 0x40, 0x20}, // 't'
    {0x3C, 0x40, 0x40, 0x20, 0x7C}, // 'u'
    {0x1C, 0x20, 0x40, 0x20, 0x1C}, // 'v'
    {0x3C, 0x40, 0x30, 0x40, 0x3C}, // 'w'
    {0x44, 0x28, 0x10, 0x28, 0x44}, // 'x'
    {0x0C, 0x50, 0x50, 0x50, 0x3C}, // 'y'
    {0x44, 0x64, 0x54, 0x4C, 0x44}, // 'z'
    {0x00, 0x08, 0x36, 0x41, 0x00}, // '{'
    {0x00, 0x00, 0x7F, 0x00, 0x00}, // '|'
    {0x00, 0x41, 0x36, 0x08, 0x00}, // '}'
    {0x08, 0x08, 0x2A, 0x1C, 0x08}, // '~'
    {0x7F, 0x7F, 0x7F, 0x7F, 0x7F}  // Delta / block (127)
};

DisplayDriver::DisplayDriver()
    : _i80_bus(NULL),
      _io_handle(NULL),
      _panel_handle(NULL),
      _framebuffer(NULL),
      _initialized(false),
      _sleeping(false)
{
}

bool DisplayDriver::begin() {
    if (_initialized) return true;

    // Pull RD pin high
    pinMode(LCD_PIN_RD, OUTPUT);
    digitalWrite(LCD_PIN_RD, HIGH);

    // Turn on backlight pin
    pinMode(LCD_PIN_BL, OUTPUT);
    digitalWrite(LCD_PIN_BL, HIGH);

    // Setup Intel 8080 parallel bus
    esp_lcd_i80_bus_config_t bus_config = {
        .dc_gpio_num = LCD_PIN_DC,
        .wr_gpio_num = LCD_PIN_WR,
        .clk_src = LCD_CLK_SRC_PLL240M,
        .data_gpio_nums = {
            LCD_PIN_DATA0, LCD_PIN_DATA1, LCD_PIN_DATA2, LCD_PIN_DATA3,
            LCD_PIN_DATA4, LCD_PIN_DATA5, LCD_PIN_DATA6, LCD_PIN_DATA7
        },
        .bus_width = 8,
        .max_transfer_bytes = LCD_WIDTH * LCD_HEIGHT * sizeof(uint16_t),
        .dma_burst_size = 64,
    };
    esp_err_t ret = esp_lcd_new_i80_bus(&bus_config, &_i80_bus);
    if (ret != ESP_OK) return false;

    esp_lcd_panel_io_i80_config_t io_config = {
        .cs_gpio_num = LCD_PIN_CS,
        .pclk_hz = 10 * 1000 * 1000,
        .trans_queue_depth = 10,
        .on_color_trans_done = NULL,
        .user_ctx = NULL,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
        .dc_levels = {
            .dc_idle_level = 0,
            .dc_cmd_level = 0,
            .dc_dummy_level = 0,
            .dc_data_level = 1,
        },
        .flags = {
            .cs_active_high = 0,
            .reverse_color_bits = 0,
            .swap_color_bytes = 1,
            .pclk_active_neg = 0,
            .pclk_idle_low = 0,
        },
    };
    ret = esp_lcd_new_panel_io_i80(_i80_bus, &io_config, &_io_handle);
    if (ret != ESP_OK) return false;

    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = LCD_PIN_RST,
        .rgb_endian = LCD_RGB_ENDIAN_RGB,
        .bits_per_pixel = 16,
    };
    ret = esp_lcd_new_panel_st7789(_io_handle, &panel_config, &_panel_handle);
    if (ret != ESP_OK) return false;

    esp_lcd_panel_reset(_panel_handle);
    esp_lcd_panel_init(_panel_handle);
    esp_lcd_panel_invert_color(_panel_handle, true);
    esp_lcd_panel_set_gap(_panel_handle, 0, 0);
    esp_lcd_panel_swap_xy(_panel_handle, false);
    esp_lcd_panel_mirror(_panel_handle, false, false);
    esp_lcd_panel_disp_on_off(_panel_handle, true);

    // Allocate 16-bit draw buffer (240x320x2 = 153.6 KB)
    _framebuffer = (uint16_t*)heap_caps_malloc(LCD_WIDTH * LCD_HEIGHT * sizeof(uint16_t), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (!_framebuffer) {
        _framebuffer = (uint16_t*)malloc(LCD_WIDTH * LCD_HEIGHT * sizeof(uint16_t));
    }
    if (!_framebuffer) return false;

    clear(COLOR565_DARK_NAVY);
    flush();

    _initialized = true;
    return true;
}

void DisplayDriver::setBacklight(uint8_t brightness) {
    if (brightness > 0) {
        digitalWrite(LCD_PIN_BL, HIGH);
    } else {
        digitalWrite(LCD_PIN_BL, LOW);
    }
}

void DisplayDriver::sleep() {
    if (_sleeping) return;
    setBacklight(0);
    if (_panel_handle) {
        esp_lcd_panel_disp_on_off(_panel_handle, false);
    }
    _sleeping = true;
}

void DisplayDriver::wakeup() {
    if (!_sleeping) return;
    if (_panel_handle) {
        esp_lcd_panel_disp_on_off(_panel_handle, true);
    }
    setBacklight(255);
    _sleeping = false;
}

void DisplayDriver::clear(uint16_t color) {
    if (!_framebuffer) return;
    for (int i = 0; i < LCD_WIDTH * LCD_HEIGHT; i++) {
        _framebuffer[i] = color;
    }
}

void DisplayDriver::flush() {
    if (!_framebuffer || !_panel_handle) return;
    esp_lcd_panel_draw_bitmap(_panel_handle, 0, 0, LCD_WIDTH, LCD_HEIGHT, _framebuffer);
}

void DisplayDriver::drawPixel(int x, int y, uint16_t color) {
    if (x < 0 || x >= LCD_WIDTH || y < 0 || y >= LCD_HEIGHT || !_framebuffer) return;
    _framebuffer[y * LCD_WIDTH + x] = color;
}

void DisplayDriver::drawFastHLine(int x, int y, int w, uint16_t color) {
    if (y < 0 || y >= LCD_HEIGHT || !_framebuffer) return;
    if (x < 0) { w += x; x = 0; }
    if (x + w > LCD_WIDTH) w = LCD_WIDTH - x;
    if (w <= 0) return;

    uint16_t *p = &_framebuffer[y * LCD_WIDTH + x];
    while (w--) *p++ = color;
}

void DisplayDriver::drawFastVLine(int x, int y, int h, uint16_t color) {
    if (x < 0 || x >= LCD_WIDTH || !_framebuffer) return;
    if (y < 0) { h += y; y = 0; }
    if (y + h > LCD_HEIGHT) h = LCD_HEIGHT - y;
    if (h <= 0) return;

    uint16_t *p = &_framebuffer[y * LCD_WIDTH + x];
    while (h--) {
        *p = color;
        p += LCD_WIDTH;
    }
}

void DisplayDriver::fillRect(int x, int y, int w, int h, uint16_t color) {
    if (x >= LCD_WIDTH || y >= LCD_HEIGHT || w <= 0 || h <= 0) return;
    if (x < 0) { w += x; x = 0; }
    if (y < 0) { h += y; y = 0; }
    if (x + w > LCD_WIDTH) w = LCD_WIDTH - x;
    if (y + h > LCD_HEIGHT) h = LCD_HEIGHT - y;

    for (int row = y; row < y + h; row++) {
        uint16_t *p = &_framebuffer[row * LCD_WIDTH + x];
        for (int col = 0; col < w; col++) {
            p[col] = color;
        }
    }
}

void DisplayDriver::drawRect(int x, int y, int w, int h, uint16_t color) {
    drawFastHLine(x, y, w, color);
    drawFastHLine(x, y + h - 1, w, color);
    drawFastVLine(x, y, h, color);
    drawFastVLine(x + w - 1, y, h, color);
}

void DisplayDriver::fillRoundRect(int x, int y, int w, int h, int r, uint16_t color) {
    fillRect(x + r, y, w - 2 * r, h, color);
    // Draw corner edges
    for (int i = 0; i < r; i++) {
        int dx = r - (int)sqrtf((float)(r * r - (r - i) * (r - i)));
        drawFastHLine(x + dx, y + i, w - 2 * dx, color);
        drawFastHLine(x + dx, y + h - 1 - i, w - 2 * dx, color);
    }
}

void DisplayDriver::drawRoundRect(int x, int y, int w, int h, int r, uint16_t color) {
    drawFastHLine(x + r, y, w - 2 * r, color);
    drawFastHLine(x + r, y + h - 1, w - 2 * r, color);
    drawFastVLine(x, y + r, h - 2 * r, color);
    drawFastVLine(x + w - 1, y + r, h - 2 * r, color);

    for (int i = 0; i < r; i++) {
        int dx = r - (int)sqrtf((float)(r * r - (r - i) * (r - i)));
        drawPixel(x + dx, y + i, color);
        drawPixel(x + w - 1 - dx, y + i, color);
        drawPixel(x + dx, y + h - 1 - i, color);
        drawPixel(x + w - 1 - dx, y + h - 1 - i, color);
    }
}

void DisplayDriver::drawCircle(int x0, int y0, int r, uint16_t color) {
    int f = 1 - r;
    int ddF_x = 1;
    int ddF_y = -2 * r;
    int x = 0;
    int y = r;

    drawPixel(x0, y0 + r, color);
    drawPixel(x0, y0 - r, color);
    drawPixel(x0 + r, y0, color);
    drawPixel(x0 - r, y0, color);

    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        drawPixel(x0 + x, y0 + y, color);
        drawPixel(x0 - x, y0 + y, color);
        drawPixel(x0 + x, y0 - y, color);
        drawPixel(x0 - x, y0 - y, color);
        drawPixel(x0 + y, y0 + x, color);
        drawPixel(x0 - y, y0 + x, color);
        drawPixel(x0 + y, y0 - x, color);
        drawPixel(x0 - y, y0 - x, color);
    }
}

void DisplayDriver::fillCircle(int x0, int y0, int r, uint16_t color) {
    drawFastVLine(x0, y0 - r, 2 * r + 1, color);
    int f = 1 - r;
    int ddF_x = 1;
    int ddF_y = -2 * r;
    int x = 0;
    int y = r;

    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        drawFastVLine(x0 + x, y0 - y, 2 * y + 1, color);
        drawFastVLine(x0 - x, y0 - y, 2 * y + 1, color);
        drawFastVLine(x0 + y, y0 - x, 2 * x + 1, color);
        drawFastVLine(x0 - y, y0 - x, 2 * x + 1, color);
    }
}

void DisplayDriver::drawLine(int x0, int y0, int x1, int y1, uint16_t color) {
    int steep = abs(y1 - y0) > abs(x1 - x0);
    if (steep) {
        int t = x0; x0 = y0; y0 = t;
        t = x1; x1 = y1; y1 = t;
    }
    if (x0 > x1) {
        int t = x0; x0 = x1; x1 = t;
        t = y0; y0 = y1; y1 = t;
    }

    int dx = x1 - x0;
    int dy = abs(y1 - y0);
    int err = dx / 2;
    int ystep = (y0 < y1) ? 1 : -1;
    int y = y0;

    for (int x = x0; x <= x1; x++) {
        if (steep) {
            drawPixel(y, x, color);
        } else {
            drawPixel(x, y, color);
        }
        err -= dy;
        if (err < 0) {
            y += ystep;
            err += dx;
        }
    }
}

void DisplayDriver::drawChar(int x, int y, char c, uint16_t color, uint16_t bg, uint8_t size) {
    if (c < 32 || c > 127) c = '?';
    int fontIdx = c - 32;

    for (int col = 0; col < 5; col++) {
        uint8_t line = pgm_read_byte(&FONT_5X7[fontIdx][col]);
        for (int row = 0; row < 7; row++) {
            bool pixelOn = (line >> row) & 1;
            uint16_t pCol = pixelOn ? color : bg;

            if (pCol != 0 || bg != 0 || pixelOn) {
                if (size == 1) {
                    if (pixelOn || bg != 0) drawPixel(x + col, y + row, pCol);
                } else {
                    if (pixelOn || bg != 0) fillRect(x + col * size, y + row * size, size, size, pCol);
                }
            }
        }
    }

    // 1-pixel spacing column
    if (bg != 0) {
        if (size == 1) {
            drawFastVLine(x + 5, y, 7, bg);
        } else {
            fillRect(x + 5 * size, y, size, 7 * size, bg);
        }
    }
}

void DisplayDriver::drawString(const char *str, int x, int y, uint16_t color, uint16_t bg, uint8_t size) {
    if (!str) return;
    int curX = x;
    int charWidth = 6 * size;

    while (*str) {
        if (*str == '\n') {
            curX = x;
            y += 8 * size;
        } else {
            drawChar(curX, y, *str, color, bg, size);
            curX += charWidth;
        }
        str++;
    }
}

void DisplayDriver::drawStringCenter(const char *str, int y, uint16_t color, uint16_t bg, uint8_t size) {
    if (!str) return;
    int len = strlen(str);
    int totalWidth = len * 6 * size;
    int x = (LCD_WIDTH - totalWidth) / 2;
    if (x < 0) x = 0;
    drawString(str, x, y, color, bg, size);
}
