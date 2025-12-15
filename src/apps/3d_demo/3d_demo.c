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

static vector tetra_model_vertex[4], tetra_view_vertex[4] ;
static vector temp1, temp2, temp3 ;
static vector cube_model_vertex[8], cube_view_vertex[8]  ; 

static matrix TrotZ, Ttrans, TrotY, Ttrans_tetra, Ttrans_merge, Ttrans_pre_merge, Tscale_mesh ;
static int t, last_vsync=1, vsync;
static long long t_start ;
//
static int mesh_size = 10 ;
// vector for backface culling
static vector Nview = {0,0,1};
// edges of faces
static short line_color = BLACK ;

static bool init_check_3d_demo = false;

static object tetrahedron;
static object cube;
static object mesh;
static object test_merge ;

static int x_offset = 0;
static int y_offset = 0;

void init_objs_3d_demo () {

    init3D();

    x_offset = 0;
    y_offset = 0;

    // build the static objects and transfomrs
    build_Tetrahedron(&tetrahedron, RED, CULL); 
    build_Cube(&cube, MED_GREEN, CULL) ;
    // build a merged object
    build_Translate (float_to_s15x16(-1.0), float_to_s15x16(-0.5), float_to_s15x16(0), &Ttrans_pre_merge);
    // transform the actual tetrahedon
    for (int i=0; i<tetrahedron.N_vertex; i++){
      VxM(&tetrahedron.vertex[i], &Ttrans_pre_merge, &tetrahedron.vertex[i]);
    }
    // merge cube and modified tetrahedron
    merge_object(&cube, &tetrahedron, &test_merge, LIGHT_BLUE, CULL); 
    // rebuild fresh tetrahedron with default position
    build_Tetrahedron(&tetrahedron, DARK_ORANGE, CULL);
    //printf("%d \n\r", test_merge.N_vertex) ;
    build_Mesh(&mesh, mesh_size, YELLOW, ORANGE) ;

    // move stuff
    build_Translate (float_to_s15x16(-4.0), float_to_s15x16(-1.5), float_to_s15x16(-2), &Ttrans_tetra);
    build_Translate (float_to_s15x16(-0.5), float_to_s15x16(-0.5), float_to_s15x16(-0.5), &Ttrans);
    build_Translate (float_to_s15x16(-2.0), float_to_s15x16(-1), float_to_s15x16(1), &Ttrans_merge);
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

void draw_3d_demo() {

    // Init objs only on first run.
    if (!init_check_3d_demo) {
        init_objs_3d_demo();
        init_check_3d_demo = true;
    }
    
    // Draw some filled rectangles
    fillRect(64, 0, 176, 50, BLUE); // blue box
    //fillRect(250, 0, 176, 50, DARK_ORANGE); // red box
    fillRect(435, 0, 176, 50, LIGHT_BLUE); // green box

    // Write some text
    setTextColor(WHITE) ;
    setCursor(65, 0) ;
    setTextSize(1) ;
    writeString("Raspberry Pi Pico2") ;
    setCursor(65, 10) ;
    writeString("3D demo") ;
    setCursor(65, 20) ;
    writeString("Hunter Adams") ;
    setCursor(65, 30) ;
    writeString("Bruce Land") ;
    setCursor(65, 40) ;
    writeString("ece4760 Cornell") ;
    //
    setCursor(445, 10) ;
    setTextColor(BLACK) ;
    setTextSize(1) ;
    
    writeString("Protothreads rp2040 v1.4") ;
    setCursor(445, 20) ;
    writeString("VGA_16_v2") ;
    setCursor(445, 30) ;
    writeString("Tested: pico2 SDK2.1") ;
    ///
    //
    setCursor(270, 20) ;
    setTextColor2(WHITE, BLACK) ;
    writeStringBig("3D Render Test") ;
    setCursor(270, 37) ;

    t = (t + 1) % 1000;

    if (dpad_left) y_offset--;
    if (dpad_right) y_offset++;
    if (dpad_down) x_offset--;
    if (dpad_up) x_offset++;

    matrix Tcamera ;
    matrix Tview ;
    matrix Tpersp ;
    
    // === define the camera ======
    // 'from' is camera location in model sapace
    // 'to' is where the camera is pointed
    vector from = {float_to_s15x16(4.0 - (x_offset/2.0)), float_to_s15x16(1.0 - (y_offset/2.0)), -float_to_s15x16(0.5)} ;
    vector to = {float_to_s15x16(0.), float_to_s15x16(0.), 0} ;
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

    // === cube modeling
    for (int i = 0; i < cube.N_vertex; i++) {
        VxM( & cube.vertex[i], & Ttrans, & cube_model_vertex[i]); //
        VxM( & cube_model_vertex[i], & TrotZ, & cube_model_vertex[i]); //
        VxM( & cube_model_vertex[i], & TrotY, & cube_model_vertex[i]); //
        //
    }

    // === tetrahedron modeling
    for (int i = 0; i < tetrahedron.N_vertex; i++) {
        VxM( & tetrahedron.vertex[i], & TrotY, & tetra_model_vertex[i]);
        VxM( & tetra_model_vertex[i], & Ttrans_tetra, & tetra_model_vertex[i]);
    }

    // mesh deform funciton
    for (int i = 0; i < mesh_size; i++) {
        for (int j = 0; j < mesh_size; j++) {
            mesh.vertex[i + mesh_size * j].z = muls15x16(cosine[36 * j % 360], muls15x16(cosine[36 * i % 360], cosine[(4 * t) % 360] >> 2));
        }
    }

    // this vector has to be declared in the time loop beause the dimension
    // is not known at compile time, but is dynamically allocated
    vector test_merge_model_vertex[test_merge.N_vertex];
    // merge object modeling
    for (int i = 0; i < test_merge.N_vertex; i++) {
        VxM( & test_merge.vertex[i], & TrotY, & test_merge_model_vertex[i]);
        VxM( & test_merge_model_vertex[i], & Ttrans_merge, & test_merge_model_vertex[i]);
    }

    // make the display list
    view_insert(tetrahedron.N_vertex, tetrahedron.N_face, tetra_model_vertex, tetrahedron.face, tetrahedron.color, tetrahedron.backface_color);
    view_insert(cube.N_vertex, cube.N_face, cube_model_vertex, cube.face, cube.color, cube.backface_color);
    view_insert(test_merge.N_vertex, test_merge.N_face, test_merge_model_vertex, test_merge.face, test_merge.color, test_merge.backface_color);
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
    clearRect(100, 25, 540, 474, BLACK);

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

            int height_min = 25;
            int width_min = 100;
            int height_max = 474;
            int width_max = 540;

            if (check_x0 < width_min || check_x1 < width_min || check_x2 < width_min) continue;
            if (check_x0 > width_max || check_x1 > width_max || check_x2 > width_max) continue;
            if (check_y0 < height_min || check_y1 < height_min || check_y2 < height_min) continue;
            if (check_y0 > height_max && check_y1 > height_max && check_y2 > height_max) continue;

            if (face_visible < 0) {
                fillTri(view.vertex[view.face[i].v0].x, view.vertex[view.face[i].v0].y,
                    view.vertex[view.face[i].v1].x, view.vertex[view.face[i].v1].y,
                    view.vertex[view.face[i].v2].x, view.vertex[view.face[i].v2].y, view.color[i]);

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
                fillTri(view.vertex[view.face[i].v0].x, view.vertex[view.face[i].v0].y,
                    view.vertex[view.face[i].v1].x, view.vertex[view.face[i].v1].y,
                    view.vertex[view.face[i].v2].x, view.vertex[view.face[i].v2].y, view.backface_color[i]);
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
        init_check_3d_demo = false;
        kill_wiimote_objs(draggable_objs, clickable_objs);
        current_game = GAMES_MAIN_MENU;
        free_obj(&tetrahedron);
        free_obj(&cube);
        free_obj(&test_merge);
        free_obj(&mesh);
    }
    // do a second sync, depending on scene complexity 
    // you may erase this
    while((vsync=gpio_get(17))) {};
}