#include "drawing.h"
#include "vga16_graphics_v2.h"
#include "controls.h"
#include "wiimote_pointer.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static bool init_check_draw = false;

// Canvas size in pixels
#define CANVAS_W 320
#define CANVAS_H 240
#define CANVAS_X 160    // centered area
#define CANVAS_Y  80

#define TILE_W 8
#define TILE_H 8

#define TILES_X (CANVAS_W / TILE_W)
#define TILES_Y (CANVAS_H / TILE_H)

static uint8_t *tile_fb = NULL;

#define PALETTE_X     20
#define PALETTE_Y     20
#define PALETTE_SIZE  20
#define PALETTE_GAP    4
#define BRUSH_R        0    

static char selected_color = WHITE;
static bool erase_mode = false;

// 16 VGA colors
static const char palette_colors[16] = {
    BLACK, DARK_GREEN, MED_GREEN, GREEN,
    DARK_BLUE, BLUE, LIGHT_BLUE, CYAN,
    RED, DARK_ORANGE, ORANGE, YELLOW,
    MAGENTA, PINK, LIGHT_PINK, WHITE
};

static void center_text(const char *msg, int y, char color)
{
    setTextColor(color);
    setTextSize(1);
    int w = strlen(msg) * 6;
    int x = (640 - w) / 2;
    setCursor(x, y);
    writeString((char*)msg);
}


static void draw_palette()
{
    for (int i = 0; i < 16; i++) {
        int y = PALETTE_Y + i * (PALETTE_SIZE + PALETTE_GAP);
        fillRect(PALETTE_X, y, PALETTE_SIZE, PALETTE_SIZE, palette_colors[i]);

        // Selection box
        if (!erase_mode && palette_colors[i] == selected_color) {
            drawRect(PALETTE_X - 2, y - 2, PALETTE_SIZE + 4, PALETTE_SIZE + 4, WHITE);
        }
    }

    // Eraser box
    int er_y = PALETTE_Y + 16 * (PALETTE_SIZE + PALETTE_GAP);
    fillRect(PALETTE_X, er_y, PALETTE_SIZE, PALETTE_SIZE, BLACK);
    drawChar(PALETTE_X + 6, er_y + 5, 'E', WHITE, BLACK, 1);

    if (erase_mode) {
        drawRect(PALETTE_X - 2, er_y - 2, PALETTE_SIZE + 4, PALETTE_SIZE + 4, WHITE);
    }
}

static void draw_tile_canvas()
{
    for (int ty = 0; ty < TILES_Y; ty++) {
        for (int tx = 0; tx < TILES_X; tx++) {

            uint8_t col = tile_fb[ty * TILES_X + tx];

            int sx = CANVAS_X + tx * TILE_W;
            int sy = CANVAS_Y + ty * TILE_H;

            fillRect(sx, sy, TILE_W, TILE_H, col);
        }
    }
}

void init_objs_drawing()
{
    bg_color = BLACK;
    clearLowFrame(0, bg_color);

    erase_mode = false;
    selected_color = WHITE;
    wiimote_pt_color = WHITE;

    tile_fb = malloc(TILES_X * TILES_Y);
    if (!tile_fb) return; // fail-safe

    // Set canvas to black
    for (int i = 0; i < TILES_X * TILES_Y; i++)
        tile_fb[i] = bg_color;

    init_check_draw = true;
}

void draw_drawing()
{
    if (!init_check_draw)
        init_objs_drawing();

    if (!tile_fb) return;

    // Clear UI area
    clearRect(0, 0, 150, 479, bg_color);
    clearRect(0, 430, 639, 479, bg_color);

    // Draw UI
    draw_palette();
    center_text("A = Draw    B/A+Eraser = Erase    HOME = Exit", 450, WHITE);

    // PALETTE SELECTION
    if (button_a) {
        int px = pointer_x, py = pointer_y;

        for (int i = 0; i < 16; i++) {
            int y = PALETTE_Y + i * (PALETTE_SIZE + PALETTE_GAP);

            if (px >= PALETTE_X && px <= PALETTE_X + PALETTE_SIZE &&
                py >= y         && py <= y + PALETTE_SIZE)
            {
                selected_color = palette_colors[i];
                erase_mode = false;
            }
        }

        // Eraser
        int er_y = PALETTE_Y + 16 * (PALETTE_SIZE + PALETTE_GAP);
        if (px >= PALETTE_X && px <= PALETTE_X + PALETTE_SIZE &&
            py >= er_y      && py <= er_y + PALETTE_SIZE)
        {
            erase_mode = true;
        }
    }

    if (button_a || button_b) {

        int px = pointer_x - CANVAS_X;
        int py = pointer_y - CANVAS_Y;

        int tx = px / TILE_W;
        int ty = py / TILE_H;

        if (tx >= 0 && tx < TILES_X &&
            ty >= 0 && ty < TILES_Y)
        {
            tile_fb[ty * TILES_X + tx] =
                (erase_mode || button_b) ? bg_color : selected_color;
        }
    }

    draw_tile_canvas();

    if (button_home) {
        if (tile_fb) free(tile_fb);
        tile_fb = NULL;

        kill_wiimote_objs(draggable_objs, clickable_objs);
        init_check_draw = false;

        current_game = GAMES_MAIN_MENU;
        draw_curr_app();
        return;
    }
}
