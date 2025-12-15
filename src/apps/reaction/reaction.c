#include "reaction.h"
#include "drawing_logic_2d.h"
#include "game_primitives.h"
#include "vga16_graphics_v2.h"
#include "pico/rand.h"
#include "controls.h"
#include "wiimote_pointer.h"

static bool init_check_reaction = false;

// Reaction game states
typedef enum {
    RT_STATE_WAITING,
    RT_STATE_READY,
    RT_STATE_GO,
    RT_STATE_RESULT
} rt_state_t;

static rt_state_t rt_state;

// Timing
static absolute_time_t start_time;
static int reaction_time_ms;
static uint32_t wait_delay_ms;

// Random button
typedef enum {
    BTN_A,
    BTN_B,
    BTN_PLUS,
    BTN_MINUS,
    BTN_ONE,
    BTN_TWO,
    BTN_DPAD_UP,
    BTN_DPAD_DOWN,
    BTN_DPAD_LEFT,
    BTN_DPAD_RIGHT
} rt_button_t;

static rt_button_t required_button;

static const char* button_name(rt_button_t b)
{
    switch(b) {
        case BTN_A:          return "PRESS A!";
        case BTN_B:          return "PRESS B!";
        case BTN_PLUS:       return "PRESS +!";
        case BTN_MINUS:      return "PRESS -!";
        case BTN_ONE:        return "PRESS 1!";
        case BTN_TWO:        return "PRESS 2!";
        case BTN_DPAD_UP:    return "DPAD UP!";
        case BTN_DPAD_DOWN:  return "DPAD DOWN!";
        case BTN_DPAD_LEFT:  return "DPAD LEFT!";
        case BTN_DPAD_RIGHT: return "DPAD RIGHT!";
    }
    return "?";
}

static bool correct_button_pressed()
{
    switch(required_button) {
        case BTN_A:         return button_a;
        case BTN_B:         return button_b;
        case BTN_PLUS:      return button_plus;
        case BTN_MINUS:     return button_minus;
        case BTN_ONE:       return button_one;
        case BTN_TWO:       return button_two;
        case BTN_DPAD_UP:   return dpad_up;
        case BTN_DPAD_DOWN: return dpad_down;
        case BTN_DPAD_LEFT: return dpad_left;
        case BTN_DPAD_RIGHT:return dpad_right;
    }
    return false;
}

static bool wrong_button_pressed()
{
    if (correct_button_pressed()) return false;

    return (
        button_a || button_b || button_plus || button_minus ||
        button_one || button_two ||
        dpad_up || dpad_down || dpad_left || dpad_right
    );
}

static void center_big(const char *s, int y, char color)
{
    setTextColor(color);
    setTextSize(2);

    int w = strlen(s) * 12;
    int x = (640 - w) / 2;

    setCursor(x, y);
    writeString((char*)s);
}

void init_objs_reaction()
{
    bg_color = BLACK;
    wiimote_pt_color = WHITE;

    clearLowFrame(0, bg_color);

    rt_state = RT_STATE_WAITING;

    wait_delay_ms = 1000 + (get_rand_32() % 2000);
    required_button = (rt_button_t)(get_rand_32() % 10);

    init_check_reaction = true;
}

void draw_reaction()
{
    if (!init_check_reaction)
        init_objs_reaction();

    clearRect(0, 120, 639, 360, bg_color);

    char msg[64];

    switch(rt_state)
    {
        case RT_STATE_WAITING:
            center_big("WAIT...", 180, YELLOW);

            start_time = make_timeout_time_ms(wait_delay_ms);
            rt_state = RT_STATE_READY;
            break;

        case RT_STATE_READY:
            center_big("WAIT...", 180, YELLOW);

            if (absolute_time_diff_us(get_absolute_time(), start_time) <= 0) {
                rt_state = RT_STATE_GO;
                start_time = get_absolute_time();  // begin timing reaction
            }
            break;

        case RT_STATE_GO:
            center_big("GO!", 150, GREEN);
            center_big(button_name(required_button), 220, WHITE);

            if (correct_button_pressed()) {
                reaction_time_ms =
                    absolute_time_diff_us(start_time, get_absolute_time()) / 1000;

                center_big("Reaction:", 150, CYAN);
                sprintf(msg, "%d ms", reaction_time_ms);
                center_big(msg, 200, CYAN);
                center_big("Press A to play again", 260, WHITE);

                rt_state = RT_STATE_RESULT;
            }
            else if (wrong_button_pressed()) {
                center_big("FAILED!", 180, RED);
                center_big("Press A to play again", 260, WHITE);

                reaction_time_ms = -1;
                rt_state = RT_STATE_RESULT;
            }
            break;

        case RT_STATE_RESULT:
            if (reaction_time_ms >= 0) {
                sprintf(msg, "Reaction: %d ms", reaction_time_ms);
                center_big(msg, 180, CYAN);
            } else {
                center_big("FAILED!", 180, RED);
            }

            center_big("Press A to play again", 260, WHITE);

            if (button_a) {
                wait_delay_ms = 1000 + (get_rand_32() % 2000);
                required_button = (rt_button_t)(get_rand_32() % 10);
                rt_state = RT_STATE_WAITING;
            }
            break;
    }

    if (button_home) {
        kill_wiimote_objs(draggable_objs, clickable_objs);
        init_check_reaction = false;
        current_game = GAMES_MAIN_MENU;
        draw_curr_app();
        return;
    }
}
