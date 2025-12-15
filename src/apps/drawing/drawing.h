
/*
    This is a TEMPLATE .h file for apps. This file is never compiled. If you want
    to make a new app, copy this file and replace everything with your desired 
    functionality.

    ALSO
    - Make sure to update cmake
    - Include this file in app_state.h
    - Add this app to the enum in app_state.h
    - Add the draw function to the switch statement in app_state.c

*/

#ifndef DRAWING_H
#define DRAWING_H

// You will almost definitely need these header files at minimum, feel free to include more though
#include <pico/stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include "controls.h"
#include "vga16_graphics_v2.h"
#include "game_primitives.h"
#include "app_state.h"

void init_objs_drawing();
void draw_drawing();

#endif
