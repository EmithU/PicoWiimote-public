
#ifndef COLOR_SCREEN_H
#define COLOR_SCREEN_H

#include <pico/stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include "controls.h"
#include "vga16_graphics_v2.h"
#include "game_primitives.h"
#include "drawing_logic_2d.h"
#include "app_state.h"

void init_objs_color_screen();
void draw_color_screen();

#endif