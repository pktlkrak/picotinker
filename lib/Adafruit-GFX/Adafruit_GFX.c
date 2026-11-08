/*
This is the core graphics library for all our displays, providing a common
set of graphics primitives (points, lines, circles, etc.).  It needs to be
paired with a hardware-specific library for each display device we carry
(to handle the lower-level functions).

Adafruit invests time and resources providing this open source code, please
support Adafruit & open-source hardware by purchasing products from Adafruit!

Copyright (c) 2013 Adafruit Industries.  All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

- Redistributions of source code must retain the above copyright notice,
  this list of conditions and the following disclaimer.
- Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
 */

#include "Adafruit_GFX.h"

#include "../../graphics/graphics.h"
#include "glcdfont.c"
#include "hil.h"
#include <math.h>
#include <stdint.h>
#include <stdlib.h>

#define DEGREES_TO_RADIANS 57.29578f

#define pgm_read_byte(addr) (*(const unsigned char *)(addr))

static int16_t cursor_x = 0; ///< x location to start print()ing text
static int16_t cursor_y = 0; ///< y location to start print()ing text
static color_t textcolor = {255, 255, 255}; ///< text color for print()
static uint8_t textsize_x =
    1; ///< Desired magnification in X-axis of text to print()
static uint8_t textsize_y =
    1; ///< Desired magnification in Y-axis of text to print()
static bool wrap = true;

/**************************************************************************/
/*!
   @brief    Draw a perfectly vertical line
    @param    x   Top-most x coordinate
    @param    y   Top-most y coordinate
    @param    h   Height in pixels
   @param    color Color to fill with
*/
/**************************************************************************/
void adafruit_gfx_drawFastVLine(int16_t x, int16_t y, int16_t h,
                                color_t color) {
    int16_t y_end = y + h;
    if (y < 0)
        y = 0;
    if (y_end > SCREEN_SIZE_Y) {
        y_end = SCREEN_SIZE_Y;
    }
    for (int16_t pos_y = y; pos_y < y_end; pos_y++) {
        graphics_draw_pixel(x, pos_y, color);
    }
}

/**************************************************************************/
/*!
   @brief    Draw a perfectly horizontal line
    @param    x   Left-most x coordinate
    @param    y   Left-most y coordinate
    @param    w   Width in pixels
   @param    color Color to fill with
*/
/**************************************************************************/
void adafruit_gfx_drawFastHLine(int16_t x, int16_t y, int16_t w,
                                color_t color) {
    int16_t x_end = x + w;
    if (x < 0)
        x = 0;
    if (x_end > SCREEN_SIZE_X) {
        x_end = SCREEN_SIZE_X;
    }
    for (int16_t pos_x = x; pos_x < x_end; pos_x++) {
        graphics_draw_pixel(pos_x, y, color);
    }
}

/**************************************************************************/
/*!
   @brief    Fill a rectangle completely with one color. Update in subclasses if
   desired!
    @param    x   Top left corner x coordinate
    @param    y   Top left corner y coordinate
    @param    w   Width in pixels
    @param    h   Height in pixels
   @param    color Color to fill with
*/
/**************************************************************************/
void adafruit_gfx_drawFillRect(int16_t x, int16_t y, int16_t w, int16_t h,
                               color_t color) {
    for (int16_t i = x; i < x + w; i++) {
        adafruit_gfx_drawFastVLine(i, y, h, color);
    }
}

/**************************************************************************/
/*!
   @brief    Fill the screen completely with one color. Update in subclasses if
   desired!
    @param    color Color to fill with
*/
/**************************************************************************/
void adafruit_gfx_fillScreen(color_t color) {
    adafruit_gfx_drawFillRect(0, 0, SCREEN_SIZE_X, SCREEN_SIZE_Y, color);
}

/**************************************************************************/
/*!
   @brief    Draw a line
    @param    x0  Start point x coordinate
    @param    y0  Start point y coordinate
    @param    x1  End point x coordinate
    @param    y1  End point y coordinate
    @param    color Color to draw with
*/
/**************************************************************************/
void adafruit_gfx_drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
                           color_t color) {
    // Update in subclasses if desired!
    if (x0 == x1) {
        if (y0 > y1) {
            int16_t tmp = y0;
            y0 = y1;
            y1 = tmp;
        }
        adafruit_gfx_drawFastVLine(x0, y0, y1 - y0 + 1, color);
    } else if (y0 == y1) {
        if (x0 > x1) {
            int16_t tmp = x0;
            x0 = x1;
            x1 = tmp;
        }
        adafruit_gfx_drawFastHLine(x0, y0, x1 - x0 + 1, color);
    } else {
        int16_t steep = abs(y1 - y0) > abs(x1 - x0);
        if (steep) {
            int16_t tmp = x0;
            x0 = y0;
            y0 = tmp;
            tmp = x1;
            x1 = y1;
            y1 = tmp;
        }

        if (x0 > x1) {
            int16_t tmp = y0;
            y0 = y1;
            y1 = tmp;
            tmp = x0;
            x0 = x1;
            x1 = tmp;
        }

        int16_t dx, dy;
        dx = x1 - x0;
        dy = abs(y1 - y0);

        int16_t err = dx / 2;
        int16_t ystep;

        if (y0 < y1) {
            ystep = 1;
        } else {
            ystep = -1;
        }

        for (; x0 <= x1; x0++) {
            if (steep) {
                graphics_draw_pixel(y0, x0, color);
            } else {
                graphics_draw_pixel(x0, y0, color);
            }
            err -= dy;
            if (err < 0) {
                y0 += ystep;
                err += dx;
            }
        }
    }
}

/**************************************************************************/
/*!
   @brief    Draw an ellipse outline
    @param    x0   Center-point x coordinate
    @param    y0   Center-point y coordinate
    @param    rw   Horizontal radius of ellipse
    @param    rh   Vertical radius of ellipse
    @param    color Color to draw with
*/
/**************************************************************************/
void adafruit_gfx_drawEllipse(int16_t x0, int16_t y0, int16_t rw, int16_t rh,
                              color_t color) {
    // Bresenham's ellipse algorithm
    int16_t x = 0, y = rh;
    int32_t rw2 = rw * rw, rh2 = rh * rh;
    int32_t twoRw2 = 2 * rw2, twoRh2 = 2 * rh2;

    int32_t decision = rh2 - (rw2 * rh) + (rw2 / 4);

    // region 1
    while ((twoRh2 * x) < (twoRw2 * y)) {
        graphics_draw_pixel(x0 + x, y0 + y, color);
        graphics_draw_pixel(x0 - x, y0 + y, color);
        graphics_draw_pixel(x0 + x, y0 - y, color);
        graphics_draw_pixel(x0 - x, y0 - y, color);
        x++;
        if (decision < 0) {
            decision += rh2 + (twoRh2 * x);
        } else {
            decision += rh2 + (twoRh2 * x) - (twoRw2 * y);
            y--;
        }
    }

    // region 2
    decision = ((rh2 * (2 * x + 1) * (2 * x + 1)) >> 2) +
               (rw2 * (y - 1) * (y - 1)) - (rw2 * rh2);
    while (y >= 0) {
        graphics_draw_pixel(x0 + x, y0 + y, color);
        graphics_draw_pixel(x0 - x, y0 + y, color);
        graphics_draw_pixel(x0 + x, y0 - y, color);
        graphics_draw_pixel(x0 - x, y0 - y, color);
        y--;
        if (decision > 0) {
            decision += rw2 - (twoRw2 * y);
        } else {
            decision += rw2 + (twoRh2 * x) - (twoRw2 * y);
            x++;
        }
    }
}

/**************************************************************************/
/*!
   @brief    Draw an ellipse with filled colour
    @param    x0   Center-point x coordinate
    @param    y0   Center-point y coordinate
    @param    rw   Horizontal radius of ellipse
    @param    rh   Vertical radius of ellipse
    @param    color Color to draw with
*/
/**************************************************************************/
void adafruit_gfx_fillEllipse(int16_t x0, int16_t y0, int16_t rw, int16_t rh,
                              color_t color) {
    // Bresenham's ellipse algorithm
    int16_t x = 0, y = rh;
    int32_t rw2 = rw * rw, rh2 = rh * rh;
    int32_t twoRw2 = 2 * rw2, twoRh2 = 2 * rh2;

    int32_t decision = rh2 - (rw2 * rh) + (rw2 / 4);

    // region 1
    while ((twoRh2 * x) < (twoRw2 * y)) {
        x++;
        if (decision < 0) {
            decision += rh2 + (twoRh2 * x);
        } else {
            decision += rh2 + (twoRh2 * x) - (twoRw2 * y);
            adafruit_gfx_drawFastHLine(x0 - (x - 1), y0 + y, 2 * (x - 1) + 1,
                                       color);
            adafruit_gfx_drawFastHLine(x0 - (x - 1), y0 - y, 2 * (x - 1) + 1,
                                       color);
            y--;
        }
    }

    // region 2
    decision = ((rh2 * (2 * x + 1) * (2 * x + 1)) >> 2) +
               (rw2 * (y - 1) * (y - 1)) - (rw2 * rh2);
    while (y >= 0) {
        adafruit_gfx_drawFastHLine(x0 - x, y0 + y, 2 * x + 1, color);
        adafruit_gfx_drawFastHLine(x0 - x, y0 - y, 2 * x + 1, color);

        y--;
        if (decision > 0) {
            decision += rw2 - (twoRw2 * y);
        } else {
            decision += rw2 + (twoRh2 * x) - (twoRw2 * y);
            x++;
        }
    }
}

/**************************************************************************/
/*!
   @brief    Draw a circle outline
    @param    x0   Center-point x coordinate
    @param    y0   Center-point y coordinate
    @param    r   Radius of circle
    @param    color Color to draw with
*/
/**************************************************************************/
void adafruit_gfx_drawCircle(int16_t x0, int16_t y0, int16_t r, color_t color) {
#if defined(ESP8266)
    optimistic_yield(10000);
#endif
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;

    graphics_draw_pixel(x0, y0 + r, color);
    graphics_draw_pixel(x0, y0 - r, color);
    graphics_draw_pixel(x0 + r, y0, color);
    graphics_draw_pixel(x0 - r, y0, color);

    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        graphics_draw_pixel(x0 + x, y0 + y, color);
        graphics_draw_pixel(x0 - x, y0 + y, color);
        graphics_draw_pixel(x0 + x, y0 - y, color);
        graphics_draw_pixel(x0 - x, y0 - y, color);
        graphics_draw_pixel(x0 + y, y0 + x, color);
        graphics_draw_pixel(x0 - y, y0 + x, color);
        graphics_draw_pixel(x0 + y, y0 - x, color);
        graphics_draw_pixel(x0 - y, y0 - x, color);
    }
}

/**************************************************************************/
/*!
    @brief    Quarter-circle drawer, used to do circles and roundrects
    @param    x0   Center-point x coordinate
    @param    y0   Center-point y coordinate
    @param    r   Radius of circle
    @param    cornername  Mask bit #1, #2, #4, and #8 to indicate which quarters
              of the circle we're doing
    @param    color Color to draw with
*/
/**************************************************************************/
static void adafruit_gfx_drawCircleHelper(int16_t x0, int16_t y0, int16_t r,
                                          uint8_t cornername, color_t color) {
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;

    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;
        if (cornername & 0x4) {
            graphics_draw_pixel(x0 + x, y0 + y, color);
            graphics_draw_pixel(x0 + y, y0 + x, color);
        }
        if (cornername & 0x2) {
            graphics_draw_pixel(x0 + x, y0 - y, color);
            graphics_draw_pixel(x0 + y, y0 - x, color);
        }
        if (cornername & 0x8) {
            graphics_draw_pixel(x0 - y, y0 + x, color);
            graphics_draw_pixel(x0 - x, y0 + y, color);
        }
        if (cornername & 0x1) {
            graphics_draw_pixel(x0 - y, y0 - x, color);
            graphics_draw_pixel(x0 - x, y0 - y, color);
        }
    }
}

/**************************************************************************/
/*!
    @brief  Half-circle drawer with fill, used for circles and roundrects
    @param  x0       Center-point x coordinate
    @param  y0       Center-point y coordinate
    @param  r        Radius of circle
    @param  corners  Mask bits indicating which sides of the circle we are
                     doing, left (1) and/or right (2)
    @param  delta    Offset from center-point, used for round-rects
    @param  color    Color to fill with
*/
/**************************************************************************/
void adafruit_gfx_fillCircleHelper(int16_t x0, int16_t y0, int16_t r,
                                   uint8_t corners, int16_t delta,
                                   color_t color) {

    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;
    int16_t px = x;
    int16_t py = y;

    delta++; // Avoid some +1's in the loop

    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;
        // These checks avoid double-drawing certain lines, important
        // for the SSD1306 library which has an INVERT drawing mode.
        if (x < (y + 1)) {
            if (corners & 1)
                adafruit_gfx_drawFastVLine(x0 + x, y0 - y, 2 * y + delta,
                                           color);
            if (corners & 2)
                adafruit_gfx_drawFastVLine(x0 - x, y0 - y, 2 * y + delta,
                                           color);
        }
        if (y != py) {
            if (corners & 1)
                adafruit_gfx_drawFastVLine(x0 + py, y0 - px, 2 * px + delta,
                                           color);
            if (corners & 2)
                adafruit_gfx_drawFastVLine(x0 - py, y0 - px, 2 * px + delta,
                                           color);
            py = y;
        }
        px = x;
    }
}

/**************************************************************************/
/*!
   @brief    Draw a circle with filled color
    @param    x0   Center-point x coordinate
    @param    y0   Center-point y coordinate
    @param    r   Radius of circle
    @param    color Color to fill with
*/
/**************************************************************************/
void adafruit_gfx_fillCircle(int16_t x0, int16_t y0, int16_t r, color_t color) {
    adafruit_gfx_drawFastVLine(x0, y0 - r, 2 * r + 1, color);
    adafruit_gfx_fillCircleHelper(x0, y0, r, 3, 0, color);
}

/**************************************************************************/
/*!
    @brief  Quarter-circle drawer with fill.
    @param  x0       Center-point x coordinate
    @param  y0       Center-point y coordinate
    @param  r        Radius of circle
    @param  corners  Mask bits 0x1, 0x2, 0x4, and 0x8 to indicate which quarters
                     of the circle to fill: 0x1 = top-left, 0x2 = top-right,
                     0x4 = bottom-right, 0x8 = bottom-left
    @param  color    Color to fill with
*/
/**************************************************************************/
void adafruit_gfx_fillQuarterCircle(int16_t x0, int16_t y0, int16_t r,
                                    uint8_t corners, color_t color) {

    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;
    int16_t px = x;
    int16_t py = y;

    if (corners & (0x2 | 0x1))
        adafruit_gfx_drawFastVLine(x0, y0 - r, r + 1, color);
    if (corners & (0x8 | 0x4))
        adafruit_gfx_drawFastVLine(x0, y0, r + 1, color);

    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        if (x < (y + 1)) {
            if (corners & 0x4)
                adafruit_gfx_drawFastVLine(x0 + x, y0, y + 1, color);
            if (corners & 0x2)
                adafruit_gfx_drawFastVLine(x0 + x, y0 - y, y + 1, color);
            if (corners & 0x8)
                adafruit_gfx_drawFastVLine(x0 - x, y0, y + 1, color);
            if (corners & 0x1)
                adafruit_gfx_drawFastVLine(x0 - x, y0 - y, y + 1, color);
        }
        if (y != py) {
            if (corners & 0x4)
                adafruit_gfx_drawFastVLine(x0 + py, y0, px + 1, color);
            if (corners & 0x2)
                adafruit_gfx_drawFastVLine(x0 + py, y0 - px, px + 1, color);
            if (corners & 0x8)
                adafruit_gfx_drawFastVLine(x0 - py, y0, px + 1, color);
            if (corners & 0x1)
                adafruit_gfx_drawFastVLine(x0 - py, y0 - px, px + 1, color);
            py = y;
        }
        px = x;
    }
}

/**************************************************************************/
/*!
   @brief   Draw a rectangle with no fill color
    @param    x   Top left corner x coordinate
    @param    y   Top left corner y coordinate
    @param    w   Width in pixels
    @param    h   Height in pixels
    @param    color Color to draw with
*/
/**************************************************************************/
void adafruit_gfx_drawRect(int16_t x, int16_t y, int16_t w, int16_t h,
                           color_t color) {
    adafruit_gfx_drawFastHLine(x, y, w, color);
    adafruit_gfx_drawFastHLine(x, y + h - 1, w, color);
    adafruit_gfx_drawFastVLine(x, y, h, color);
    adafruit_gfx_drawFastVLine(x + w - 1, y, h, color);
}

/**************************************************************************/
/*!
   @brief   Draw a rounded rectangle with no fill color
    @param    x   Top left corner x coordinate
    @param    y   Top left corner y coordinate
    @param    w   Width in pixels
    @param    h   Height in pixels
    @param    r   Radius of corner rounding
    @param    color Color to draw with
*/
/**************************************************************************/
void adafruit_gfx_drawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h,
                                int16_t r, color_t color) {
    int16_t max_radius = ((w < h) ? w : h) / 2; // 1/2 minor axis
    if (r > max_radius)
        r = max_radius;
    // smarter version
    adafruit_gfx_drawFastHLine(x + r, y, w - 2 * r, color);         // Top
    adafruit_gfx_drawFastHLine(x + r, y + h - 1, w - 2 * r, color); // Bottom
    adafruit_gfx_drawFastVLine(x, y + r, h - 2 * r, color);         // Left
    adafruit_gfx_drawFastVLine(x + w - 1, y + r, h - 2 * r, color); // Right
    // draw four corners
    adafruit_gfx_drawCircleHelper(x + r, y + r, r, 1, color);
    adafruit_gfx_drawCircleHelper(x + w - r - 1, y + r, r, 2, color);
    adafruit_gfx_drawCircleHelper(x + w - r - 1, y + h - r - 1, r, 4, color);
    adafruit_gfx_drawCircleHelper(x + r, y + h - r - 1, r, 8, color);
}

/**************************************************************************/
/*!
   @brief   Draw a rounded rectangle with fill color
    @param    x   Top left corner x coordinate
    @param    y   Top left corner y coordinate
    @param    w   Width in pixels
    @param    h   Height in pixels
    @param    r   Radius of corner rounding
    @param    color Color to draw/fill with
*/
/**************************************************************************/
void adafruit_gfx_fillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h,
                                int16_t r, color_t color) {
    int16_t max_radius = ((w < h) ? w : h) / 2; // 1/2 minor axis
    if (r > max_radius)
        r = max_radius;
    // smarter version
    adafruit_gfx_drawFillRect(x + r, y, w - 2 * r, h, color);
    // draw four corners (helpers open their own transaction)
    adafruit_gfx_fillCircleHelper(x + w - r - 1, y + r, r, 1, h - 2 * r - 1,
                                  color);
    adafruit_gfx_fillCircleHelper(x + r, y + r, r, 2, h - 2 * r - 1, color);
}

/**************************************************************************/
/*!
   @brief     Draw a rotated rectangle
    @param    cenX  x coordinate of center of rectangle.
                    For even width rectangles, this point
                    represents the pixel to the left of
                    true center.
    @param    cenY  y coordinate of center of rectangle.
                    For even height rectangles, this point
                    represents the pixel above
                    true center.
    @param    w  width of rectangle
    @param    h  height of rectangle
    @param    angleDeg  angle of rotation of rectangle
    @param    color Color to fill/draw with
*/
/**************************************************************************/
void adafruit_gfx_drawRotatedRect(int16_t cenX, int16_t cenY, int16_t w,
                                  int16_t h, int16_t angleDeg, color_t color) {

    if (w < 1 || h < 1)
        return; // We don't draw zero dimensioned objects

    int16_t W = w - 1;
    int16_t H = h - 1;

    int16_t halfW = (W / 2); // Midpoint should always be integer
    int16_t halfH = (H / 2); // Midpoint should always be integer

    int16_t x0 = W - halfW; // bottom-right
    int16_t y0 = H - halfH; // bottom-right
    int16_t x1 = -halfW;    // bottom-left
    int16_t y1 = H - halfH; // bottom-left
    int16_t x2 = -halfW;    // top-left
    int16_t y2 = -halfH;    // top-left
    int16_t x3 = W - halfW; // top-right
    int16_t y3 = -halfH;    // top-right

    adafruit_gfx_rotatePoint(&x0, &y0, angleDeg);
    adafruit_gfx_rotatePoint(&x1, &y1, angleDeg);
    adafruit_gfx_rotatePoint(&x2, &y2, angleDeg);
    adafruit_gfx_rotatePoint(&x3, &y3, angleDeg);

    x0 += cenX;
    x1 += cenX;
    x2 += cenX;
    x3 += cenX;

    y0 += cenY;
    y1 += cenY;
    y2 += cenY;
    y3 += cenY;

    adafruit_gfx_drawLine(x0, y0, x1, y1, color); // bottom right to bottom left
    adafruit_gfx_drawLine(x1, y1, x2, y2, color); // bottom left to top left
    adafruit_gfx_drawLine(x2, y2, x3, y3, color); // top left to top right
    adafruit_gfx_drawLine(x3, y3, x0, y0, color); // top right to bottom right
}

/**************************************************************************/
/*!
   @brief     Draw a filled rotated rectangle
    @param    cenX  x coordinate of center of rectangle.
                    For even width rectangles, this point
                    represents the pixel to the left of
                    true center.
    @param    cenY  y coordinate of center of rectangle.
                    For even height rectangles, this point
                    represents the pixel above
                    true center.
    @param    w  width of rectangle
    @param    h  height of rectangle
    @param    angleDeg  angle of rotation of rectangle
    @param    color Color to fill/draw with
*/
/**************************************************************************/
void adafruit_gfx_fillRotatedRect(int16_t cenX, int16_t cenY, int16_t w,
                                  int16_t h, int16_t angleDeg, color_t color) {

    if (w < 1 || h < 1)
        return; // We don't draw zero dimensioned objects

    int16_t W = w - 1;
    int16_t H = h - 1;

    int16_t halfW = (W / 2); // Midpoint should always be integer
    int16_t halfH = (H / 2); // Midpoint should always be integer

    int16_t x0 = W - halfW; // bottom-right
    int16_t y0 = H - halfH; // bottom-right
    int16_t x1 = -halfW;    // bottom-left
    int16_t y1 = H - halfH; // bottom-left
    int16_t x2 = -halfW;    // top-left
    int16_t y2 = -halfH;    // top-left
    int16_t x3 = W - halfW; // top-right
    int16_t y3 = -halfH;    // top-right

    adafruit_gfx_rotatePoint(&x0, &y0, angleDeg);
    adafruit_gfx_rotatePoint(&x1, &y1, angleDeg);
    adafruit_gfx_rotatePoint(&x2, &y2, angleDeg);
    adafruit_gfx_rotatePoint(&x3, &y3, angleDeg);

    x0 += cenX;
    x1 += cenX;
    x2 += cenX;
    x3 += cenX;

    y0 += cenY;
    y1 += cenY;
    y2 += cenY;
    y3 += cenY;

    adafruit_gfx_fillTriangle(x0, y0, x1, y1, x2, y2, color);
    adafruit_gfx_fillTriangle(x2, y2, x3, y3, x0, y0, color);
}

/**************************************************************************/
/*!
   @brief     Rotate a point in standard position
    @param    x0  x coordinate of point to rotate. This is passed by reference
                  and updated upon return
    @param    y0  y coordinate of point to rotate. This is passed by reference
                  and updated upon return
    @param    angleDeg  angle to rotate the point by (degrees)
*/
/**************************************************************************/
void adafruit_gfx_rotatePoint(int16_t *x0, int16_t *y0, int16_t angleDeg) {
    float angleRad = (float)angleDeg / DEGREES_TO_RADIANS;
    float s = sin(angleRad);
    float c = cos(angleRad);

    float x = *x0;
    float y = *y0;

    // Rotate point
    *x0 = (int16_t)((x * c) - (y * s));
    *y0 = (int16_t)((x * s) + (y * c));
}

/**************************************************************************/
/*!
   @brief   Draw a triangle with no fill color
    @param    x0  Vertex #0 x coordinate
    @param    y0  Vertex #0 y coordinate
    @param    x1  Vertex #1 x coordinate
    @param    y1  Vertex #1 y coordinate
    @param    x2  Vertex #2 x coordinate
    @param    y2  Vertex #2 y coordinate
    @param    color Color to draw with
*/
/**************************************************************************/
void adafruit_gfx_drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
                               int16_t x2, int16_t y2, color_t color) {
    adafruit_gfx_drawLine(x0, y0, x1, y1, color);
    adafruit_gfx_drawLine(x1, y1, x2, y2, color);
    adafruit_gfx_drawLine(x2, y2, x0, y0, color);
}

/**************************************************************************/
/*!
   @brief     Draw a triangle with color-fill
    @param    x0  Vertex #0 x coordinate
    @param    y0  Vertex #0 y coordinate
    @param    x1  Vertex #1 x coordinate
    @param    y1  Vertex #1 y coordinate
    @param    x2  Vertex #2 x coordinate
    @param    y2  Vertex #2 y coordinate
    @param    color Color to fill/draw with
*/
/**************************************************************************/
void adafruit_gfx_fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
                               int16_t x2, int16_t y2, color_t color) {

    int16_t a, b, y, last;

    // Sort coordinates by Y order (y2 >= y1 >= y0)
    if (y0 > y1) {
        int16_t tmp = y0;
        y0 = y1;
        y1 = tmp;
        tmp = x0;
        x0 = x1;
        x1 = tmp;
    }
    if (y1 > y2) {
        int16_t tmp = y1;
        y1 = y2;
        y2 = tmp;
        tmp = x1;
        x1 = x2;
        x2 = tmp;
    }
    if (y0 > y1) {
        int16_t tmp = y0;
        y0 = y1;
        y1 = tmp;
        tmp = x0;
        x0 = x1;
        x1 = tmp;
    }

    if (y0 == y2) { // Handle awkward all-on-same-line case as its own thing
        a = b = x0;
        if (x1 < a)
            a = x1;
        else if (x1 > b)
            b = x1;
        if (x2 < a)
            a = x2;
        else if (x2 > b)
            b = x2;
        adafruit_gfx_drawFastHLine(a, y0, b - a + 1, color);
        return;
    }

    int16_t dx01 = x1 - x0, dy01 = y1 - y0, dx02 = x2 - x0, dy02 = y2 - y0,
            dx12 = x2 - x1, dy12 = y2 - y1;
    int32_t sa = 0, sb = 0;

    // For upper part of triangle, find scanline crossings for segments
    // 0-1 and 0-2.  If y1=y2 (flat-bottomed triangle), the scanline y1
    // is included here (and second loop will be skipped, avoiding a /0
    // error there), otherwise scanline y1 is skipped here and handled
    // in the second loop...which also avoids a /0 error here if y0=y1
    // (flat-topped triangle).
    if (y1 == y2)
        last = y1; // Include y1 scanline
    else
        last = y1 - 1; // Skip it

    for (y = y0; y <= last; y++) {
        a = x0 + sa / dy01;
        b = x0 + sb / dy02;
        sa += dx01;
        sb += dx02;
        /* longhand:
        a = x0 + (x1 - x0) * (y - y0) / (y1 - y0);
        b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
        */
        if (a > b) {
            int16_t tmp = a;
            a = b;
            b = tmp;
        }
        adafruit_gfx_drawFastHLine(a, y, b - a + 1, color);
    }

    // For lower part of triangle, find scanline crossings for segments
    // 0-2 and 1-2.  This loop is skipped if y1=y2.
    sa = (int32_t)dx12 * (y - y1);
    sb = (int32_t)dx02 * (y - y0);
    for (; y <= y2; y++) {
        a = x1 + sa / dy12;
        b = x0 + sb / dy02;
        sa += dx12;
        sb += dx02;
        /* longhand:
        a = x1 + (x2 - x1) * (y - y1) / (y2 - y1);
        b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
        */
        if (a > b) {
            int16_t tmp = a;
            a = b;
            b = tmp;
        }
        adafruit_gfx_drawFastHLine(a, y, b - a + 1, color);
    }
}

// BITMAP / XBITMAP / GRAYSCALE / RGB BITMAP FUNCTIONS ---------------------

/**************************************************************************/
/*!
   @brief      Draw a RAM-resident 1-bit image at the specified (x,y) position,
   using the specified foreground color (unset bits are transparent).
    @param    x   Top left corner x coordinate
    @param    y   Top left corner y coordinate
    @param    bitmap  byte array with monochrome bitmap
    @param    w   Width of bitmap in pixels
    @param    h   Height of bitmap in pixels
    @param    color Color to draw with
*/
/**************************************************************************/
void adafruit_gfx_drawBitmap(int16_t x, int16_t y, const uint8_t *bitmap,
                             int16_t w, int16_t h, color_t color) {

    int16_t byteWidth = (w + 7) / 8; // Bitmap scanline pad = whole byte
    uint8_t b = 0;

    for (int16_t j = 0; j < h; j++, y++) {
        for (int16_t i = 0; i < w; i++) {
            if (i & 7)
                b <<= 1;
            else
                b = bitmap[j * byteWidth + i / 8];
            if (b & 0x80)
                graphics_draw_pixel(x + i, y, color);
        }
    }
}

// Draw a character
/**************************************************************************/
/*!
   @brief   Draw a single character
    @param    x   Bottom left corner x coordinate
    @param    y   Bottom left corner y coordinate
    @param    c   The 8-bit font-indexed character (likely ascii)
    @param    color Color to draw character with
    @param    bg Color to fill background with (if same as color,
   no background)
    @param    size_x  Font magnification level in X-axis, 1 is 'original' size
    @param    size_y  Font magnification level in Y-axis, 1 is 'original' size
*/
/**************************************************************************/
void adafruit_gfx_drawChar(int16_t x, int16_t y, unsigned char c, color_t color,
                           uint8_t size_x, uint8_t size_y) {
    if ((x >= SCREEN_SIZE_X) ||       // Clip right
        (y >= SCREEN_SIZE_Y) ||       // Clip bottom
        ((x + 6 * size_x - 1) < 0) || // Clip left
        ((y + 8 * size_y - 1) < 0))   // Clip top
        return;

    int8_t rp = 0; // Count sequential FG pixels in column
    for (int8_t i = 0; i < 5; i++, rp = 0) { // Char bitmap = 5 columns
        uint8_t line = pgm_read_byte(&font[c * 5 + i]);
        for (int8_t j = 0; j < 8; j++, line >>= 1) {
            if (line & 1) {
                rp++;
                if (!(1 & (line >> 1)) ||
                    j == 7) { // write sequential pixels with writeFillRect
                    if (rp == 1 && size_x == 1 && size_y == 1) {
                        graphics_draw_pixel(x + i, y + j, color);
                    } else if (rp > 0) {
                        adafruit_gfx_drawFillRect(x + i * size_x,
                                                  y + (j - (rp - 1)) * size_y,
                                                  size_x, size_y * rp, color);
                    }
                    rp = 0;
                }
            }
        }
    }
}
/**************************************************************************/
/*!
    @brief  Print one byte/character of data, used to support print()
    @param  c  The 8-bit ascii character to write
*/
/**************************************************************************/
void adafruit_gfx_write(char c) {
    if (c == '\n') {                // Newline?
        cursor_x = 0;               // Reset x to zero,
        cursor_y += textsize_y * 8; // advance y one line
    } else if (c != '\r') {         // Ignore carriage returns
        if (wrap &&
            ((cursor_x + textsize_x * 6) > SCREEN_SIZE_X)) { // Off right?
            cursor_x = 0;                                    // Reset x to zero,
            cursor_y += textsize_y * 8; // advance y one line
        }
        adafruit_gfx_drawChar(cursor_x, cursor_y, c, textcolor, textsize_x,
                              textsize_y);
        cursor_x += textsize_x * 6; // Advance x one char
    }
}

void adafruit_gfx_print(const char *str) {
    size_t i = 0;
    while (str[i] != 0) {
        adafruit_gfx_write(str[i]);
        i++;
    }
}

/**************************************************************************/
/*!
    @brief   Set text 'magnification' size. Each increase in s makes 1 pixel
   that much bigger.
    @param  s_x  Desired text width magnification level in X-axis. 1 is default
    @param  s_y  Desired text width magnification level in Y-axis. 1 is default
*/
/**************************************************************************/
void adafruit_gfx_setTextSize(uint8_t s_x, uint8_t s_y) {
    textsize_x = (s_x > 0) ? s_x : 1;
    textsize_y = (s_y > 0) ? s_y : 1;
}

/**************************************************************************/
/*!
    @brief  Helper to determine size of a character with current font/size.
            Broke this out as it's used by both the PROGMEM- and RAM-resident
            getTextBounds() functions.
    @param  c     The ASCII character in question
    @param  x     Pointer to x location of character. Value is modified by
                  this function to advance to next character.
    @param  y     Pointer to y location of character. Value is modified by
                  this function to advance to next character.
    @param  minx  Pointer to minimum X coordinate, passed in to AND returned
                  by this function -- this is used to incrementally build a
                  bounding rectangle for a string.
    @param  miny  Pointer to minimum Y coord, passed in AND returned.
    @param  maxx  Pointer to maximum X coord, passed in AND returned.
    @param  maxy  Pointer to maximum Y coord, passed in AND returned.
*/
/**************************************************************************/
void adafruit_gfx_charBounds(unsigned char c, int16_t *x, int16_t *y,
                             int16_t *minx, int16_t *miny, int16_t *maxx,
                             int16_t *maxy) {
    if (c == '\n') {          // Newline?
        *x = 0;               // Reset x to zero,
        *y += textsize_y * 8; // advance y one line
        // min/max x/y unchanged -- that waits for next 'normal' character
    } else if (c != '\r') { // Normal char; ignore carriage returns
        if (wrap && ((*x + textsize_x * 6) > SCREEN_SIZE_X)) { // Off right?
            *x = 0;               // Reset x to zero,
            *y += textsize_y * 8; // advance y one line
        }
        int x2 = *x + textsize_x * 6 - 1, // Lower-right pixel of char
            y2 = *y + textsize_y * 8 - 1;
        if (x2 > *maxx)
            *maxx = x2; // Track max x, y
        if (y2 > *maxy)
            *maxy = y2;
        if (*x < *minx)
            *minx = *x; // Track min x, y
        if (*y < *miny)
            *miny = *y;
        *x += textsize_x * 6; // Advance x one char
    }
}

/**************************************************************************/
/*!
    @brief  Helper to determine size of a string with current font/size.
            Pass string and a cursor position, returns UL corner and W,H.
    @param  str  The ASCII string to measure
    @param  x    The current cursor X
    @param  y    The current cursor Y
    @param  x1   The boundary X coordinate, returned by function
    @param  y1   The boundary Y coordinate, returned by function
    @param  w    The boundary width, returned by function
    @param  h    The boundary height, returned by function
*/
/**************************************************************************/
void adafruit_gfx_getTextBounds(const char *str, int16_t x, int16_t y,
                                int16_t *x1, int16_t *y1, uint16_t *w,
                                uint16_t *h) {

    uint8_t c; // Current character
    int16_t minx = 0x7FFF, miny = 0x7FFF, maxx = -1, maxy = -1; // Bound rect
    // Bound rect is intentionally initialized inverted, so 1st char sets it

    *x1 = x; // Initial position is value passed in
    *y1 = y;
    *w = *h = 0; // Initial size is zero

    while ((c = *str++)) {
        // charBounds() modifies x/y to advance for each character,
        // and min/max x/y are updated to incrementally build bounding rect.
        adafruit_gfx_charBounds(c, &x, &y, &minx, &miny, &maxx, &maxy);
    }

    if (maxx >= minx) {       // If legit string bounds were found...
        *x1 = minx;           // Update x1 to least X coord,
        *w = maxx - minx + 1; // And w to bound rect width
    }
    if (maxy >= miny) { // Same for height
        *y1 = miny;
        *h = maxy - miny + 1;
    }
}

uint8_t adafruit_gfx_getTextSizeX() { return textsize_x; }

uint8_t adafruit_gfx_getTextSizeY() { return textsize_y; }

/**********************************************************************/
/*!
  @brief  Set text cursor location
  @param  x    X coordinate in pixels
  @param  y    Y coordinate in pixels
*/
/**********************************************************************/
void adafruit_gfx_setCursor(int16_t x, int16_t y) {
    cursor_x = x;
    cursor_y = y;
}

/**********************************************************************/
/*!
  @brief   Set text font color with transparant background
  @param   c   Color to draw text with
  @note    For 'transparent' background, background and foreground
           are set to same color rather than using a separate flag.
*/
/**********************************************************************/
void adafruit_gfx_setTextColor(color_t c) { textcolor = c; }

/**********************************************************************/
/*!
  @brief  Get text color
  @returns text color
*/
/**********************************************************************/
color_t adafruit_gfx_getTextColor() { return textcolor; }

/**********************************************************************/
/*!
@brief  Set whether text that is too long for the screen width should
        automatically wrap around to the next line (else clip right).
@param  w  true for wrapping, false for clipping
*/
/**********************************************************************/
void adafruit_gfx_setTextWrap(bool w) { wrap = w; }

/************************************************************************/
/*!
  @brief      Get text cursor Y location
  @returns    Y coordinate in pixels
*/
/************************************************************************/
int16_t adafruit_gfx_getCursorX(void) { return cursor_x; }

/************************************************************************/
/*!
  @brief      Get text cursor Y location
  @returns    Y coordinate in pixels
*/
/************************************************************************/
int16_t adafruit_gfx_getCursorY(void) { return cursor_y; };
