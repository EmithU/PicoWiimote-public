
/*
    Buttons and knobs and other stuff, VERY fun and VERY cool!!!!
*/

#include "stats.h"
#include "app_state.h"
#include "drawing_logic_2d.h"
#include "game_primitives.h"
#include "vga16_graphics_v2.h"

static bool init_check_stats = false;

void writeBoolColored(char *label, bool value) {
    setTextColor(value ? GREEN : RED);
    writeString(label);
    setTextColor(BLACK);  // reset for future text
}

// Change [CHANGE_THIS] to the name of the file, and don't add any parameters
void init_objs_stats () {

    bg_color = WHITE;
    wiimote_pt_color = BLACK;
    clearLowFrame(0, bg_color);

}

void draw_stats () {

    if (!init_check_stats) {
        init_objs_stats();
        init_check_stats = true;
    }

    // Snapshot globals into locals
    bool l_dpad_left   = dpad_left;
    bool l_dpad_right  = dpad_right;
    bool l_dpad_up     = dpad_up;
    bool l_dpad_down   = dpad_down;

    bool l_button_a     = button_a;
    bool l_button_b     = button_b;
    bool l_button_plus  = button_plus;
    bool l_button_minus = button_minus;
    bool l_button_home  = button_home;
    bool l_button_one   = button_one;
    bool l_button_two   = button_two;

    short l_raw_x = raw_x;
    short l_raw_y = raw_y;
    short l_raw_z = raw_z;

    ir_blob_t l_blob_1 = blob_1;
    ir_blob_t l_blob_2 = blob_2;
    ir_blob_t l_blob_3 = blob_3;
    ir_blob_t l_blob_4 = blob_4;

    char buf[64];

    const short X0 = 140;   // centered left margin
    const short Y0 = 40;    // top for the stats area
    const short W  = 360;
    const short H  = 340;

    clearRect(X0-10, Y0-10, X0+W+10, Y0+H+10, bg_color);

    setTextColor(BLACK);
    setTextSize(1);

    setCursor(X0 + 40, Y0);
    writeStringBig("Controller Stats");

    drawHLine(X0, Y0 + 28, 360, BLACK);

    // DPAD (CENTERED BLOCK)
    setCursor(X0, Y0 + 45);
    writeString("D-Pad:");

    setCursor(X0 + 20, Y0 + 60);  writeBoolColored("Up",    l_dpad_up);
    setCursor(X0 + 90, Y0 + 60);  writeBoolColored("Down",  l_dpad_down);
    setCursor(X0 + 170, Y0 + 60); writeBoolColored("Left",  l_dpad_left);
    setCursor(X0 + 240, Y0 + 60); writeBoolColored("Right", l_dpad_right);

    // BUTTONS
    setCursor(X0, Y0 + 90);
    writeString("Buttons:");

    setCursor(X0 + 20,  Y0 + 105); writeBoolColored("A", l_button_a);
    setCursor(X0 + 55,  Y0 + 105); writeBoolColored("B", l_button_b);
    setCursor(X0 + 90,  Y0 + 105); writeBoolColored("1", l_button_one);
    setCursor(X0 + 125, Y0 + 105); writeBoolColored("2", l_button_two);
    setCursor(X0 + 160, Y0 + 105); writeBoolColored("+", l_button_plus);
    setCursor(X0 + 195, Y0 + 105); writeBoolColored("-", l_button_minus);
    setCursor(X0 + 230, Y0 + 105); writeBoolColored("H", l_button_home);

    // ACCEL
    setCursor(X0, Y0 + 140);
    writeString("Accel:");

    sprintf(buf, "X:%6d  Y:%6d  Z:%6d", l_raw_x, l_raw_y, l_raw_z);
    setCursor(X0 + 20, Y0 + 155);
    writeString(buf);

    sprintf(buf, "Magnitude: %d", l_raw_x*l_raw_x + l_raw_y*l_raw_y + l_raw_z*l_raw_z - 98*98);
    setCursor(X0 + 20, Y0 + 170);
    writeString(buf);

    // IR BLOBS
    setCursor(X0, Y0 + 200);
    writeString("IR Blobs:");

    sprintf(buf, "1: x=%4d y=%4d s=%3u", l_blob_1.x, l_blob_1.y, l_blob_1.s);
    setCursor(X0 + 20, Y0 + 215); writeString(buf);

    sprintf(buf, "2: x=%4d y=%4d s=%3u", l_blob_2.x, l_blob_2.y, l_blob_2.s);
    setCursor(X0 + 20, Y0 + 230); writeString(buf);

    sprintf(buf, "3: x=%4d y=%4d s=%3u", l_blob_3.x, l_blob_3.y, l_blob_3.s);
    setCursor(X0 + 20, Y0 + 245); writeString(buf);

    sprintf(buf, "4: x=%4d y=%4d s=%3u", l_blob_4.x, l_blob_4.y, l_blob_4.s);
    setCursor(X0 + 20, Y0 + 260); writeString(buf);

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

    if (l_button_home) {
        kill_wiimote_objs(draggable_objs, clickable_objs);
        init_check_stats = false;
        current_game = GAMES_MAIN_MENU;
        draw_curr_app();
    }
}
