
#ifndef DRAWING_LOGIC_2D_H
#define DRAWING_LOGIC_2D_H

#include <pico/stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include "controls.h"
#include "vga16_graphics_v2.h"
#include "game_primitives.h"
#include "app_state.h"

void check_movable();
void check_clickable();
void draw_2d_wiimote_objs();

#endif