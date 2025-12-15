
#ifndef APP_STATE_H
#define APP_STATE_H

#include "vga16_graphics_v2.h"
#include "main_menu.h"

// Include any games here
#include "color_screen.h"
#include "3d_demo.h"
#include "reaction.h"
#include "drawing.h"
#include "spotlight.h"
#include "shovelware.h"
#include "shovelware2.h"
#include "stats.h"
#include "3d_demo_2.h"

typedef enum {
    GAMES_MAIN_MENU,
    GAMES_COLOR_SCREEN,
    GAMES_DRAWING,
    GAMES_3D_DEMO,
    GAMES_3D_DEMO_2,
    GAMES_LAB2,
    GAMES_LAB3,
    GAMES_REACTION,
    GAMES_SPOTLIGHT,
    GAMES_SHOVELWARE,
    GAMES_SHOVELWARE2,
    GAMES_STATS,
} Games;

// Color of background - each app must declare a background color in their init!
extern volatile int bg_color;

extern volatile Games current_game;
void init_curr_app();
void draw_curr_app();

#endif