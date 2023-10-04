#include <stdio.h>
#include <stdlib.h>

#include "pd_api.h"

static int update(void* userdata);

#ifdef _WINDLL
__declspec(dllexport)
#endif

typedef struct Point {int32_t x; int32_t y;} point;
typedef struct Rect {int32_t x; int32_t y; int32_t w; int32_t h;} rect;
static const int32_t ps = 50;
static rect pac = { 0, 0, ps, ps };
static rect hbox = { 0.2 * ps, 0.1 * ps, 0.6 * ps, 0.8 * ps };
static rect vbox = { 0.1 * ps, 0.2 * ps, 0.8 * ps, 0.6 * ps };
static int32_t mouth_step = 3;
static int32_t mouth_angle = 45;
static uint8_t direction = 1;
static PlaydateAPI* papi; // global crimes.


int eventHandler(PlaydateAPI* pd, PDSystemEvent event, uint32_t arg)
{
    (void)arg; // arg is currently only used for event = kEventKeyPressed
    if ( event == kEventInit ) {
        pd->system->setUpdateCallback(update, pd);
    }
    return 0;
}

typedef uint8_t uint8_t;
typedef int32_t int32_t;
void draw_vert_line(uint8_t* debug_bitmap, int32_t x, int32_t y, int32_t extent) {
    if (extent <= 0 || x >= 400 || x < 0 || y >= 240 || y + extent < 0)
        return;
    if (y + extent >= 240)
        extent = 240 - y;
    if (y < 0) {
        extent += y;
        y = 0;
    }
    for (int i = 0; i < extent; i++) {
        uint8_t* bitmap_row = (uint8_t*)debug_bitmap + ((y + i) * 52);
        bitmap_row[x / 8] |= 1 << (7 - (x % 8));
    }
}

void draw_horiz_line(uint8_t* debug_bitmap, int32_t x, int32_t y, int32_t extent) {
    if (extent <= 0 || y >= 240 || y < 0 || x >= 400 || x + extent < 0)
        return;
    if (x + extent >= 400)
        extent = 400 - x;
    if (x < 0) {
        extent += x;
        x = 0;
    }
    uint8_t* bitmap_row = (uint8_t*)debug_bitmap + (y * 52);
    for (int i = 0; i < extent; i++) {
        bitmap_row[(x + i) / 8] |= 1 << (7 - ((x + i) % 8));
    }
}

// Draws a rectangle directly onto the bitmap data for the debug bitmap.
void draw_rect(uint8_t* debug_bitmap, int32_t x, int32_t y, int32_t width, int32_t height) {
    draw_horiz_line(debug_bitmap, x, y, width);
    draw_horiz_line(debug_bitmap, x, y + height - 1, width);
    draw_vert_line(debug_bitmap, x, y, height);
    draw_vert_line(debug_bitmap, x + width - 1, y, height);
}

static void debug_draw(PlaydateAPI* pd) {
    uint8_t* debug_bitmap;
    LCDBitmap* simulator = pd->graphics->getDebugBitmap();
    if (simulator) {
        pd->graphics->getBitmapData(simulator, NULL, NULL, NULL, NULL, &debug_bitmap);
        if (direction == 1 || direction == 3) {
            draw_rect(debug_bitmap, pac.x + hbox.x, pac.y + hbox.y, hbox.w, hbox.h);
        } else {
            draw_rect(debug_bitmap, pac.x + vbox.x, pac.y + vbox.y, vbox.w, vbox.h);
        }
    }
}

void update_pacman(PlaydateAPI* pd) {
    pd->graphics->fillEllipse(
        pac.x, pac.y, pac.w, pac.h,
        90 * direction + mouth_angle, 90 * direction - mouth_angle, 0);
}

void buttons(PlaydateAPI* pd) {
    const uint8_t speed = 2;
    PDButtons current, pushed, released;
    pd->system->getButtonState(&current, &pushed, &released);
    if (current & kButtonLeft) {
        direction = 3;
        pac.x -= speed;
    } else if (current & kButtonRight) {
        direction = 1;
        pac.x += speed;
    } else if (current & kButtonUp) {
        direction = 0;
        pac.y -= speed;
    } else if (current & kButtonDown) {
        direction = 2;
        pac.y += speed;
    }
    // if (pac.x < 0) pac.x = 0;
    // if (pac.x > 400 - ps) pac.x = 400 - ps;
    // if (pac.y < 0) pac.y = 0;
    // if (pac.y > 240 - ps) pac.y = 240 - ps;

    if (current & (kButtonRight|kButtonLeft|kButtonUp|kButtonDown)) {
        mouth_angle += mouth_step;
        if (mouth_angle >= 45) {
            mouth_angle = 45;
            mouth_step = -3;
        } else if (mouth_angle <= 0) {
            mouth_angle = 0;
            mouth_step = 3;
        }
    }
}

static int update(void* userdata)
{
    PlaydateAPI* pd = userdata;
    papi = userdata;
    pd->graphics->clear(1);
    buttons(pd);
    update_pacman(pd);
    debug_draw(pd);

    pd->system->drawFPS(385, 228);
    return 1;
}
