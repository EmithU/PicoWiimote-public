/*
    Reaction Time Game

    TEMPLATE NOTES:
    - Make sure to update CMakeLists.txt
    - Include this file in app_state.h
    - Add this app to the enum in app_state.h
    - Add the draw function to the switch statement in app_state.c
*/

#ifndef SPOTLIGHT_H
#define SPOTLIGHT_H

#include <pico/stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include "pico/rand.h"
#include "controls.h"
#include "vga16_graphics_v2.h"
#include "game_primitives.h"

// Initialization and draw loop
void init_objs_spotlight();
void draw_spotlight();

#endif