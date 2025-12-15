
/*
    This is responsible for setting the main menu of the game.
*/

#include "main_menu.h"
#include "app_state.h"
#include "vga16_graphics_v2.h"
#include "wiimote_pointer.h"

// Create int variables for any Wiimote objs at the top of the file
int  app_1; // Color Playground (color_screen.c)
int  app_2; // 3D Cube Demo (cube.c)
int  app_3; // Farhan's second mystery 3d app
int  app_4; // Reaction Time Game (reaction.c)
int  app_5; // ECE 4760 Lab 2 (lab2.c)
int  app_6; // ECE 4760 Lab 3 (lab3.c)
int  app_7; // Drawing (drawing.c)
int  app_8; // Spotlight Game (spotlight.c)
int  app_9; // Shovelware 1 (shovelware.c)
int app_10; // Shovelware 2 (shovelware2.c)
int app_11; // Wiimote Stats (stats.c)

static bool init_check_main_menu = false;
void load_app();

void init_objs_main_menu () {

    int w = 140;
    int h = 130;

    int xoff = 5;
    int yoff = 5;

    app_1  = create_rect_clickable(20+xoff, 20+yoff, w, h,RED,CLICKTYPE_TAP);
    app_2  = create_rect_clickable(172+xoff, 20+yoff,  w, h,ORANGE,CLICKTYPE_TAP);
    app_3  = create_rect_clickable(324+xoff, 20+yoff, w, h, DARK_BLUE,CLICKTYPE_TAP);
    app_4  = create_rect_clickable(476+xoff, 20+yoff, w, h,MAGENTA,CLICKTYPE_TAP);

    app_5  = create_rect_clickable(20+xoff, 170+yoff, w, h,PINK,CLICKTYPE_TAP);
    app_6  = create_rect_clickable(172+xoff, 170+yoff, w, h,MED_GREEN,CLICKTYPE_TAP);
    app_7  = create_rect_clickable(324+xoff, 170+yoff,w, h,LIGHT_BLUE,CLICKTYPE_TAP);
    app_8  = create_rect_clickable(476+xoff, 170+yoff, w, h,DARK_GREEN,CLICKTYPE_TAP);

    app_9  = create_rect_clickable(20+xoff, 320+yoff, w, h,BLUE,CLICKTYPE_TAP);
    app_10 = create_rect_clickable(172+xoff, 320+yoff, w, h,LIGHT_PINK,CLICKTYPE_TAP);
    app_11 = create_rect_clickable(324+xoff, 320+yoff, w, h,DARK_ORANGE,CLICKTYPE_TAP);

    bg_color = WHITE;
    wiimote_pt_color = BLACK;
    clearLowFrame(0, bg_color);

}

void draw_main_menu () {

    draw_2d_wiimote_objs();

    // Draw all of the text

    // Write out names of each app

    setCursor(45, 85) ;
    writeString("Color Playground");

    setCursor(225, 85) ;
    writeString("3D Demo");

    setTextColor(WHITE) ;
    setCursor(370, 85) ;
    writeString("3D Demo 2");

    setCursor(510, 80) ;
    writeString("Reaction Time");
    setCursor(535, 90) ;
    writeString("Game");

    setTextColor(BLACK) ;
    setCursor(55, 230) ;
    writeString("ECE4760 Lab 2");

    setCursor(205, 230) ;
    writeString("ECE4760 Lab 3");

    setCursor(375, 230) ;
    writeString("Drawing");

    setTextColor(WHITE) ;
    setCursor(525, 230) ;
    writeString("Spotlight");

    setCursor(60, 385) ;
    writeString("Shovelware");

    setTextColor(BLACK) ;
    setCursor(205, 385) ;
    writeString("Shovelware 2");

    setTextColor(WHITE) ;
    setCursor(360, 385) ;
    writeString("Wiimote Stats");

    // Bottom Right
    setTextColor(BLACK) ;
    setTextSize(1) ;

    setCursor(510, 345) ;
    writeStringBig("PicoWiimote") ;
    setCursor(481, 365) ;
    writeString("ECE 4760 Final Project");
    setCursor(481, 375) ;
    writeString("Emith Uyanwatte (eu52)");
    setCursor(481, 385) ;
    writeString("Farhan Naqib (fn72)");

    setTextSize(1) ;

    setCursor(481, 415) ;
    writeString("Press A to enter an app!");

    setCursor(481, 425) ;
    writeString("Press HOME to come back!");

    if (!init_check_main_menu) {
        init_objs_main_menu();
        init_check_main_menu = true;
    }

    load_app();

}

void load_app() {

    if (clickable_objs[app_1].heldState) {
        init_check_main_menu = false;
        kill_wiimote_objs(draggable_objs, clickable_objs);
        current_game = GAMES_COLOR_SCREEN;
    }
    
    else if (clickable_objs[app_2].heldState) {
        init_check_main_menu = false;
        kill_wiimote_objs(draggable_objs, clickable_objs);
        current_game = GAMES_3D_DEMO;
    }

    else if (clickable_objs[app_3].heldState) {
        init_check_main_menu = false;
        kill_wiimote_objs(draggable_objs, clickable_objs);
        current_game = GAMES_3D_DEMO_2;
    }

    else if (clickable_objs[app_4].heldState) {
        init_check_main_menu = false;
        kill_wiimote_objs(draggable_objs, clickable_objs);
        current_game = GAMES_REACTION;
    }

    else if (clickable_objs[app_5].heldState) {
        init_check_main_menu = false;
        kill_wiimote_objs(draggable_objs, clickable_objs);
        current_game = GAMES_LAB2;
    }

    else if (clickable_objs[app_6].heldState) {
        init_check_main_menu = false;
        kill_wiimote_objs(draggable_objs, clickable_objs);
        current_game = GAMES_LAB3;
    }

    else if (clickable_objs[app_7].heldState) {
        init_check_main_menu = false;
        kill_wiimote_objs(draggable_objs, clickable_objs);
        current_game = GAMES_DRAWING;
    }

    else if (clickable_objs[app_8].heldState) {
        init_check_main_menu = false;
        kill_wiimote_objs(draggable_objs, clickable_objs);
        current_game = GAMES_SPOTLIGHT;
    }

    else if (clickable_objs[app_9].heldState) {
        init_check_main_menu = false;
        kill_wiimote_objs(draggable_objs, clickable_objs);
        current_game = GAMES_SHOVELWARE;
    }

    else if (clickable_objs[app_10].heldState) {
        init_check_main_menu = false;
        kill_wiimote_objs(draggable_objs, clickable_objs);
        current_game = GAMES_SHOVELWARE2;
    }

    else if (clickable_objs[app_11].heldState) {
        init_check_main_menu = false;
        kill_wiimote_objs(draggable_objs, clickable_objs);
        current_game = GAMES_STATS;
    }

}

