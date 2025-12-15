
#include "color_screen.h"
#include "drawing_logic_2d.h"
#include "game_primitives.h"
#include "vga16_graphics_v2.h"
#include <stdbool.h>

#include "wiimote_bt.h"
#include "controls.h"
#include "app_state.h"
#include "wiimote_pointer.h"

/*
    Just a basic environment for me to test primitives over.
*/

int red_circle;
int green_circle;

// Actually orange. cry about it.
int blue_rectangle;
int green_rectangle_hold;

int orange_circle_hold;
int dark_green_circle_button;

// character array
char lefttext[20];
char righttext[20];

static bool init_check_color_screen = false;

void init_objs_color_screen() {

    red_circle = create_circle_movable_unbounded(50,50,10,RED);
    green_circle = create_circle_movable_bounded(100,150,10,GREEN,BOUND_Y,100,300);
    blue_rectangle = create_rect_movable_bounded(200, 200, 25, 25, ORANGE,BOUND_X,50,300);
    green_rectangle_hold = create_rect_clickable(100, 200, 40, 25, GREEN, CLICKTYPE_TAP);

    orange_circle_hold = create_circle_clickable(200, 100, 10, ORANGE, CLICKTYPE_HOLD);
    dark_green_circle_button = create_circle_clickable(260, 225, 10, DARK_GREEN, CLICKTYPE_TAP);

    bg_color = LIGHT_BLUE;
    wiimote_pt_color = WHITE;
    clearLowFrame(0, bg_color);

}

void draw_color_screen() {

    // Init objs only on first run
    if (!init_check_color_screen) {
        init_objs_color_screen();
        init_check_color_screen = true;
    }

    draw_2d_wiimote_objs();

    // Draw text
    setTextColor2(WHITE, BLACK) ;
    setTextSize(1) ;
    setCursor(10, 10) ;
    writeString("If the red ball is here, ");
    setCursor(10, 20) ;
    writeString("the green ball can be moved");

    setCursor(330, 430) ;
    writeString("If the red ball is here,");
    setCursor(330, 440) ;
    writeString("the green ball cannot be moved");

    drawVLine(320, 0, 480, WHITE);

    // More logic
    if (draggable_objs[red_circle].x > 320) {
        draggable_objs[green_circle].movable = false;
    }
    else {
        draggable_objs[green_circle].movable = true;
    }

    if (clickable_objs[green_rectangle_hold].heldState == true) {
        fillCircle(300, 250, 10, GREEN);
    }

    if (button_home) {
        kill_wiimote_objs(draggable_objs, clickable_objs);
        init_check_color_screen = false;
        current_game = GAMES_MAIN_MENU;
        draw_curr_app();
    }

}