
#include "wiimote_pointer.h"
#include "app_state.h"
#include "vga16_graphics_v2.h"

/*
    This file is responsible for drawing the Wii pointer.

    BLOB 1
    x = 1000 - Far left
    x = 0   - Far right
    x = Anything > 1000, < 0: Out of range

    y = 0   - Top
    y = 760 - Bottom
    y = Anything < 0, > 760 - Out of range

    BLOB 2
    Do we even need this? 
    
    Reading into it more, the actual Wii has 2 IR blasters so it can identify the Wii bar from other IR sources 
    in a noisy environment (these 2 points are collinear no matter what, outide noise is unlikely to retain this property). 
    This is also why the wiimote can pick up on up to 4 blobs - it doesn't immediately know what is noise, so it assumes 
    2 will be noise, 2 will be the actual bar, and does work to figure out what to do. The lab vseems to be pretty 
    clean of IR noise and we can choose where we demo lmao. 

*/

volatile short pointer_x = 0;
volatile short pointer_y = 0;
volatile int wiimote_pt_color = WHITE;

int remap_x (int curr_x) {
    return (short)(((1000 - (int)curr_x) * 640) / 1000);
}

int remap_y (int curr_y) {
    // That should be "* 480", but 640 works better??????
    return (short)(((int)curr_y * 640) / 760);
}

void draw_pointer() {

    // To erase old pointer
    fillCircle(pointer_x, pointer_y, 6, bg_color) ;

    short x_unfilt = remap_x(blob_1.x);
    short y_unfilt = remap_y(blob_1.y);

    pointer_x = x_unfilt;
    pointer_y = y_unfilt;

    const short R = 6;

    short dx = pointer_x;
    short dy = pointer_y;

    // If pointer is completely offscreen, do not draw it or erase it
    if (dx < -R || dx > 639 + R || dy < -R || dy > 479 + R) {
        return;
    }

    // Clamp draw coordinates
    if (dx < R) dx = R;
    if (dx > 639 - R) dx = 639 - R;
    if (dy < R) dy = R;
    if (dy > 479 - R) dy = 479 - R;

    fillCircle(pointer_x, pointer_y, 6, wiimote_pt_color) ;

}