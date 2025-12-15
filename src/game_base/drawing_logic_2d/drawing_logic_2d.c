
/*
    Implements some of the drawing logic for Wiimote-interactable objects
*/

#include "drawing_logic_2d.h"
#include "game_primitives.h"

void check_movable() {
    // Button A is used for dragging
    static int held_obj = -1;     // -1 = no object currently held
    static bool holding = false;  // true while A is actively dragging

    // Button B is used for clicking
    // Object is not held
    if (!button_a) {
        held_obj = -1;

        holding = false;
        return;
    }

    // Object is held
    if (!holding) {
        int count = sizeof(draggable_objs)/sizeof(draggable_objs[0]);
        for (int i = 0; i < count; i++) {
            if (!draggable_objs[i].present) continue;

            // Circle
            if (draggable_objs[i].movable && draggable_objs[i].shape == SHAPE_CIRCLE && circle_bound_check(i)) {
                held_obj = i;      
                break;
            }

            // Rectangle
            if (draggable_objs[i].movable && draggable_objs[i].shape == SHAPE_RECTANGLE && rect_bound_check(i)) {
                held_obj = i;      
                break;
            }
        }
        holding = true;
    }

    // While object is held, keep up
    if (held_obj >= 0) {
        int px = pointer_x;
        int py = pointer_y;

        if (px < 0) px = 0;
        if (px > 639) px = 639;
        if (py < 0) py = 0;
        if (py > 479) py = 479;

        if (draggable_objs[held_obj].bounded == false) {
            draggable_objs[held_obj].x = px;
            draggable_objs[held_obj].y = py;
        }
        else {

            if (draggable_objs[held_obj].bound_dir == BOUND_X) {
                
                // Set bounds
                if (px < draggable_objs[held_obj].min_bound) px = draggable_objs[held_obj].min_bound;
                if (px > draggable_objs[held_obj].max_bound) px = draggable_objs[held_obj].max_bound;

                // Only x can change
                draggable_objs[held_obj].x = px;
            }
            else {
                
                // Set bounds
                if (py < draggable_objs[held_obj].min_bound) py = draggable_objs[held_obj].min_bound;
                if (py > draggable_objs[held_obj].max_bound) py = draggable_objs[held_obj].max_bound;

                // Only y can change
                draggable_objs[held_obj].y = py;
            }
        }
    }
}

void check_clickable() {
    static bool prev_b = false;

    bool b_now = button_a || button_b;

    // Rising edge detect - button was released, now pressed
    if (b_now && !prev_b) {

        // Find first clickable object under pointer
        int count = sizeof(clickable_objs)/sizeof(clickable_objs[0]);
        for (int i = 0; i < count; i++) {

            if (!clickable_objs[i].present) continue;
            if (!clickable_objs[i].clickable) continue;

            // Hit test
            bool hit = false;

            // Circle
            if (clickable_objs[i].shape == SHAPE_CIRCLE) {
                int dx = pointer_x - clickable_objs[i].x;
                int dy = pointer_y - clickable_objs[i].y;
                hit = (dx*dx + dy*dy) <= (clickable_objs[i].r * clickable_objs[i].r);
            }

            // Rectangle
            else if (clickable_objs[i].shape == SHAPE_RECTANGLE) {
                int x = clickable_objs[i].x;
                int y = clickable_objs[i].y;
                int w = clickable_objs[i].w;
                int h = clickable_objs[i].h;
                hit = (pointer_x >= x && pointer_x <= x+w &&
                        pointer_y >= y && pointer_y <= y+h);
            }

            if (!hit) continue;

            // Hold
            if (clickable_objs[i].toggleHeldState == false) {
                clickable_objs[i].heldState = true;
            }

            // Brief click
            else {
                clickable_objs[i].heldState = !clickable_objs[i].heldState;
            }

            // Only the first hit object receives the click
            break;
        }
    }

    // Falling edge: B was pressed last frame but now released
    if (!b_now && prev_b) {
        int count = sizeof(clickable_objs)/sizeof(clickable_objs[0]);
        for (int i = 0; i < count; i++) {
            if (!clickable_objs[i].present) continue;
            if (!clickable_objs[i].clickable) continue;

            // Reset momentary click (toggleClick remains unchanged)
            if (!clickable_objs[i].toggleHeldState) {
                clickable_objs[i].heldState = false;
            }
        }
    }

    prev_b = b_now;

}

void draw_2d_wiimote_objs() {
    erase_old_wiimote_objs(draggable_objs);
    check_movable();
    check_clickable();
    draw_wiimote_objs(draggable_objs,clickable_objs);
}