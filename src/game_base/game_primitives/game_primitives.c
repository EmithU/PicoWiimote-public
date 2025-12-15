
#include "game_primitives.h"
#include "app_state.h"
#include "vga16_graphics_v2.h"
#include "wiimote_pointer.h"
#include <stdbool.h>

/*

    Most apps will involve certain elements on the screen interfacing with the Wiimote.
    This file defines such objects. It carries the responsibility of drawing the object,
    as well as dictating what happens when the object moves. This could be thought of as
    a game object 'class'.
    
*/

struct wiimote_drag_obj draggable_objs[25];
struct wiimote_click_obj clickable_objs[25];


// Creates a circle with it's top right coords being x0,y0, of radius r and color color.
// This circle can be moved to anywhere on the screen if the user is holding the 'A' button
// over it. Returns the index of the object in the object array.
int create_circle_movable_unbounded(short x0, short y0, short r, char color) {

    // Add to the struct
    int structPtr = 0;
    while (draggable_objs[structPtr].present == true) {
        structPtr++;
    }

    // Specifies that this is a circle
    draggable_objs[structPtr].shape = SHAPE_CIRCLE;

    // Init
    draggable_objs[structPtr].x = x0;
    draggable_objs[structPtr].y = y0;
    draggable_objs[structPtr].r = r;
    draggable_objs[structPtr].color = color;
    draggable_objs[structPtr].movable = true;

    draggable_objs[structPtr].present = true;

    return structPtr;
}

int create_rect_movable_unbounded(short x, short y, short w, short h, char color) {

    // Add to the struct
    int structPtr = 0;
    while (draggable_objs[structPtr].present == true) {
        structPtr++;
    }

    // Specifies that this is a rectangle
    draggable_objs[structPtr].shape = SHAPE_RECTANGLE;

    // Init
    draggable_objs[structPtr].x = x;
    draggable_objs[structPtr].y = y;
    draggable_objs[structPtr].w = w;
    draggable_objs[structPtr].h = h;
    draggable_objs[structPtr].color = color;
    draggable_objs[structPtr].movable = true;

    draggable_objs[structPtr].present = true;

    return structPtr;

}

int create_circle_movable_bounded(short x0, short y0, short r, char color, int boundDir, short min, short max) {
    int i = create_circle_movable_unbounded(x0,y0,r,color);

    draggable_objs[i].bounded = true;
    draggable_objs[i].bound_dir = boundDir;
    draggable_objs[i].max_bound = max;
    draggable_objs[i].min_bound = min;

    return i;
}

int create_rect_movable_bounded(short x, short y, short w, short h, char color, int boundDir, short min, short max) {
    int i = create_rect_movable_unbounded(x,y,w,h,color);

    draggable_objs[i].bounded = true;
    draggable_objs[i].bound_dir = boundDir;
    draggable_objs[i].max_bound = max;
    draggable_objs[i].min_bound = min;

    return i;
}

int create_rect_clickable(short x0, short y0, short w, short h, char color, bool hold) {

    // Add to the struct
    int structPtr = 0;
    while (clickable_objs[structPtr].present == true) {
        structPtr++;
    }

    clickable_objs[structPtr].shape = SHAPE_RECTANGLE;

    // Init
    clickable_objs[structPtr].x = x0;
    clickable_objs[structPtr].y = y0;
    clickable_objs[structPtr].w = w;
    clickable_objs[structPtr].h = h;
    clickable_objs[structPtr].color = color;
    clickable_objs[structPtr].clickable = true;
    clickable_objs[structPtr].toggleHeldState = hold;

    clickable_objs[structPtr].present = true;

    return structPtr;
}

// Creates a circle with it's top right coords being x0,y0, of radius r and color color.
// This circle can be clicked if the user is holding the 'B' button over it. Returns the 
// index of the object in the object array.
int create_circle_clickable(short x0, short y0, short r, char color, bool hold) {

    // Add to the struct
    int structPtr = 0;
    while (clickable_objs[structPtr].present == true) {
        structPtr++;
    }

    // Specifies that this is a circle
    clickable_objs[structPtr].shape = SHAPE_CIRCLE;

    // Init
    clickable_objs[structPtr].x = x0;
    clickable_objs[structPtr].y = y0;
    clickable_objs[structPtr].r = r;
    clickable_objs[structPtr].color = color;
    clickable_objs[structPtr].clickable = true;
    clickable_objs[structPtr].toggleHeldState = hold;

    clickable_objs[structPtr].present = true;

    return structPtr;
}

// Two circles are intersecting
bool circle_bound_check(int i) {
    int pr = 6;              
    int or = draggable_objs[i].r;    

    int dx = pointer_x - draggable_objs[i].x;
    int dy = pointer_y - draggable_objs[i].y;

    int dist2 = dx*dx + dy*dy;
    int sumR  = pr + or;

    return dist2 <= sumR*sumR;
}

// Circle and rectangle are intersecting
bool rect_bound_check(int i)
{
    int pr = 6;                       // pointer radius
    int cx = pointer_x + 3;           // pointer center
    int cy = pointer_y + 3;

    int rx = draggable_objs[i].x;            // rectangle top-left
    int ry = draggable_objs[i].y;
    int rw = draggable_objs[i].w;
    int rh = draggable_objs[i].h;

    // Closest point on rect to circle
    int closest_x = cx;
    if (closest_x < rx) closest_x = rx;
    else if (closest_x > rx + rw) closest_x = rx + rw;

    int closest_y = cy;
    if (closest_y < ry) closest_y = ry;
    else if (closest_y > ry + rh) closest_y = ry + rh;

    // Distance from circle center - closest point
    int dx = cx - closest_x;
    int dy = cy - closest_y;
    int dist2 = dx*dx + dy*dy;

    return dist2 <= (pr * pr);
}

// Yes, I know that I can iterate once and draw both lists. No, I do not care to fix it.
void draw_wiimote_objs(struct wiimote_drag_obj *draglist, struct wiimote_click_obj *clicklist) {

    for (int i = 0; i < 25; i++) {
        if (!draglist[i].present) continue;

        // circle
        if (draglist[i].shape == SHAPE_CIRCLE) {

            int x = draglist[i].x;
            int y = draglist[i].y;
            int r = draglist[i].r;

            // Skip if offscreen
            if (x < -r || x > 639 + r || y < -r || y > 479 + r)
                continue;

            // Clamp safely inside screen bounds
            if (x < r) x = r;
            if (x > 639 - r) x = 639 - r;
            if (y < r) y = r;
            if (y > 479 - r) y = 479 - r;

            fillCircle(x, y, r, draglist[i].color);
        }

        // rectangle
        else if (draglist[i].shape == SHAPE_RECTANGLE) {

            int x = draglist[i].x;
            int y = draglist[i].y;
            int w = draglist[i].w;
            int h = draglist[i].h;

            // Skip if totally offscreen
            if (x + w < 0 || x > 639 || y + h < 0 || y > 479)
                continue;

            fillRect(draglist[i].x, draglist[i].y, draglist[i].w, draglist[i].h, bg_color);

            // Clamp to valid area
            if (x < 0) x = 0;
            if (x + w > 639) w = 639 - x;
            if (y < 0) y = 0;
            if (y + h > 479) h = 479 - y;

            fillRect(x, y, w, h, draglist[i].color);
        }
    }

    // Clickable objs - cannot be moved, so no need for bounds checks
    for (int i = 0; i < 25; i++) {
        if (!clicklist[i].present) continue;

        // circle
        if (clicklist[i].shape == SHAPE_CIRCLE) {

            int x = clicklist[i].x;
            int y = clicklist[i].y;
            int r = clicklist[i].r;

            fillCircle(x, y, r, clicklist[i].color);
        }

        // rectangle
        else if (clicklist[i].shape == SHAPE_RECTANGLE) {

            int x = clicklist[i].x;
            int y = clicklist[i].y;
            int w = clicklist[i].w;
            int h = clicklist[i].h;

            fillRect(x, y, w, h, clicklist[i].color);
        }
    }
}

// For erasing ghost movable Wiimote objects while moving them
void erase_old_wiimote_objs(struct wiimote_drag_obj *draglist) {

    for (int i = 0; i < 25; i++) {
        if (!draglist[i].present) continue;

        // circle
        if (draglist[i].shape == SHAPE_CIRCLE) {

            int x = draglist[i].x;
            int y = draglist[i].y;
            int r = draglist[i].r;

            // Skip if offscreen
            if (x < -r || x > 639 + r || y < -r || y > 479 + r)
                continue;

            // Clamp safely inside screen bounds
            if (x < r) x = r;
            if (x > 639 - r) x = 639 - r;
            if (y < r) y = r;
            if (y > 479 - r) y = 479 - r;

            fillCircle(x, y, r, bg_color);
        }

        // rectangle
        else if (draglist[i].shape == SHAPE_RECTANGLE) {

            int x = draglist[i].x;
            int y = draglist[i].y;
            int w = draglist[i].w;
            int h = draglist[i].h;

            // Skip if totally offscreen
            if (x + w < 0 || x > 639 || y + h < 0 || y > 479)
                continue;

            fillRect(draglist[i].x, draglist[i].y, draglist[i].w, draglist[i].h, bg_color);

            // Clamp to valid area
            if (x < 0) x = 0;
            if (x + w > 639) w = 639 - x;
            if (y < 0) y = 0;
            if (y + h > 479) h = 479 - y;

            fillRect(x, y, w, h, bg_color);
        }
    }
}

// Sets both arrays to null
void kill_wiimote_objs(struct wiimote_drag_obj *draglist, struct wiimote_click_obj *clicklist) {
    
    // For dragList
    for (int i = 0; i < 25; i++) {
        
        // Shape present (lets us preallocate this array and save a ton of headaches)
        draglist[i].present = false;

        // Shape type (0=circle, 1=rect)
        draglist[i].shape = 0;

        // All shapes
        draglist[i].x = 0;
        draglist[i].y = 0;
        draglist[i].color = 0;
        draglist[i].movable = false;
        draglist[i].bounded = false;
        draglist[i].bound_dir = 0;

        // Relevant if circle
        draglist[i].r = 0;

        // Relevant if rectangle
        draglist[i].w = 0;
        draglist[i].h = 0;

        // If we need to deal with bounds somehow. Either bounded along x or y.
        // If bounded on x, cannot move along y, and vice versa. Good for sliders.
        draglist[i].min_bound = 0;
        draglist[i].max_bound = 0;
    }

    // For clickList
    for (int i = 0; i < 25; i++) {

        // Shape present (lets us preallocate this array and save a ton of headaches)
        clicklist[i].present = false;

        // Shape type (0=circle, 1=rect)
        clicklist[i].shape = 0;

        // All shapes
        clicklist[i].x = 0;
        clicklist[i].y = 0;
        clicklist[i].color = 0;
        clicklist[i].clickable = false;
        clicklist[i].toggleHeldState = false;
        clicklist[i].heldState = false;
        clicklist[i].clicked = false;

        // Relevant if circle
        clicklist[i].r = 0;

        // Relevant if rectangle
        clicklist[i].w = 0;
        clicklist[i].h = 0;
        
    }
}
