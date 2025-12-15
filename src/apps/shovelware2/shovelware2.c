
/*
    Buttons and knobs and other stuff, VERY fun and VERY cool!!!!
*/

#include "shovelware2.h"
#include "drawing_logic_2d.h"
#include "game_primitives.h"
#include "vga16_graphics_v2.h"

static bool init_check_shovelware2 = false;

int slider1r;
int slider2r;
int slider3r;
int slider4r;
int slider5r;

int objsr[20];


// Change [CHANGE_THIS] to the name of the file, and don't add any parameters
void init_shovelware2 () {

    slider1r = create_rect_movable_bounded(20, 30, 10, 10,RED, BOUND_X, 30, 620-75);
    slider3r = create_rect_movable_bounded(20, 90, 20, 20,GREEN, BOUND_X, 30, 620-75);
    slider2r = create_rect_movable_bounded(20, 150, 10, 10,DARK_ORANGE, BOUND_X, 30, 620-75);
    slider4r = create_rect_movable_bounded(610, 20, 10, 10,BLACK, BOUND_Y, 20, 460);
    slider5r = create_rect_movable_bounded(570, 20, 10, 10,WHITE, BOUND_Y, 20, 460);

    for (int i = 0; i < 20; i++) {
        objsr[i] = create_rect_movable_unbounded(
            100 + (get_rand_32() % 501), 
            100 + (get_rand_32() % 301), 
            30 + (get_rand_32() % 31),
           30 + (get_rand_32() % 31), 
            get_rand_32() & 0xF);
    }

    bg_color = LIGHT_BLUE;
    wiimote_pt_color = MAGENTA;
    clearLowFrame(0, bg_color);

}

void draw_shovelware2 () {

    if (!init_check_shovelware2) {
        init_shovelware2();
        init_check_shovelware2 = true;
    }

    draw_2d_wiimote_objs();

    drawHLine(20, 35, 525, RED);
    drawHLine(20, 100, 525, RED);
    drawHLine(20, 155, 525, RED);
    drawVLine(615, 20, 440, DARK_BLUE);
    drawVLine(575, 20, 440, DARK_BLUE);

    if (button_home) {
        kill_wiimote_objs(draggable_objs, clickable_objs);
        init_check_shovelware2 = false;
        current_game = GAMES_MAIN_MENU;
        draw_curr_app();
    }

}

