
/*
    PicoWiimote
    ECE 4760 - Digital Systems Design Using Microcontrollers
    Emith Uyanwatte, Farhan Naqib

    This code establises a connection between a Wiimote and Pi Pico W, enables the IR camera,
    and prints out all of the recieved information through serial.

    Core 0 is what actually performs the Bluetooth connection, core 1 prints to serial. It does
    not have to be this way; we did it like this because this code is part of a bigger project
    (check the other branch on Github) for which splitting into 2 cores made development more
    convinient.
*/

#include <btstack_util.h>
#include <pico/stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "pico/multicore.h"
#include "string.h"
#include "hardware/sync.h"
#include "hardware/timer.h"
#include "pico/multicore.h"
#include "hardware/pio.h"
#include "hardware/dma.h"
#include "hardware/vreg.h"
#include "hardware/clocks.h"

#include "wiimote_bt.h"
#include "controls.h"

#define ANSI_CLEAR_LINE "\033[K"
#define ANSI_HIDE_CURSOR "\033[?25l"
#define ANSI_SHOW_CURSOR "\033[?25h"
#define ANSI_GREEN       "\033[32m"
#define ANSI_RESET       "\033[0m"

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

static void PRINT_SERIAL ()
{

    // Hide cursor for a cleaner look
    printf(ANSI_HIDE_CURSOR);
    fflush(stdout);

    // Add gap between bt serial and this serial
    printf("\n");

    // Reserve lines for this
    printf("------ CONTROLLER OUTPUT ------\n\n\n\n\n\n\n\n");
    fflush(stdout);

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

    }

    // Should never get here, inf loop!!!
    printf(ANSI_SHOW_CURSOR);
    fflush(stdout);

} // Serial debug thread


void core1_main(void) {

    // Small delay so core 0 can finish init + BT logs
    sleep_ms(5000);

    PRINT_SERIAL();
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

    printf("TEST");

    return 0;
}