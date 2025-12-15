
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

// Replace [APP_TEMPLATE] with the name of the file, all caps
#ifndef MAIN_MENU_H
#define MAIN_MENU_H

// You will almost definitely need these header files at minimum, feel free to include more though
#include <pico/stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include "controls.h"
#include "vga16_graphics_v2.h"
#include "game_primitives.h"
#include "app_state.h"

// Replace [app_template] with the name of the file
void init_objs_main_menu();
void draw_main_menu();

#endif
