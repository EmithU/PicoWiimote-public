# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
set(PICO_BOARD pico_w)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(pico_sdk_import.cmake)

project(PicoWiimoteClassic C CXX ASM)
pico_sdk_init()

# ----------------------------------------------------------
#  Sources
# ----------------------------------------------------------
add_executable(PicoWiimoteClassic
    main.c
    src/wiimote_bt/wiimote_bt.c
    src/controls/controls.c
    src/vga/vga16_graphics_v2.c
    src/vga/vga16_graphics_3D_surface.c
    src/pointer/wiimote_pointer.c

    src/game_base/game_primitives/game_primitives.c
    src/game_base/app_state/app_state.c
    src/game_base/drawing_logic_2d/drawing_logic_2d.c

    src/apps/main_menu/main_menu.c

    src/apps/color_screen/color_screen.c
    src/apps/3d_demo/3d_demo.c

    src/apps/reaction/reaction.c
    src/apps/3d_demo_2/3d_demo_2.c
    src/apps/drawing/drawing.c
    src/apps/spotlight/spotlight.c
    src/apps/shovelware/shovelware.c
    src/apps/shovelware2/shovelware2.c
    src/apps/stats/stats.c
)

# ----------------------------------------------------------
#  Include directories
#   (btstack include paths are added automatically by the
#    pico_btstack_* libraries; we do NOT add them manually)
# ----------------------------------------------------------
target_include_directories(PicoWiimoteClassic PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}  
    src/wiimote_bt
    src/controls
    src/vga
    src/pointer

    src/game_base
    src/game_base/game_primitives
    src/game_base/app_state
    src/game_base/drawing_logic_2d


    src/apps
    src/apps/color_screen
    src/apps/main_menu
    src/apps/3d_demo
    src/apps/3d_demo_2
    src/apps/drawing
    src/apps/stats
    src/apps/shovelware
    src/apps/shovelware2
    src/apps/spotlight
    src/apps/reaction

    include                     
)

pico_generate_pio_header(PicoWiimoteClassic ${CMAKE_CURRENT_LIST_DIR}/hsync.pio)
pico_generate_pio_header(PicoWiimoteClassic ${CMAKE_CURRENT_LIST_DIR}/vsync.pio)
pico_generate_pio_header(PicoWiimoteClassic ${CMAKE_CURRENT_LIST_DIR}/rgb.pio)

# ----------------------------------------------------------
#  Program metadata
# ----------------------------------------------------------
pico_set_program_name(PicoWiimoteClassic "PicoWiimoteClassic")
pico_set_program_version(PicoWiimoteClassic "0.1")

# UART/USB output
pico_enable_stdio_uart(PicoWiimoteClassic 1)
pico_enable_stdio_usb(PicoWiimoteClassic 0)

# ----------------------------------------------------------
#  Link with Pico + BTstack libs
# ----------------------------------------------------------
target_link_libraries(PicoWiimoteClassic
    pico_stdlib
    pico_bootsel_via_double_reset
    pico_multicore

    pico_btstack_classic
    pico_btstack_cyw43
    pico_cyw43_arch_none

    pico_divider 
    pico_rand 
    
    hardware_pio 
    hardware_dma 
    hardware_adc 
    hardware_irq 
    hardware_clocks 
    hardware_pll 
    hardware_spi 
    hardware_adc
)

target_compile_definitions(PicoWiimoteClassic PRIVATE CYW43_LWIP=0)

pico_add_extra_outputs(PicoWiimoteClassic)
