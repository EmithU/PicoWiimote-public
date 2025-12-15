
/*
    PicoWiimote
    ECE 4760 - Digital Systems Design Using Microcontrollers
    Emith Uyanwatte, Farhan Naqib

    This code establishes a connection between a Wiimote and Raspberry Pi Pico W, enables the IR camera
    on the Wiimote, and allows the user to play basic motion-controlled games using VGA.

    CORE 0
     - Bluetooth
    
    CORE 1
     - VGA
     - Game logic

    With this structure, core 0 is mainly responsible for controls, core 1 is mainly responsible for gameplay.
*/

#include <btstack_util.h>
#include <pico/stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include "color_screen/color_screen.h"
#include "main_menu.h"
#include "pico/multicore.h"
#include "src/game_base/game_primitives/game_primitives.h"
#include "vga16_graphics_v2.h"

#include "wiimote_bt.h"
#include "controls.h"
#include "app_state.h"
#include "wiimote_pointer.h"

// ==========================================
// === VGA graphics library
// ==========================================
#include <stdio.h>
#include <stdlib.h>
// #include <math.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/dma.h"
//
#include "hardware/vreg.h"
#include "hardware/clocks.h"

// ==========================================
// === protothreads globals
// ==========================================
#include "hardware/sync.h"
#include "hardware/timer.h"
#include "pico/multicore.h"
#include "string.h"
// protothreads header
#include "pt_cornell_rp2040_v1_4.h"

#define ANSI_CLEAR_LINE "\033[K"
#define ANSI_HIDE_CURSOR "\033[?25l"
#define ANSI_SHOW_CURSOR "\033[?25h"
#define ANSI_GREEN       "\033[32m"
#define ANSI_RESET       "\033[0m"

// helper: print 0/1, green if true
static void print_bool_colored(const char *label, bool value);

void wait_for_vsync_pulse(void) {

    // Wait until VSYNC goes low (start of pulse)
    while (!gpio_get(VSYNC)) { }

    // Wait until VSYNC goes high again (end of pulse)
    while (gpio_get(VSYNC)) { }

    // Wait until VSYNC goes low (start of pulse)
    while (!gpio_get(VSYNC)) { }
}

// Responsible for generating graphics
static PT_THREAD (protothread_graphics(struct pt *pt)) {
    PT_BEGIN(pt);
    // the protothreads interval timer
    PT_INTERVAL_INIT() ;

    while(true) {

        uint32_t t0 = time_us_32();
        while (gpio_get(VSYNC)) {
            if (time_us_32() - t0 > 20000) break;
            PT_YIELD_usec(50);
        }

        // Each function below should be responsible for drawing something. Things will
        // be drawn in in the order that they're called. For example, the pointer
        // will be drawn above all else because draw_pointer is last. 
        // 
        // In order for this to work, every frame must be fully redrawn (we cannot draw 
        // once, then assume the pixel values will stay set). Because we are using protothreads
        // and VGA requires somewhat strict timing requirements, any drawing MUST be done in
        // this thread. We'll have to structure the rest of our code around this idea.

        // THIS CAN GET INCREDIBLY COMPLEX.
        //
        // TO make it not complex, things will be rendered in the following order:
        //  1. Non-wiimote interactable graphics
        //  2. Wiimote-interactable graphics
        //  3. Pointer
        //
        // We also assume that objects cannot be erased without the full app being reset.
        // This heavily restricts the complexity of the apps we can make, but we don't have
        // much time and the point of this project was to connect the Wiimote, not really
        // the apps themselves.

        // NOTE: Above is slightly modified. Now, the apps are responsible for everything, 
        // so we call a draw function, the apps deal with all the (non-)interactable graphics, 
        // and as long as its fast enough, it'll be shown on screen.

        wait_for_vsync_pulse();  // sync to start-of-frame

        // 1. Draw apps - on first run, also init objs
        draw_curr_app();

        // 3. Draw Wiimote
        // Should always be called last (forces pointer to be showing no matter what)
        draw_pointer();

        //wait_for_vsync_pulse();  // sync to start-of-frame
        PT_YIELD_usec(20000);
   }
   PT_END(pt);
} // graphics thread

// helper: print 0/1, green if true
static void print_bool_colored(const char *label, bool value) {
    printf("%s: ", label);
    if (value) {
        printf(ANSI_GREEN "1" ANSI_RESET);
    } else {
        printf("0");
    }
    printf("  "); // spacing
}

static PT_THREAD (protothread_debug(struct pt *pt))
{
    PT_BEGIN(pt);

    printf(ANSI_HIDE_CURSOR);
    fflush(stdout);

    // Add gap between bt serial and this serial
    printf("\n");

    // Reserve lines for this
    printf("------ CONTROLLER OUTPUT ------\n\n\n\n\n\n\n\n");
    fflush(stdout);

    PT_INTERVAL_INIT() ;

    while (true) {
        // Snapshot globals (volatile) into locals
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

        // Move cursor UP 7 lines to reserved status line
        printf("\033[8A");
        printf(ANSI_CLEAR_LINE);

        // Print everything on that single line, then a newline
        // D-pad
        print_bool_colored("DPad Up", l_dpad_up);
        print_bool_colored("DPad Down", l_dpad_down);
        print_bool_colored("DPad Left", l_dpad_left);
        print_bool_colored("DPad Right", l_dpad_right);

        // Buttons
        printf("\n");
        print_bool_colored("A", l_button_a); 
        print_bool_colored("B", l_button_b);
        print_bool_colored("1", l_button_one);
        print_bool_colored("2", l_button_two);
        print_bool_colored("+", l_button_plus);
        print_bool_colored("-", l_button_minus);
        print_bool_colored("H", l_button_home);

        // Accel
        printf("\nAccel_X: %4d Accel_Y: %4d Accel_Z: %4d ", l_raw_x, l_raw_y, l_raw_z);
        // Accel swing > 100k
        printf("\nAccel Magn: %7d", l_raw_x*l_raw_x + l_raw_y*l_raw_y + l_raw_z*l_raw_z - 98*98);

        // IR
        printf("\nIR1: x=%4d y=%4d s=%3u", l_blob_1.x, l_blob_1.y, l_blob_1.s);
        printf("\nIR2: x=%4d y=%4d s=%3u", l_blob_2.x, l_blob_2.y, l_blob_2.s);
        printf("\nIR3: x=%4d y=%4d s=%3u", l_blob_3.x, l_blob_3.y, l_blob_3.s);
        printf("\nIR4: x=%4d y=%4d s=%3u", l_blob_4.x, l_blob_4.y, l_blob_4.s);

        // End the status line with a newline, so next time we can move 1 line up
        printf("\n");

        fflush(stdout);

        // ~20 Hz update rate
        PT_YIELD_INTERVAL(50000);
    }

    // Should never get here, inf loop!!!
    printf(ANSI_SHOW_CURSOR);
    fflush(stdout);
    PT_END(pt);

} // Serial debug thread


void core1_main(void) {

    // Initialize the VGA screen
    initVGA() ;

    // Small delay so core 0 can finish init + BT logs
    sleep_ms(5000);

    pt_add_thread(protothread_debug);
    pt_add_thread(protothread_graphics);

    pt_schedule_start ;

}


int main() {

    // overclock
    set_sys_clock_khz(160000, true) ;   

    stdio_init_all();
    sleep_ms(100);
    printf("Starting program...\n");

    // Start core 1
    multicore_launch_core1(core1_main);

    // Bluetooth runs on core 0 (this never returns)
    bluetooth_init();

    return 0;
}