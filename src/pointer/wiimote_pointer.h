
#ifndef WIIMOTE_POINTER_H
#define WIIMOTE_POINTER_H

#include <pico/stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include "controls.h"
#include "vga16_graphics_v2.h"
#include "app_state.h"

void draw_pointer();

extern volatile short pointer_x;
extern volatile short pointer_y;

extern volatile int wiimote_pt_color;

#endif