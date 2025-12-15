
/*
    Reaction Time Game
*/

#ifndef REACTION_H
#define REACTION_H

#include <pico/stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "pico/rand.h"
#include "controls.h"
#include "vga16_graphics_v2.h"
#include "game_primitives.h"

void init_objs_reaction();
void draw_reaction();

#endif