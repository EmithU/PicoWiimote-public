
/*
    Buttons and knobs and other stuff, VERY fun and VERY cool!!!!
*/

#include "shovelware.h"
#include "drawing_logic_2d.h"
#include "game_primitives.h"
#include "vga16_graphics_v2.h"

static bool init_check_shovelware = false;

int slider1;
int slider2;
int slider3;
int slider4;
int slider5;

int objs[20];


// Change [CHANGE_THIS] to the name of the file, and don't add any parameters
void init_shovelware () {

    slider1 = create_circle_movable_bounded(20, 30, 10, RED, BOUND_X, 30, 620-75);
    slider3 = create_circle_movable_bounded(   20, 90, 20, GREEN, BOUND_X, 30, 620-75);
    slider2 = create_circle_movable_bounded(20, 150, 10, DARK_ORANGE, BOUND_X, 30, 620-75);
    slider4 = create_circle_movable_bounded(610, 20, 10, BLACK, BOUND_Y, 20, 460);
    slider5 = create_circle_movable_bounded(570, 20, 10, WHITE, BOUND_Y, 20, 460);

    for (int i = 0; i < 20; i++) {
        objs[i] = create_circle_movable_unbounded(
            100 + (get_rand_32() % 501), 
            100 + (get_rand_32() % 301), 
            5 + (get_rand_32() % 16), 
            get_rand_32() & 0xF);
    }

    bg_color = LIGHT_PINK;
    wiimote_pt_color = MAGENTA;
    clearLowFrame(0, bg_color);

}

void draw_shovelware () {

    if (!init_check_shovelware) {
        init_shovelware();
        init_check_shovelware = true;
    }

    draw_2d_wiimote_objs();

    drawHLine(20, 30, 525, RED);
    drawHLine(20, 90, 525, RED);
    drawHLine(20, 150, 525, RED);
    drawVLine(610, 20, 440, DARK_BLUE);
    drawVLine(570, 20, 440, DARK_BLUE);

    if (button_home) {
        kill_wiimote_objs(draggable_objs, clickable_objs);
        init_check_shovelware = false;
        current_game = GAMES_MAIN_MENU;
        draw_curr_app();
    }

}

