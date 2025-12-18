/*
 * Copyright (C) 2017 BlueKitchen GmbH
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holders nor the names of
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 * 4. Any redistribution, use, or modification is done solely for
 *    personal benefit and not for any commercial purpose or for
 *    monetary gain.
 *
 * THIS SOFTWARE IS PROVIDED BY BLUEKITCHEN GMBH AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL BLUEKITCHEN
 * GMBH OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * Please inquire about commercial licensing options at 
 * contact@bluekitchen-gmbh.com
 *
 */

#include <stdbool.h>
#include <stdint.h>
#define BTSTACK_FILE__ "wiimote_bt.c"

#include "wiimote_bt.h"
#include "controls.h"

/*
 * bluetooth.c
 */

/* EXAMPLE_START(hid_host_demo): HID Host Classic
 *
 * @text This example implements a HID Host. For now, it connects to a fixed device.
 * It will connect in Report protocol mode if this mode is supported by the HID Device,
 * otherwise it will fall back to BOOT protocol mode. 
 */

#include <inttypes.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "pico/stdlib.h"

#include "btstack_config.h"
#include "btstack.h"
#include "btstack_util.h"

#include "pico/cyw43_arch.h"
#include "pico/async_context.h"
#include "pico/btstack_cyw43.h"



const btstack_run_loop_t *btstack_run_loop_async_context_get_instance(async_context_t *context);
const btstack_chipset_t  *btstack_chipset_cyw43_instance(void);
const hci_transport_t    *hci_transport_cyw43_instance(void);

static uint8_t wiimote_rumble = 0;  // 0 or 1
static uint8_t led = 0x10; // LED1

#define MAX_ATTRIBUTE_VALUE_SIZE 300

// HARDCODED WIIMOTE BLUETOOTH ADDRESS
// Could theoretically have the Pico search for any Wiimotes, but I think it's unnecessary here...
static const char * remote_addr_string = "00:1F:32:91:83:70";

static bd_addr_t remote_addr;

static btstack_packet_callback_registration_t hci_event_callback_registration;

// SDP
static uint8_t hid_descriptor_storage[MAX_ATTRIBUTE_VALUE_SIZE];

// App
static enum {
    APP_IDLE,
    APP_CONNECTED
} app_state = APP_IDLE;

static uint16_t hid_host_cid = 0;
static bool     hid_host_descriptor_available = false;
static hid_protocol_mode_t hid_host_report_mode = HID_PROTOCOL_MODE_REPORT;

typedef enum {
    INIT_IR13,
    INIT_IR1A,
    INIT_IR_LOGIC1,
    INIT_SENS1,
    INIT_SENS2,
    INIT_MODE,
    INIT_IR_LOGIC2,
    INIT_SET_REPORT_MODE,
    INIT_SET_LEDS,
    INIT_DONE
} init_state_t;

static init_state_t init_state = INIT_IR13;
static btstack_timer_source_t init_timer;

static void wiimote_send_report(uint16_t hid_cid, uint16_t report_id, uint8_t *report, uint8_t report_len) {
    report[0] |= wiimote_rumble;
    hid_host_send_report(hid_cid, report_id, report, report_len);
};


/* @section Main application configuration
 *
 * @text In the application configuration, L2CAP and HID host are initialized, and the link policies 
 * are set to allow sniff mode and role change. 
 */

/* LISTING_START(PanuSetup): Panu setup */
static void packet_handler (uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size);

/*
    When writing to registers, we need to write through report ID 0x16.

    WRITE MESSAGE LOOKS LIKE THE FOLLOWING:
    (a2) 16 MM FF FF FF SS DD ...
     - a2: Indicates output report - BTStack appends for us
     - 16: Indicates type of output report - BTStack appends for us
     - MM: Address space selector. Clear to write to EEPROM. Also include rumble flag
        (for now, going to assume no rumble for simplicity, so rumble = 0)
     - FF FF FF: Memory offset
     - SS: Size of payload
     - DD ...: Up to 16 bytes of data. If less than 16 is used, pad with 0's to 16.
*/
static void wiimote_write_reg(uint32_t addr, const uint8_t *data, uint8_t len)
{
    uint8_t buf[21] = {0};

    // MM byte -- 0x04
    buf[0] = 0x04;

    // 3 byte address
    buf[1] = (addr >> 16) & 0xFF;
    buf[2] = (addr >> 8)  & 0xFF;
    buf[3] = (addr >> 0)  & 0xFF;

    // Length
    buf[4] = len;

    // Data
    memcpy(&buf[5], data, len);

    // Print (for debugging)
    for (int i = 0; i < 21; i++) {
        printf("%02x ", buf[i]);
    }

    wiimote_send_report(hid_host_cid, 0x16, buf, 21);
}


/*
    State machine to control what phase of initialization the Wiimote IR is in.
    We need to ensure that we go to the next step when the previous step is done, so we
    model with a state machine and use a scheduler to implement it. 
*/
static void wiimote_init_step(btstack_timer_source_t *ts) {

    switch (init_state) {

        // 1. Enable IR Camera (Send 0x04 to Output Report 0x13)
        case INIT_IR13: {
            printf("\n=== WIIMOTE INIT START ===\n");
            uint8_t v = 0x04;
            wiimote_send_report(hid_host_cid, 0x13, &v, 1);
            init_state = INIT_IR1A;
        } break;

        // 2. Enable IR Camera 2 (Send 0x04 to Output Report 0x1a)
        case INIT_IR1A: {
            uint8_t v = 0x04;
            wiimote_send_report(hid_host_cid, 0x1A, &v, 1);
            init_state = INIT_IR_LOGIC1;
        } break;

        // 3. Write 0x08 to register 0xb00030
        case INIT_IR_LOGIC1: {
            uint8_t v = 0x08;
            wiimote_write_reg(0xB00030, &v, 1);
            init_state = INIT_SENS1;
        } break;

        // 4. Write Sensitivity Block 1 to registers at 0xb00000
        // Assumes Wii level 3 sensitivity (should adjust in lab)
        case INIT_SENS1: {
            const uint8_t s1[9] = {0x02,0x00,0x00,0x71,0x01,0x00,0xAA,0x00,0x64};
            wiimote_write_reg(0xB00000, s1, 9);
            init_state = INIT_SENS2;
        } break;

        // 5. Write Sensitivity Block 2 to registers at 0xb0001a
        // Assumes Wii level 3 sensitivity (should adjust in lab)
        case INIT_SENS2: {
            const uint8_t s2[2] = {0x63,0x03};
            wiimote_write_reg(0xB0001A, s2, 2);
            init_state = INIT_MODE;
        } break;

        // 6. Write Mode Number to register 0xb00033
        // Can technically be adjusted but I think we should just keep the reporting
        // mode of the wiimote constant, seems like a pain to do anything else...
        case INIT_MODE: {
            uint8_t v = 0x03;
            wiimote_write_reg(0xB00033, &v, 1);
            init_state = INIT_IR_LOGIC2;
        } break;

        // 7. Write 0x08 to register 0xb00030 (again)
        case INIT_IR_LOGIC2: {
            uint8_t v = 0x08;
            wiimote_write_reg(0xB00030, &v, 1);
            init_state = INIT_SET_REPORT_MODE;
        } break;

        // AT THIS POINT, IR CONFIG IS DONE

        // Sets report mode to 0x33 (Core + accel + 12 IR)
        case INIT_SET_REPORT_MODE: {
            uint8_t modecmd[2] = {0x00, 0x33};
            wiimote_send_report(hid_host_cid, 0x12, modecmd, 2);
            init_state = INIT_SET_LEDS;
        } break;

        // Toggles on LED 1
        case INIT_SET_LEDS: {
            wiimote_send_report(hid_host_cid, 0x11, &led, 1);
            init_state = INIT_DONE;
        } break;

        // Done + leave scheduler
        case INIT_DONE:
            printf("\n=== WIIMOTE INIT COMPLETE ===\n");
            return;  // stop scheduling
    }

    // schedule next step in 50ms
    btstack_run_loop_set_timer(ts, 50);
    btstack_run_loop_add_timer(ts);
}


static void hid_host_setup(void){

    printf("Attempting to run hid_host_setup\n");

    // Initialize L2CAP
    printf("Attempting l2cap_init\n");
    l2cap_init();

#ifdef ENABLE_BLE
    // Initialize LE Security Manager. Needed for cross-transport key derivation
    sm_init();
#endif

    // Initialize HID Host
    printf("Attempting hid_host_init\n");
    hid_host_init(hid_descriptor_storage, sizeof(hid_descriptor_storage));
    hid_host_register_packet_handler(packet_handler);
    printf("Done with hid_host_init\n");

    // Allow sniff mode requests by HID device and support role switch
    gap_set_default_link_policy_settings(LM_LINK_POLICY_ENABLE_SNIFF_MODE | LM_LINK_POLICY_ENABLE_ROLE_SWITCH);

    // try to become master on incoming connections
    hci_set_master_slave_policy(HCI_ROLE_MASTER);

    // register for HCI events
    hci_event_callback_registration.callback = &packet_handler;
    hci_add_event_handler(&hci_event_callback_registration);

    // make discoverable to allow HID device to initiate connection
    gap_discoverable_control(1);

    // Disable stdout buffering
    setvbuf(stdin, NULL, _IONBF, 0);
}
/* LISTING_END */

/*
 * @section HID Report Handler
 * 
 * @text Use BTstack's compact HID Parser to process incoming HID Report in Report protocol mode. 
 * Iterate over all fields and process fields with usage page = 0x07 / Keyboard
 * Check if SHIFT is down and process first character (don't handle multiple key presses)
 * 
 */

#define NUM_KEYS 6
static uint8_t last_keys[NUM_KEYS];
static bool hid_host_caps_lock;

static uint16_t hid_host_led_report_id;
static uint8_t  hid_host_led_report_len;
static uint8_t  hid_host_led_caps_lock_bit;

static void hid_host_set_leds(void){
    if (hid_host_led_report_len == 0) return;

    uint8_t output_report[8];

    uint8_t caps_lock_report_offset = hid_host_led_caps_lock_bit >> 3;
    if (caps_lock_report_offset >= sizeof(output_report)) return;

    memset(output_report, 0, sizeof(output_report));
    if (hid_host_caps_lock){
        output_report[caps_lock_report_offset] = 1 << (hid_host_led_caps_lock_bit & 0x07);
    }
    wiimote_send_report(hid_host_cid, hid_host_led_report_id, output_report, hid_host_led_report_len);
}

static void hid_host_demo_lookup_caps_lock_led(void){
    btstack_hid_usage_iterator_t iterator;
    const uint8_t *hid_descriptor = hid_descriptor_storage_get_descriptor_data(hid_host_cid);
    const uint16_t hid_descriptor_len = hid_descriptor_storage_get_descriptor_len(hid_host_cid);
    btstack_hid_usage_iterator_init(&iterator, hid_descriptor, hid_descriptor_len, HID_REPORT_TYPE_OUTPUT);
    while (btstack_hid_usage_iterator_has_more(&iterator)){
        btstack_hid_usage_item_t item;
        btstack_hid_usage_iterator_get_item(&iterator, &item);
        if (item.usage_page == HID_USAGE_PAGE_LED && item.usage == HID_USAGE_LED_CAPS_LOCK){
            hid_host_led_report_id     = item.report_id;
            hid_host_led_report_len    = btstack_hid_get_report_size_for_id(hid_host_led_report_id, HID_REPORT_TYPE_OUTPUT, hid_descriptor, hid_descriptor_len);
            hid_host_led_caps_lock_bit = (uint8_t) item.bit_pos;
            printf("Found CAPS LOCK in Output Report with ID 0x%04x at bit %3u\n", hid_host_led_report_id, hid_host_led_caps_lock_bit);
        }
    }
}

/*
    MESSAGE INTERRUPT HANDLER

    MESSAGE FORMAT
    (a1) 33 BB BB AA AA AA II II II II II II II II II II II II 

*/
static void hid_host_handle_interrupt_report(const uint8_t * report, uint16_t report_len){

    // Debug - uncomment to print all messages to serial
    // printf_hexdump(report, report_len);

    // Wiimote gives all types of reports. At this point, we know that the init works so we
    // only care about actual controls reports.
    if (report[1] == 0x33) {

        // We ignore the first 2 bytes of the payload
        uint8_t buttons[2] = { report[2], report[3] };
        uint8_t accel[3] = { report[4], report[5], report[6] };

        // Core Button Decoding - they are buttons, so either 0 or 1. Wiimote auto-applies debouncing logic I think.
        dpad_left         = (buttons[0] & 0x01) != 0;
        dpad_right        = (buttons[0] & 0x02) != 0;
        dpad_up           = (buttons[0] & 0x08) != 0;
        dpad_down         = (buttons[0] & 0x04) != 0;
        button_plus       = (buttons[0] & 0x10) != 0;
        button_minus      = (buttons[1] & 0x10) != 0;
        button_one        = (buttons[1] & 0x02) != 0;
        button_two        = (buttons[1] & 0x01) != 0;
        button_a          = (buttons[1] & 0x08) != 0;
        button_b          = (buttons[1] & 0x04) != 0;
        button_home       = (buttons[1] & 0x80) != 0;

        // Accel Decoding
        raw_x = (signed) (accel[0] - 0x80) << 2 | ((buttons[0] & 0x60) >> 5);        // 2 lsbs -> buttons[1] bit 6 | bit 5
        raw_y = (signed) (accel[1] - 0x80) << 2 | ((buttons[1] & 0x40) >> 4);        // 2 lsbs -> buttons[1] bit 6 | 0
        raw_z = (signed) (accel[2] - 0x80) << 2 | ((buttons[1] & 0x20) >> 5);        // 2 lsbs -> buttons[1] bit 5 | 0

        // IR Decoding
        blob_1 = (ir_blob_t){ report[7] | ((short) report[9] & 0x30) << 4,  
                              report[8] | ((short) report[9] & 0xC0) << 2, 
                              report[9] & 0x0F };
        blob_2 = (ir_blob_t){ report[10] | ((short) report[12] & 0x30) << 4,  
                              report[11] | ((short) report[12] & 0xC0) << 2, 
                              report[12] & 0x0F };
        blob_3 = (ir_blob_t){ report[13] | ((short) report[15] & 0x30) << 4,  
                              report[14] | ((short) report[15] & 0xC0) << 2, 
                              report[15] & 0x0F };
        blob_4 = (ir_blob_t){ report[16] | ((short) report[18] & 0x30) << 4,  
                              report[17] | ((short) report[18] & 0xC0) << 2, 
                              report[18] & 0x0F };
    }
}

/*
 * @section Packet Handler
 * 
 * @text The packet handler responds to various HID events.
 */

/* LISTING_START(packetHandler): Packet Handler */
static void packet_handler (uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size)
{
    /* LISTING_PAUSE */
    UNUSED(channel);
    UNUSED(size);

    uint8_t   event;
    bd_addr_t event_addr;
    uint8_t   status;

    /* LISTING_RESUME */
    switch (packet_type) {
		case HCI_EVENT_PACKET:
            event = hci_event_packet_get_type(packet);
            
            switch (event) {            
#ifndef HAVE_BTSTACK_STDIN
                /* @text When BTSTACK_EVENT_STATE with state HCI_STATE_WORKING
                 * is received and the example is started in client mode, the remote SDP HID query is started.
                 */
                case BTSTACK_EVENT_STATE:
                    if (btstack_event_state_get_state(packet) == HCI_STATE_WORKING){
                        status = hid_host_connect(remote_addr, hid_host_report_mode, &hid_host_cid);
                        if (status != ERROR_CODE_SUCCESS){
                            printf("HID host connect failed, status 0x%02x.\n", status);
                        }
                    }
                    break;
#endif
                /* LISTING_PAUSE */
                case HCI_EVENT_PIN_CODE_REQUEST:
                    bd_addr_t addr;
                    hci_event_pin_code_request_get_bd_addr(packet, addr);
                    printf("PIN code request from %s\n", bd_addr_to_str(addr));

                    // Wiimote pairing in 1+2 mode:
                    // PIN = Wiimote BD_ADDR bytes reversed, 6 bytes, binary.
                    uint8_t pin[6];
                    pin[0] = addr[5];
                    pin[1] = addr[4];
                    pin[2] = addr[3];
                    pin[3] = addr[2];
                    pin[4] = addr[1];
                    pin[5] = addr[0];

                    gap_pin_code_response_binary(addr, pin, sizeof(pin));
                    printf("Responded with Wiimote PIN (reversed BDADDR)\n");
                    break;

                case HCI_EVENT_USER_CONFIRMATION_REQUEST:
                    // inform about user confirmation request
                    printf("SSP User Confirmation Request with numeric value '%"PRIu32"'\n", little_endian_read_32(packet, 8));
                    printf("SSP User Confirmation Auto accept\n");
                    break;

                /* LISTING_RESUME */
                case HCI_EVENT_HID_META:
                    switch (hci_event_hid_meta_get_subevent_code(packet)){

                        case HID_SUBEVENT_INCOMING_CONNECTION:
                            // There is an incoming connection: we can accept it or decline it

                            // If there are no resources to handle the connection,
                            // the status will be ERROR_CODE_MEMORY_CAPACITY_EXCEEDED

                            // The hid_host_report_mode in the hid_host_accept_connection function
                            // allows the application to request a protocol mode.

                            // For available protocol modes, see hid_protocol_mode_t in btstack_hid.h file.

                            /*
                            if (hid_subevent_incoming_connection_get_handle(packet) == ERROR_CODE_SUCCESS) {
                                hid_host_accept_connection(hid_subevent_incoming_connection_get_hid_cid(packet), hid_host_report_mode);
                            }
                            break;
                            */

                            uint16_t cid = hid_subevent_incoming_connection_get_hid_cid(packet);
                            printf("Incoming HID connection, cid=%u\n", cid);

                            printf("Accepting incoming HID connection\n");
                            hid_host_accept_connection(cid, hid_host_report_mode);
                            break;
                        
                        case HID_SUBEVENT_CONNECTION_OPENED:
                            // The status field of this event indicates if the control and interrupt
                            // connections were opened successfully.

                            status = hid_subevent_connection_opened_get_status(packet);
                            if (status != ERROR_CODE_SUCCESS) {
                                printf("Connection failed, status 0x%02x\n", status);
                                app_state = APP_IDLE;
                                hid_host_cid = 0;
                                return;
                            }
                            app_state = APP_CONNECTED;
                            hid_host_descriptor_available = false;
                            hid_host_cid = hid_subevent_connection_opened_get_hid_cid(packet);
                            hid_host_caps_lock = false;
                            hid_host_led_report_len = 0;
                            printf("HID Host connected.\n");

                            break;

                        case HID_SUBEVENT_DESCRIPTOR_AVAILABLE:
                            // This event will follows HID_SUBEVENT_CONNECTION_OPENED event. 
                            // For incoming connections, i.e. HID Device initiating the connection,
                            // the HID_SUBEVENT_DESCRIPTOR_AVAILABLE is delayed, and some HID  
                            // reports may be received via HID_SUBEVENT_REPORT event. It is up to 
                            // the application if these reports should be buffered or ignored until 
                            // the HID descriptor is available.
                            status = hid_subevent_descriptor_available_get_status(packet);
                            if (status == ERROR_CODE_SUCCESS){
                                hid_host_descriptor_available = true;
                                printf("HID Descriptor available.\n");

                                // Declare first state
                                init_state = INIT_IR13;

                                // Init btstack timer
                                btstack_run_loop_set_timer(&init_timer, 50);

                                // Declares the state machine
                                init_timer.process = wiimote_init_step;

                                // Starts it all
                                btstack_run_loop_add_timer(&init_timer);

                                hid_host_demo_lookup_caps_lock_led();
                            } else {
                                printf("Cannot handle input report, HID Descriptor is not available, status 0x%02x\n", status);
                            }
                            break;

                        case HID_SUBEVENT_REPORT:
                            // Handle input report.
                            if (hid_host_descriptor_available){
                                hid_host_handle_interrupt_report(hid_subevent_report_get_report(packet), hid_subevent_report_get_report_len(packet));
                            } else {
                                printf_hexdump(hid_subevent_report_get_report(packet), hid_subevent_report_get_report_len(packet));
                            }
                            break;

                        case HID_SUBEVENT_SET_PROTOCOL_RESPONSE:
                            // For incoming connections, the library will set the protocol mode of the
                            // HID Device as requested in the call to hid_host_accept_connection. The event 
                            // reports the result. For connections initiated by calling hid_host_connect, 
                            // this event will occur only if the established report mode is boot mode.
                            status = hid_subevent_set_protocol_response_get_handshake_status(packet);
                            if (status != HID_HANDSHAKE_PARAM_TYPE_SUCCESSFUL){
                                printf("Error set protocol, status 0x%02x\n", status);
                                break;
                            }
                            switch ((hid_protocol_mode_t)hid_subevent_set_protocol_response_get_protocol_mode(packet)){
                                case HID_PROTOCOL_MODE_BOOT:
                                    printf("Protocol mode set: BOOT.\n");
                                    break;  
                                case HID_PROTOCOL_MODE_REPORT:
                                    printf("Protocol mode set: REPORT.\n");
                                    break;
                                default:
                                    printf("Unknown protocol mode.\n");
                                    break; 
                            }
                            break;

                        case HID_SUBEVENT_CONNECTION_CLOSED:
                            // The connection was closed.
                            hid_host_cid = 0;
                            hid_host_descriptor_available = false;
                            printf("HID Host disconnected.\n");
                            break;
                        
                        default:
                            break;
                    }
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }
}

int btstack_main(int argc, const char * argv[]){
    (void)argc;
    (void)argv;

    hid_host_setup();

    // parse human readable Bluetooth address
    printf("Scanning for address... \n\r");
    sscanf_bd_addr(remote_addr_string, remote_addr);

#ifdef HAVE_BTSTACK_STDIN
    btstack_stdin_setup(stdin_process);
#endif

    // Turn on the device
    hci_power_control(HCI_POWER_ON);
    
    return 0;
}

void bluetooth_init(void) {

    // Bring up CYW43 WiFi/Bluetooth subsystem
    if (cyw43_arch_init()) {
        printf("CYW43 init failed\n");
        return;
    }

    // Set up BTstack memory pools
    btstack_memory_init();

    // Use Pico's async-context run loop integration for BTstack
    async_context_t *ctx = cyw43_arch_async_context();
    const btstack_run_loop_t *run_loop =
        btstack_run_loop_async_context_get_instance(ctx);
    btstack_run_loop_init(run_loop);

    // Initialize HCI for CYW43 using the Pico BTstack wrapper
    hci_init(hci_transport_cyw43_instance(), NULL);
    hci_set_chipset(btstack_chipset_cyw43_instance());

    // Call original btstack_main()
    btstack_main(0, NULL);

    // Hand control to BTstack (does not return)
    btstack_run_loop_execute();
}

void bluetooth_rumble(bool rumble) {
    wiimote_rumble = rumble;
    uint8_t v = 0x00;
    wiimote_send_report(hid_host_cid, 0x10, &v, 1);
}
