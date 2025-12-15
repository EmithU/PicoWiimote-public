
#ifndef GAME_PRIMITIVES_H
#define GAME_PRIMITIVES_H

#include <pico/stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include "controls.h"
#include "vga16_graphics_v2.h"
#include <math.h>

// Draggable objects. Optional bounds.
struct wiimote_drag_obj {

    // Shape present (lets us preallocate this array and some headache)
    bool present;

    // Shape type (0=circle, 1=rect)
    int shape;

    // All shapes
    short x;
    short y;
    char color;
    bool movable;
    bool bounded;
    int bound_dir;

    // Relevant if circle
    short r;

    // Relevant if rectangle
    short w;
    short h;

    // If we need to deal with bounds somehow. Either bounded along x or y.
    // If bounded on x, cannot move along y, and vice versa. Good for sliders.
    short min_bound;
    short max_bound;

};

struct wiimote_click_obj {

    // Shape present (lets us preallocate this array and save some headache)
    bool present;

    // Shape type (0=circle, 1=rect)
    int shape;

    // All shapes
    short x;
    short y;
    char color;
    bool clickable;
    bool toggleHeldState;
    bool heldState;
    bool clicked;

    // Relevant if circle
    short r;

    // Relevant if rectangle
    short w;
    short h;

};

typedef enum {
    SHAPE_CIRCLE,
    SHAPE_RECTANGLE
} Shapes;

typedef enum {
    BOUND_X,
    BOUND_Y
} BoundDir;

typedef enum {
    CLICKTYPE_TAP,
    CLICKTYPE_HOLD
} ClickType;

// Going to assume no game will ever have more than 50 objects, change this if this is not the case
extern struct wiimote_drag_obj draggable_objs[25];
extern struct wiimote_click_obj clickable_objs[25];

int create_circle_movable_unbounded(short x0, short y0, short r, char color);
int create_rect_movable_unbounded(short x, short y, short w, short h, char color);
int create_circle_movable_bounded(short x0, short y0, short r, char color, int boundDir, short min, short max);
int create_rect_movable_bounded(short x, short y, short w, short h, char color, int boundDir, short min, short max);
int create_circle_clickable(short x0, short y0, short r, char color, bool hold);
int create_rect_clickable(short x0, short y0, short w, short h, char color, bool hold);
void draw_wiimote_objs(struct wiimote_drag_obj *draglist, struct wiimote_click_obj *clicklist);
void erase_old_wiimote_objs(struct wiimote_drag_obj *draglist);
void kill_wiimote_objs(struct wiimote_drag_obj *draglist, struct wiimote_click_obj *clicklist);
bool circle_bound_check(int i);
bool rect_bound_check(int i);

#endif