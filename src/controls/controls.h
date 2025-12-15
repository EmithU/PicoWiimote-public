
/*
    This file simply declares each of the controls for the Wiimote
    as games/apps will later need a clean way to read the controls, and ideally
    the apps are abstracted away from the raw bits from the Wiimote connection
    itself.
*/

#ifndef CONTROLS_H
#define CONTROLS_H

#include <stdint.h>
#include <stdbool.h>

// Just so including controls also gets pointer x,y
#include "wiimote_pointer.h"

// Buttons
extern volatile bool dpad_left;
extern volatile bool dpad_right;
extern volatile bool dpad_up;
extern volatile bool dpad_down;

extern volatile bool button_a;
extern volatile bool button_b;
extern volatile bool button_plus;
extern volatile bool button_minus;
extern volatile bool button_home;
extern volatile bool button_one;
extern volatile bool button_two;

// Accel
extern volatile short raw_x;
extern volatile short raw_y;
extern volatile short raw_z;

// IR blobs
typedef struct {
    short x;
    short y;
    uint16_t s;
} ir_blob_t;

extern volatile ir_blob_t blob_1;
extern volatile ir_blob_t blob_2;
extern volatile ir_blob_t blob_3;
extern volatile ir_blob_t blob_4;
#endif