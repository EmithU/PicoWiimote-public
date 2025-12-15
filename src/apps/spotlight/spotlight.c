#include "spotlight.h"
#include "drawing_logic_2d.h"
#include "game_primitives.h"
#include "vga16_graphics_v2.h"
#include "pico/rand.h"
#include "controls.h"
#include "wiimote_pointer.h"

static bool init_check_spotlight = false;

// Spotlight game states
typedef enum {
    SPOT_STATE_PLAYING,
    SPOT_STATE_FOUND,
    SPOT_STATE_RESULT
} sp_state_t;

static sp_state_t sp_state;

// Hidden object position
static int obj_x;
static int obj_y;
static int obj_r = 20; 

// Timer
static absolute_time_t start_time;
static int elapsed_ms;

// Utility
static bool pointer_on_object()
{
    int dx = pointer_x - obj_x;
    int dy = pointer_y - obj_y;
    return (dx*dx + dy*dy <= obj_r * obj_r);
}

// Draw centered big text
static void center_big(const char *s, int y, char color)
{
    setTextColor(color);
    setTextSize(2);
    int w = strlen(s) * 12;
    int x = (640 - w) / 2;
    setCursor(x, y);
    writeString((char*)s);
}

void init_objs_spotlight()
{
    bg_color = BLACK;
    wiimote_pt_color = WHITE;
    clearLowFrame(0, bg_color);

    obj_x = 40 + (get_rand_32() % 560);
    obj_y = 40 + (get_rand_32() % 400);

    start_time = get_absolute_time();
    sp_state = SPOT_STATE_PLAYING;

    init_check_spotlight = true;
}

void draw_spotlight()
{


    if (!init_check_spotlight)
        init_objs_spotlight();

    clearLowFrame(0, bg_color);

    switch (sp_state)
    {
        case SPOT_STATE_PLAYING:
        {
            // Spotlight circle (bigger pointer)
            drawCircle(pointer_x, pointer_y, 20, WHITE);

            // If pointer overlaps object, reveal it
            if (pointer_on_object()) {
                fillCircle(obj_x, obj_y, obj_r, YELLOW);
            }

            center_big("Find the object!", 120, WHITE);

            // Winning condition
            if (pointer_on_object() && button_a) {
                absolute_time_t now = get_absolute_time();
                elapsed_ms = absolute_time_diff_us(start_time, now) / 1000;
                sp_state = SPOT_STATE_RESULT;
            }
        }
        break;

        case SPOT_STATE_RESULT:
        {
            center_big("YOU FOUND IT!", 150, GREEN);

            char msg[64];
            sprintf(msg, "Time: %d ms", elapsed_ms);
            center_big(msg, 200, CYAN);

            center_big("Press B to play again", 260, WHITE);

            if (button_b) {
                init_objs_spotlight();
                return;
            }
        }
        break;
    }

    if (button_home) {
        kill_wiimote_objs(draggable_objs, clickable_objs);
        init_check_spotlight = false;
        current_game = GAMES_MAIN_MENU;
        draw_curr_app();
        return;
    }
}
