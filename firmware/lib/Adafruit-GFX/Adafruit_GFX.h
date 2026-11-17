#pragma once

#include "../../graphics/color.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

void adafruit_gfx_drawFastVLine(int16_t x, int16_t y, int16_t h, color_t color);
void adafruit_gfx_drawFastHLine(int16_t x, int16_t y, int16_t w, color_t color);
void adafruit_gfx_fillRect(int16_t x, int16_t y, int16_t w, int16_t h,
                           color_t color);
void adafruit_gfx_fillScreen(color_t color);
void adafruit_gfx_drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
                           color_t color);
void adafruit_gfx_drawRect(int16_t x, int16_t y, int16_t w, int16_t h,
                           color_t color);
void adafruit_gfx_drawCircle(int16_t x0, int16_t y0, int16_t r, color_t color);
void adafruit_gfx_fillCircle(int16_t x0, int16_t y0, int16_t r, color_t color);
void adafruit_gfx_fillQuarterCircle(int16_t x0, int16_t y0, int16_t r,
                                    uint8_t corners, color_t color);
void adafruit_gfx_drawEllipse(int16_t x0, int16_t y0, int16_t rw, int16_t rh,
                              color_t color);
void adafruit_gfx_fillEllipse(int16_t x0, int16_t y0, int16_t rw, int16_t rh,
                              color_t color);
void adafruit_gfx_drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
                               int16_t x2, int16_t y2, color_t color);
void adafruit_gfx_fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
                               int16_t x2, int16_t y2, color_t color);
void adafruit_gfx_drawRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h,
                                int16_t radius, color_t color);
void adafruit_gfx_fillRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h,
                                int16_t radius, color_t color);
void adafruit_gfx_drawRotatedRect(int16_t cenX, int16_t cenY, int16_t w,
                                  int16_t h, int16_t angleDeg, color_t color);
void adafruit_gfx_fillRotatedRect(int16_t cenX, int16_t cenY, int16_t w,
                                  int16_t h, int16_t angleDeg, color_t color);
void adafruit_gfx_rotatePoint(int16_t *x0, int16_t *y0, int16_t angleDeg);
void adafruit_gfx_drawBitmap(int16_t x, int16_t y, const uint8_t *bitmap,
                             int16_t w, int16_t h, color_t color);
void adafruit_gfx_drawChar(int16_t x, int16_t y, unsigned char c, color_t color,
                           uint8_t size_x, uint8_t size_y);
void adafruit_gfx_getTextBounds(const char *string, int16_t x, int16_t y,
                                int16_t *x1, int16_t *y1, uint16_t *w,
                                uint16_t *h);
void adafruit_gfx_setCursor(int16_t x, int16_t y);
void adafruit_gfx_setTextSize(uint8_t s_x, uint8_t s_y);
void adafruit_gfx_setTextColor(color_t c);
void adafruit_gfx_setTextWrap(bool w);
void adafruit_gfx_print(const char *str);
void adafruit_gfx_write(char c);
int16_t adafruit_gfx_getCursorX(void);
int16_t adafruit_gfx_getCursorY(void);
