
/*
    This file contains variables for each of the buttons/controller values, as well
    as methods for converting accel/IR into usable values.
*/

#include "controls.h"

// Buttons
volatile bool dpad_left = false;
volatile bool dpad_right = false;
volatile bool dpad_up = false;
volatile bool dpad_down = false;

volatile bool button_a = false;
volatile bool button_b = false;
volatile bool button_plus = false;
volatile bool button_minus = false;
volatile bool button_home = false;
volatile bool button_one = false;
volatile bool button_two = false;

// Accel
volatile short raw_x = 0;
volatile short raw_y = 0;
volatile short raw_z = 0;

// IR blobs
volatile ir_blob_t blob_1 = {0, 0, 0};
volatile ir_blob_t blob_2 = {0, 0, 0};
volatile ir_blob_t blob_3 = {0, 0, 0};
volatile ir_blob_t blob_4 = {0, 0, 0};
