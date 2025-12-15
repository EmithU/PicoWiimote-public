#include "color_screen.h"
#include "game_primitives.h"
#include "vga16_graphics_v2.h"
#include <stdbool.h>

#include "wiimote_bt.h"
#include "controls.h"
#include "app_state.h"
#include "wiimote_pointer.h"

#include "vga16_graphics_3D_surface.h"

#include <malloc.h>

// Create int variables for any Wiimote objs at the top of the file

static vector temp1, temp2, temp3 ;

static matrix TrotZ, Ttrans, TrotY, Tscale_mesh ;
static int t, last_vsync=1, vsync;
static long long t_start ;
//
static int mesh_size = 10 ;
// vector for backface culling
static vector Nview = {0,0,1};
// edges of faces
static short line_color = WHITE ;

static bool init_check_3d_demo_2 = false;

static object wall_left;
static object wall_right;
static object wall_top;
static object wall_bottom;

static object mesh;

void init_objs_3d_demo_2() {

    init3D();

    build_Mesh(&mesh, mesh_size, YELLOW, ORANGE) ;

    matrix T;

    // ===== LEFT WALL =====
    build_Mesh(&wall_left, mesh_size, GREEN, GREEN);

    build_Scale(float_to_s15x16(4.0), float_to_s15x16(2.0), float_to_s15x16(1.0), &T);
    for (int i = 0; i < wall_left.N_vertex; i++)
        VxM(&wall_left.vertex[i], &T, &wall_left.vertex[i]);

    build_RotX(90, &T);
    for (int i = 0; i < wall_left.N_vertex; i++)
        VxM(&wall_left.vertex[i], &T, &wall_left.vertex[i]);

    build_Translate(float_to_s15x16(1), float_to_s15x16(-1), float_to_s15x16(-1.0), &T);
    for (int i = 0; i < wall_left.N_vertex; i++)
        VxM(&wall_left.vertex[i], &T, &wall_left.vertex[i]);


    // ===== RIGHT WALL =====
    build_Mesh(&wall_right, mesh_size, BLUE, BLUE);

    build_Scale(float_to_s15x16(4.0), float_to_s15x16(2.0), float_to_s15x16(1.0), &T);
    for (int i = 0; i < wall_right.N_vertex; i++)
        VxM(&wall_right.vertex[i], &T, &wall_right.vertex[i]);

    build_RotX(90, &T);
    for (int i = 0; i < wall_right.N_vertex; i++)
        VxM(&wall_right.vertex[i], &T, &wall_right.vertex[i]);

    build_Translate(float_to_s15x16(0), float_to_s15x16(2.0), float_to_s15x16(-1.0), &T);
    for (int i = 0; i < wall_right.N_vertex; i++)
        VxM(&wall_right.vertex[i], &T, &wall_right.vertex[i]);


    // ===== TOP WALL =====
    build_Mesh(&wall_top, mesh_size, YELLOW, YELLOW);

    build_Scale(float_to_s15x16(4.0), float_to_s15x16(2.0), float_to_s15x16(1.0), &T);
    for (int i = 0; i < wall_top.N_vertex; i++)
        VxM(&wall_top.vertex[i], &T, &wall_top.vertex[i]);

    build_RotX(180, &T);
    for (int i = 0; i < wall_top.N_vertex; i++)
        VxM(&wall_top.vertex[i], &T, &wall_top.vertex[i]);

    build_Translate(float_to_s15x16(0), float_to_s15x16(2.0), float_to_s15x16(-2.0), &T);
    for (int i = 0; i < wall_top.N_vertex; i++)
        VxM(&wall_top.vertex[i], &T, &wall_top.vertex[i]);


    // move stuff
    build_Translate (float_to_s15x16(-0.5), float_to_s15x16(-0.5), float_to_s15x16(-0.5), &Ttrans);
    build_Scale(float_to_s15x16(2.0), float_to_s15x16(2.0), float_to_s15x16(2.0), &Tscale_mesh) ;
    // position the mesh
    for (int i=0; i<mesh.N_vertex; i++){
      //VxM(&mesh.vertex[i], &Ttrans, &mesh.vertex[i]); 
      VxM(&mesh.vertex[i], &Tscale_mesh, &mesh.vertex[i]); 
    }

    bg_color = BLACK;
    wiimote_pt_color = WHITE;
    clearLowFrame(0, bg_color);

}

void draw_3d_demo_2() {

    // Init objs only on first run.
    if (!init_check_3d_demo_2) {
        init_objs_3d_demo_2();
        init_check_3d_demo_2 = true;
    }
    
    ///
    //
    setCursor(270, 20) ;
    setTextColor2(WHITE, BLACK) ;
    writeStringBig("3D Render Test 2.0") ;
    setCursor(270, 37) ;

    t = (t + 1) % 1000;

    matrix Tcamera ;
    matrix Tview ;
    matrix Tpersp ;
    
    // === define the camera ======
    // 'from' is camera location in model sapace
    // 'to' is where the camera is pointed
    vector from = {float_to_s15x16(4.0), float_to_s15x16(1.0), -float_to_s15x16(0.5)} ;
    vector to = {float_to_s15x16(0. - (pointer_x/200.0)), float_to_s15x16(0.), float_to_s15x16(pointer_y/200.0)} ;
    // guess a good up-vector (will be corrected when the view is consructed)
    vector approxUp = {0,0,one};
    // matrix to traansform to camera frame
    build_Tview(&from, &to, &approxUp, &Tview) ;

    // === perpective transform ===
    s15x16 d=float_to_s15x16(1.5), h=float_to_s15x16(1.5), f=20*one ; //0x0fff000 ;
    build_Tpersp( d, h, f, &Tpersp);
    // combine view and perspective into camera
    MxM(&Tview, &Tpersp, &Tcamera) ;

    // init the view list
    view_init();

    // === rotate thru 360 degrees ==
    build_RotZ(t % 360, & TrotZ);
    build_RotY(2 * t % 360, & TrotY);

    // mesh deform funciton
    for (int i = 0; i < mesh_size; i++) {
        for (int j = 0; j < mesh_size; j++) {
            mesh.vertex[i + mesh_size * j].z = muls15x16(cosine[36 * j % 360], muls15x16(cosine[36 * i % 360], cosine[(4 * t) % 360] >> 2));
        }
    }


    // make the display list
    view_insert(wall_left.N_vertex, wall_left.N_face, wall_left.vertex, wall_left.face, wall_left.color, wall_left.backface_color);
    view_insert(wall_right.N_vertex, wall_right.N_face, wall_right.vertex, wall_right.face, wall_right.color, wall_right.backface_color);
    view_insert(wall_top.N_vertex, wall_top.N_face, wall_top.vertex, wall_top.face, wall_top.color, wall_top.backface_color);
    view_insert(mesh.N_vertex, mesh.N_face, mesh.vertex, mesh.face, mesh.color, mesh.backface_color);
    //   printf("%d\n\r", view.N_vertex) ;
    //   printf("%d %d \n\r", view.N_vertex, view.N_face);

    // ===== convert to screen coordinates
    // and extract z for depth sort
    s15x16 vz[view.N_vertex];
    s15x16 vzf[view.N_face];
    for (int i = 0; i < view.N_vertex; i++) {
        VxM( & view.vertex[i], & Tcamera, & view.vertex[i]);
        view.vertex[i].x = muls15x16(int_to_s15x16(screen_width), view.vertex[i].x) + int_to_s15x16(screen_center_x);
        view.vertex[i].y = muls15x16(int_to_s15x16(screen_width), view.vertex[i].y) + int_to_s15x16(screen_center_y);
        vz[i] = view.vertex[i].z;
        //printf("%f %f  \n\r\n\r", s15x16_to_float(view.vertex[i].x), s15x16_to_float(view.vertex[i].y)) ;
    }

    // find shallowest depth of each face
    for (int i = 0; i < view.N_face; i++) {
        //view.vertex[view.face[i].v0].x
        vzf[i] = min(vz[view.face[i].v0], min(vz[view.face[i].v1], vz[view.face[i].v2]));
        //printf("%f %f  \n\r\n\r", s15x16_to_float(view.vertex[i].x), s15x16_to_float(view.vertex[i].y)) ;
    }

    // depth sort the min depths
    int I[view.N_face];
    for (int i = 0; i < view.N_face; i++) I[i] = i;
    //
    QuickSort(vzf, I, 0, view.N_face - 1);

    // clear the drawing area
    // but wait for the vertical interval VSYNC pin 17         
    while ((vsync = gpio_get(17))) {};

    // this clear is fast enough to finish during vertical sync
    clearRect(120, 50, 520, 449, BLACK);

    // 
    // draw the depth sorted faces from back to front
    for (int j = view.N_face - 1; j >= 0; j--) {
        int i = I[j];
        int x1, y1, x2, y2;
        // ===== compute face normal for back-face culling
        Vsub( & view.vertex[view.face[i].v0], & view.vertex[view.face[i].v2], & temp1);
        Vsub( & view.vertex[view.face[i].v0], & view.vertex[view.face[i].v1], & temp2);
        Vcross( & temp1, & temp2, & temp3);
        // check for facing camera
        s15x16 face_visible = Vdot( & temp3, & Nview);
        // clip if too close to camera
        if ((vzf[i] > 0)) {
            // the face fill // face toward camera

            int check_x0 = s15x16_to_int(view.vertex[view.face[i].v0].x);
            int check_y0 = s15x16_to_int(view.vertex[view.face[i].v0].y);

            int check_x1 = s15x16_to_int(view.vertex[view.face[i].v1].x);
            int check_y1 = s15x16_to_int(view.vertex[view.face[i].v1].y);

            int check_x2 = s15x16_to_int(view.vertex[view.face[i].v2].x);
            int check_y2 = s15x16_to_int(view.vertex[view.face[i].v2].y);

            int height_min = 50;
            int width_min = 120;
            int height_max = 449;
            int width_max = 520;

            if (check_x0 < width_min || check_x1 < width_min || check_x2 < width_min) continue;
            if (check_x0 > width_max || check_x1 > width_max || check_x2 > width_max) continue;
            if (check_y0 < height_min || check_y1 < height_min || check_y2 < height_min) continue;
            if (check_y0 > height_max && check_y1 > height_max && check_y2 > height_max) continue;

            if (face_visible < 0) {
                // fillTri(view.vertex[view.face[i].v0].x, view.vertex[view.face[i].v0].y,
                //     view.vertex[view.face[i].v1].x, view.vertex[view.face[i].v1].y,
                //     view.vertex[view.face[i].v2].x, view.vertex[view.face[i].v2].y, view.color[i]);

                // draw edges
                x1 = s15x16_to_int(view.vertex[view.face[i].v0].x);
                y1 = s15x16_to_int(view.vertex[view.face[i].v0].y);
                x2 = s15x16_to_int(view.vertex[view.face[i].v1].x);
                y2 = s15x16_to_int(view.vertex[view.face[i].v1].y);
                drawLine(x1, y1, x2, y2, line_color);
                // printf("%d %d %d %d\n\r", x1, y1, x2 ,y2) ;

                x1 = s15x16_to_int(view.vertex[view.face[i].v2].x);
                y1 = s15x16_to_int(view.vertex[view.face[i].v2].y);
                x2 = s15x16_to_int(view.vertex[view.face[i].v1].x);
                y2 = s15x16_to_int(view.vertex[view.face[i].v1].y);
                drawLine(x1, y1, x2, y2, line_color);
                //printf("%d %d %d %d\n\r", x1, y1, x2 ,y2) ;

                x1 = s15x16_to_int(view.vertex[view.face[i].v0].x);
                y1 = s15x16_to_int(view.vertex[view.face[i].v0].y);
                x2 = s15x16_to_int(view.vertex[view.face[i].v2].x);
                y2 = s15x16_to_int(view.vertex[view.face[i].v2].y);
                drawLine(x1, y1, x2, y2, line_color);
                //printf("%d %d %d %d\n\r", x1, y1, x2 ,y2) ;
            }
            // backface if CULL >=0
            else if ((view.backface_color[i] > CULL)) {
                //printf("%d\n\r", )
                // fillTri(view.vertex[view.face[i].v0].x, view.vertex[view.face[i].v0].y,
                //     view.vertex[view.face[i].v1].x, view.vertex[view.face[i].v1].y,
                //     view.vertex[view.face[i].v2].x, view.vertex[view.face[i].v2].y, view.backface_color[i]);
                // draw edges
                x1 = s15x16_to_int(view.vertex[view.face[i].v0].x);
                y1 = s15x16_to_int(view.vertex[view.face[i].v0].y);
                x2 = s15x16_to_int(view.vertex[view.face[i].v1].x);
                y2 = s15x16_to_int(view.vertex[view.face[i].v1].y);
                drawLine(x1, y1, x2, y2, line_color);
                // printf("%d %d %d %d\n\r", x1, y1, x2 ,y2) ;

                x1 = s15x16_to_int(view.vertex[view.face[i].v2].x);
                y1 = s15x16_to_int(view.vertex[view.face[i].v2].y);
                x2 = s15x16_to_int(view.vertex[view.face[i].v1].x);
                y2 = s15x16_to_int(view.vertex[view.face[i].v1].y);
                drawLine(x1, y1, x2, y2, line_color);
                //printf("%d %d %d %d\n\r", x1, y1, x2 ,y2) ;

                x1 = s15x16_to_int(view.vertex[view.face[i].v0].x);
                y1 = s15x16_to_int(view.vertex[view.face[i].v0].y);
                x2 = s15x16_to_int(view.vertex[view.face[i].v2].x);
                y2 = s15x16_to_int(view.vertex[view.face[i].v2].y);
                drawLine(x1, y1, x2, y2, line_color);
            }
        }
    }

    if (button_home) {
        init_check_3d_demo_2 = false;
        kill_wiimote_objs(draggable_objs, clickable_objs);
        current_game = GAMES_MAIN_MENU;
        free_obj(&wall_left);
        free_obj(&wall_right);
        free_obj(&wall_top);
        free_obj(&mesh);
    }
    // do a second sync, depending on scene complexity 
    // you may erase this
    while((vsync=gpio_get(17))) {};
}