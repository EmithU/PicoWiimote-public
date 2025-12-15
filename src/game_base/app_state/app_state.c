
#include "app_state.h"
#include "color_screen.h"
#include "3d_demo.h"
#include "main_menu.h"
#include "shovelware.h"
#include "shovelware2.h"
#include "spotlight.h"
#include "stats.h"
#include "vga16_graphics_v2.h"

volatile Games current_game = GAMES_MAIN_MENU;
volatile int bg_color = BLACK;

// helper: call the correct draw function for a given game
void init_curr_app() {

    switch (current_game) {

        case GAMES_MAIN_MENU:
            init_objs_main_menu();
            break;

        case GAMES_COLOR_SCREEN:
            init_objs_color_screen();
            break;

        case GAMES_3D_DEMO:
            init_objs_3d_demo();
            break;

        case GAMES_3D_DEMO_2:
            init_objs_3d_demo_2();
            break;
        
        case GAMES_LAB2:
            draw_color_screen();
            break;

        case GAMES_SHOVELWARE:
            init_objs_shovelware();
            break;

        case GAMES_DRAWING:
            init_objs_drawing();
            break;

        case GAMES_SHOVELWARE2:
            init_objs_shovelware2();
            break;

        case GAMES_STATS:
            init_objs_stats();
            break;

        case GAMES_REACTION:
            init_objs_reaction();
            break;

        case GAMES_SPOTLIGHT:
            init_objs_spotlight();
            break;

        case GAMES_LAB3:
            init_objs_color_screen();
            break;
    }

}

// helper: call the correct draw function for a given game
void draw_curr_app() {

    switch (current_game) {

        case GAMES_MAIN_MENU:
            draw_main_menu();
            break;

        case GAMES_COLOR_SCREEN:
            draw_color_screen();
            break;

        case GAMES_3D_DEMO:
            draw_3d_demo();
            break;

        case GAMES_3D_DEMO_2:
            draw_3d_demo_2();
            break;

        case GAMES_LAB2:
            draw_color_screen();
            break;

        case GAMES_DRAWING:
            draw_drawing();
            break;

        case GAMES_SHOVELWARE:
            draw_shovelware();
            break;

        case GAMES_SHOVELWARE2:
            draw_shovelware2();
            break;

        case GAMES_STATS:
            draw_stats();
            break;

        case GAMES_REACTION:
            draw_reaction();
            break;
        
        case GAMES_SPOTLIGHT:
            draw_spotlight();
            break;

        case GAMES_LAB3:
            draw_color_screen();
            break;
    }

}