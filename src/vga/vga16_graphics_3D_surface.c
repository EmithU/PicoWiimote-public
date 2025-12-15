/**
 * Hunter Adams (vha3@cornell.edu)
 * 
 * HARDWARE CONNECTIONS
 *  - GPIO 16 ---> VGA Hsync
 *  - GPIO 17 ---> VGA Vsync
 *  - GPIO 18 ---> 470 ohm resistor ---> VGA Green 
 *  - GPIO 19 ---> 330 ohm resistor ---> VGA Green
 *  - GPIO 20 ---> 330 ohm resistor ---> VGA Blue
 *  - GPIO 21 ---> 330 ohm resistor ---> VGA Red
 *  - RP2040 GND ---> VGA GND
 * 
 * RESOURCES USED
 *  - PIO state machines 0, 1, and 2 on PIO instance 0
 *  - DMA channels 0, 1, 2, and 3
 *  - 153.6 kBytes of RAM (for pixel color data)
 *
 * Protothreads v1.3
 * Threads:
 * core 0:
 * Graphics demo
 * blink LED25 
 * core 1:
 * Toggle gpio 4 
 * Serial i/o 
 */
// ==========================================
// === VGA graphics library
// ==========================================
#include "vga16_graphics_v2.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/dma.h"

// =========================================
// === protothreads globals
// =========================================
#include "hardware/sync.h"
#include "hardware/timer.h"
#include "pico/multicore.h"
#include "string.h"
#include "pico/malloc.h"

#include "vga16_graphics_3D_surface.h"

// signal to redraw animation
int new_str = true ;

// see below for better sqrt
//#define sqrts15x16(a) (float_to_s15x16(sqrt(s15x16_to_float(a)))) 
/*
Algorithm and code Author Christophe Meessen 1993. 
  Initially published in usenet comp.lang.c, Thu, 28 Jan 1993 08:35:23 GMT, 
  Subject: Fixed point sqrt ; by Meessen Christophe
  https://groups.google.com/forum/?hl=fr%05aacf5997b615c37&fromgroups#!topic/comp.lang.c/IpwKbw0MAxw/discussion
  Note: there was a bug in the published sqrtL2L routine. It is corrected in
  this implementation.
  This code from https://github.com/chmike/fpsqrt/blob/master/fpsqrt.c
*/
// the squrare root of a fixed point with 16 bit
// fractional part and returns a fixed point with 16 bit fractional part. It 
// requires that v is positive. The computation use only 32 bit registers and 
// simple operations.
s15x16 sqrts15x16(s15x16 v) {
    uint32_t t, q, b, r;
    r = v;
    b = 0x40000000;
    q = 0;
    while( b > 0x40 )
    {
        t = q + b;
        if( r >= t )
        {
            r -= t;
            q = t + b; // equivalent to q += 2*b
        }
        r <<= 1;
        b >>= 1;
    }
    q >>= 8;
    return q;
}

// ===== vector operations =====
// ====== add vectors
void Vadd(vector* v1, vector* v2, vector* result){
  result->x = v1->x + v2->x ;
  result->y = v1->y + v2->y ; 
  result->z = v1->z + v2->z ;
}

// ====== subtract 
void Vsub(vector* v1, vector* v2, vector* result){
  result->x = v1->x - v2->x ;
  result->y = v1->y - v2->y ; 
  result->z = v1->z - v2->z ;
}

// ======
// scale a vector by a scalar 
void VxS(vector* v1, s15x16 scale, vector* result){
  result->x = muls15x16(v1->x, scale) ;
  result->y = muls15x16(v1->y, scale) ; 
  result->z = muls15x16(v1->z, scale) ;
}

// ====== magnitude
s15x16 Vmag(vector* v){
  return sqrts15x16(muls15x16(v->x, v->x) + muls15x16(v->y, v->y) + muls15x16(v->z, v->z)) ;
}

// ====== normalize (unity magnitude)
void Vnorm(vector* v){
   s15x16 mag = sqrts15x16(muls15x16(v->x, v->x) + muls15x16(v->y, v->y) + muls15x16(v->z, v->z)) ;
   v->x = divs15x16(v->x, mag) ;
   v->y = divs15x16(v->y, mag) ;
   v->z = divs15x16(v->z, mag) ;
}

// ====== dot porduct
s15x16 Vdot(vector* v1, vector* v2){
  return (muls15x16(v1->x, v2->x) + muls15x16(v1->y, v2->y) + muls15x16(v1->z, v2->z)) ;
}

// ====== cross product
// cross_P[0] = vect_A[1] * vect_B[2] - vect_A[2] * vect_B[1];
// cross_P[1] = vect_A[2] * vect_B[0] - vect_A[0] * vect_B[2];
// cross_P[2] = vect_A[0] * vect_B[1] - vect_A[1] * vect_B[0];
// ======
void Vcross(vector* v1, vector* v2, vector* cross_p){
  vector cross ;
  cross.x = muls15x16(v1->y, v2->z) - muls15x16(v1->z, v2->y) ;
  cross.y = muls15x16(v1->z, v2->x) - muls15x16(v1->x, v2->z) ;
  cross.z = muls15x16(v1->x, v2->y) - muls15x16(v1->y, v2->x) ;
  // use temp so can overwrite input
  memcpy(cross_p, &cross, sizeof(vector)) ;
}

// ===== Matrix opesrations

// ===== matrix times matrix
void MxM(matrix* A, matrix* B, matrix* prod) {
    prod->m11 = muls15x16(A->m11, B->m11) + muls15x16(A->m12, B->m21) + muls15x16(A->m13, B->m31) + muls15x16(A->m14, B->m41);
    prod->m12 = muls15x16(A->m11, B->m12) + muls15x16(A->m12, B->m22) + muls15x16(A->m13, B->m32) + muls15x16(A->m14, B->m42);
    prod->m13 = muls15x16(A->m11, B->m13) + muls15x16(A->m12, B->m23) + muls15x16(A->m13, B->m33) + muls15x16(A->m14, B->m43);
    prod->m14 = muls15x16(A->m11, B->m14) + muls15x16(A->m12, B->m24) + muls15x16(A->m13, B->m34) + muls15x16(A->m14, B->m44);
    prod->m21 = muls15x16(A->m21, B->m11) + muls15x16(A->m22, B->m21) + muls15x16(A->m23, B->m31) + muls15x16(A->m24, B->m41);
    prod->m22 = muls15x16(A->m21, B->m12) + muls15x16(A->m22, B->m22) + muls15x16(A->m23, B->m32) + muls15x16(A->m24, B->m42);
    prod->m23 = muls15x16(A->m21, B->m13) + muls15x16(A->m22, B->m23) + muls15x16(A->m23, B->m33) + muls15x16(A->m24, B->m43);
    prod->m24 = muls15x16(A->m21, B->m14) + muls15x16(A->m22, B->m24) + muls15x16(A->m23, B->m34) + muls15x16(A->m24, B->m44);
    prod->m31 = muls15x16(A->m31, B->m11) + muls15x16(A->m32, B->m21) + muls15x16(A->m33, B->m31) + muls15x16(A->m34, B->m41);
    prod->m32 = muls15x16(A->m31, B->m12) + muls15x16(A->m32, B->m22) + muls15x16(A->m33, B->m32) + muls15x16(A->m34, B->m42);
    prod->m33 = muls15x16(A->m31, B->m13) + muls15x16(A->m32, B->m23) + muls15x16(A->m33, B->m33) + muls15x16(A->m34, B->m43);
    prod->m34 = muls15x16(A->m31, B->m14) + muls15x16(A->m32, B->m24) + muls15x16(A->m33, B->m34) + muls15x16(A->m34, B->m44);
    prod->m41 = muls15x16(A->m41, B->m11) + muls15x16(A->m42, B->m21) + muls15x16(A->m43, B->m31) + muls15x16(A->m44, B->m41);
    prod->m42 = muls15x16(A->m41, B->m12) + muls15x16(A->m42, B->m22) + muls15x16(A->m43, B->m32) + muls15x16(A->m44, B->m42);
    prod->m43 = muls15x16(A->m41, B->m13) + muls15x16(A->m42, B->m23) + muls15x16(A->m43, B->m33) + muls15x16(A->m44, B->m43);
    prod->m44 = muls15x16(A->m41, B->m14) + muls15x16(A->m42, B->m24) + muls15x16(A->m43, B->m34) + muls15x16(A->m44, B->m44);
}

// Draw a filled triangle
// uses top-right rasterization rule to leave no holes between triangles
// (see https://en.wikipedia.org/wiki/Rasterisation)
void fillTri(s15x16 x0, s15x16 y0, s15x16 x1, s15x16 y1, s15x16 x2, s15x16 y2, char color) {
  //
  // sort verts so y0 <= y1 <= y2 (p0 = top, p1 = middle, p2 = bottom)
  if (y1 < y0) {
    swap(x0, x1);
    swap(y0, y1);
  }
  if (y2 < y0) {
    swap(x0, x2);
    swap(y0, y2);
  }
  if (y2 < y1) {
    swap(x1, x2);
    swap(y1, y2);
  }

  // calculate slopes of each edge, in fix15 (don't divide by 0)
  s15x16 dxdy_01 = y1 == y0 ? 0 : divs15x16 (x1 - x0, y1 - y0);
  s15x16 dxdy_02 = y2 == y0 ? 0 : divs15x16(x2 - x0, y2 - y0);
  s15x16 dxdy_12 = y2 == y1 ? 0 : divs15x16(x2 - x1, y2 - y1);
  // same for z
  //s15x16 dzdy_01 = y1 == y0 ? 0 : divs15x16(int_to_s15x16(z1 - z0), y1 - y0);
 // s15x16 dzdy_02 = y2 == y0 ? 0 : divs15x16(int_to_s15x16(z2 - z0), y2 - y0);
 // s15x16 dzdy_12 = y2 == y1 ? 0 : divs15x16(int_to_s15x16(z2 - z1), y2 - y1);

  // figure out whether p1 is on the left or right side of the triangle
  bool flat_top = (y0 == y1);
  bool flat_bottom = (y1 == y2);
  bool p1_is_left = flat_top ? (x0 > x1) : (dxdy_02 > dxdy_01);

  // starting at p0, we draw horizontal scanlines (from x_left to x_right, at height y)
  s15x16 x_left = x0;
  s15x16 x_right = x0;
  s15x16 y = y0;

  // similarly, we have interpolators for z coordinates
  //s15x16 z_left = int_to_s15x16(z0) + zeropt5;
  //s15x16 z_right = int_to_s15x16(z0) + zeropt5;

  // x_left and x_right are moved based on slopes of left/right edges
  s15x16 dx_left, dx_right;
  s15x16 dz_left, dz_right;
  if (p1_is_left) {
    dx_left = dxdy_01;
    dx_right = dxdy_02;
   // dz_left = dzdy_01;
   // dz_right = dzdy_02;
  } else {
    dx_left = dxdy_02;
    dx_right = dxdy_01;
    //dz_left = dzdy_02;
    //dz_right = dzdy_01;
  }

  // macro function to move the scanline down, and update its endpoints
  #define moveScanline() {\
    y += one;\
    x_left += dx_left;\
    x_right += dx_right;\
  }
    //z_left += dz_left;\
   // z_right += dz_right;\
  }
  
  // draw top half of triangle; skipped for flat top case
  while (y < y1) {
    //drawScanline((short)fix2int15(y), (short)fix2int15(x_left), (short)fix2int15(x_right), z_left, z_right, color);
    drawHLine(s15x16_to_int (x_left), s15x16_to_int(y), abs(s15x16_to_int(x_right-x_left)), color);

    moveScanline();
  }

  // flat bottom triangles skip the rest
  if (flat_bottom)
    return;

  // reconfigure one end of the scanline so it goes from p1 to p2
  if (p1_is_left) {
    x_left = x1;
    dx_left = dxdy_12;
    //z_left = int2fix15(z1) + zeropt5;
   // dz_left = dzdy_12;
  } else {
    x_right = x1;
    dx_right = dxdy_12;
   // z_right = int2fix15(z1) + zeropt5;
   // dz_right = dzdy_12;
  }

  // draw horizontal line through p1 (middle)
  //drawScanline((short)fix2int15(y), (short)fix2int15(x_left), (short)fix2int15(x_right), z_left, z_right, color);
  drawHLine(s15x16_to_int(x_left), s15x16_to_int(y), s15x16_to_int(x_right-x_left), color);
  // draw bottom half of triangle; skipped for flat bottom case
  while (y < y2) {
    moveScanline();
    //drawScanline((short)fix2int15(y), (short)fix2int15(x_left), (short)fix2int15(x_right), z_left, z_right, color);
    drawHLine(s15x16_to_int(x_left), s15x16_to_int(y), s15x16_to_int(x_right-x_left), color);
  }
}
// end of copied code
/////////////////////////////////////////////////////////////////////

// ====== vector times matrix !!! All row vectors used here!!!
// 3 vector times 4x4 matrix returns 3 vector
// last element in vector is implicitly unity
// this works for all transforms EXCEPT camera perspective transform
// returns -1 if illegal w cood

int VxM( vector* v, matrix* mm, vector* prod_p ){
  s15x16 w ;
  vector prod ;
  prod.x = muls15x16(mm->m11, v->x) + muls15x16(mm->m21, v->y) + muls15x16(mm->m31, v->z) + (mm->m41) ;
  prod.y = muls15x16(mm->m12, v->x) + muls15x16(mm->m22, v->y) + muls15x16(mm->m32, v->z) + (mm->m42) ;
  prod.z = muls15x16(mm->m13, v->x) + muls15x16(mm->m23, v->y) + muls15x16(mm->m33, v->z) + (mm->m43) ;
  w      = muls15x16(mm->m14, v->x) + muls15x16(mm->m24, v->y) + muls15x16(mm->m34, v->z) + (mm->m44) ;
  if (w <= 0) return -1 ;
  if (w != one){
    prod.x = divs15x16(prod.x, w) ;
    prod.y = divs15x16(prod.y, w) ;
    prod.z = divs15x16(prod.z, w) ;
  }
  // use temp so can overwrite input
  memcpy(prod_p, &prod, sizeof(vector)) ;
  return 0 ;
}

// ===== just some test matrices
// 'one' is unity in fixed point
matrix m1={one, 0, 0, 0,
           0, one, 0, 0,
           0, 0, one, 0,
           0, 0, 0, one} ;

matrix m2={one, 2*one, 3*one, 4*one,
           5*one, 6*one, 7*one, 8*one,
           9*one, 10*one, 11*one, 12*one,
           13*one, 14*one, 15*one, 16*one} ;           

matrix p ={one, 0, 0, 0,
           0, one, 0, 0,
           0, 0, one, 0,
           0, 0, 0, one} ;

matrix m3 = {one, 0, 0, 0,
             0, one, 0, 0,
             0, 0, one, 0,
            one, 2*one, 3*one, one};

matrix m4 = {one, 0, 0, 0,
             0, one, 0, 0,
              0, 0, one, one, 
              0, 0, -2*one, 0};

// ====== Modeling transforms
// build z rotate -- angle degrees
// all sine/cos are table lookup to one degreee
s15x16 cosine[360], sine[360] ;

// ====== rotate vertices around Z axis
// angles in degrees!
// note that thses are just table lookups
void build_RotZ(int angle, matrix* TrotZ){
    TrotZ->m11 = cosine[angle];
    TrotZ->m12 = sine[angle] ;
    TrotZ->m13 = 0;
    TrotZ->m14 = 0;
    TrotZ->m21 = -sine[angle] ;
    TrotZ->m22 = cosine[angle] ;
    TrotZ->m23 = 0 ;
    TrotZ->m24 = 0 ;
    TrotZ->m31 = 0 ;
    TrotZ->m32 = 0 ;
    TrotZ->m33 = one ;
    TrotZ->m34 = 0 ;
    TrotZ->m41 = 0 ;
    TrotZ->m42 = 0 ;
    TrotZ->m43 = 0 ;
    TrotZ->m44 = one ;
}

// ====== rotate vertices around x axis
void build_RotX(int angle, matrix* TrotX){
    TrotX->m11 = one ;
    TrotX->m12 = 0 ;
    TrotX->m13 = 0;
    TrotX->m14 = 0;
    TrotX->m21 = 0 ;
    TrotX->m22 = cosine[angle] ;
    TrotX->m23 = sine[angle] ;
    TrotX->m24 = 0 ;
    TrotX->m31 = 0 ;
    TrotX->m32 = -sine[angle] ;
    TrotX->m33 = cosine[angle] ;
    TrotX->m34 = 0 ;
    TrotX->m41 = 0 ;
    TrotX->m42 = 0 ;
    TrotX->m43 = 0 ;
    TrotX->m44 = one ;
}

// ====== rotate vertices around y axis
void build_RotY(int angle, matrix* TrotX){
    TrotX->m11 = cosine[angle] ;
    TrotX->m12 = 0 ;
    TrotX->m13 = -sine[angle] ;
    TrotX->m14 = 0;
    TrotX->m21 = 0 ;
    TrotX->m22 = one ;
    TrotX->m23 = 0 ;
    TrotX->m24 = 0 ;
    TrotX->m31 = sine[angle] ;
    TrotX->m32 = 0 ;
    TrotX->m33 = cosine[angle] ;
    TrotX->m34 = 0 ;
    TrotX->m41 = 0 ;
    TrotX->m42 = 0 ;
    TrotX->m43 = 0 ;
    TrotX->m44 = one ;
}

// ====== translate vertices by (x,y,z)
void build_Translate(s15x16 x, s15x16 y, s15x16 z, matrix* Ttrans){
    Ttrans->m11 = one ;
    Ttrans->m12 = 0 ;
    Ttrans->m13 = 0;
    Ttrans->m14 = 0;
    Ttrans->m21 = 0 ;
    Ttrans->m22 = one ;
    Ttrans->m23 = 0 ;
    Ttrans->m24 = 0 ;
    Ttrans->m31 = 0 ;
    Ttrans->m32 = 0 ;
    Ttrans->m33 = one ;
    Ttrans->m34 = 0 ;
    Ttrans->m41 = x ;
    Ttrans->m42 = y ;
    Ttrans->m43 = z ;
    Ttrans->m44 = one ;
}

// ====== scale vertices along each axis
// sx, sy, sz
void build_Scale(s15x16 sx, s15x16 sy, s15x16 sz, matrix* Ttrans){
    Ttrans->m11 = sx ;
    Ttrans->m12 = 0 ;
    Ttrans->m13 = 0;
    Ttrans->m14 = 0;
    Ttrans->m21 = 0 ;
    Ttrans->m22 = sy ;
    Ttrans->m23 = 0 ;
    Ttrans->m24 = 0 ;
    Ttrans->m31 = 0 ;
    Ttrans->m32 = 0 ;
    Ttrans->m33 = sz ;
    Ttrans->m34 = 0 ;
    Ttrans->m41 = 0 ;
    Ttrans->m42 = 0 ;
    Ttrans->m43 = 0 ;
    Ttrans->m44 = one ;
}

// ===== view operations
// ======
view_s view;

// init view list
// once per frame, probably
void view_init(void){
  view.N_vertex = 0;
  view.N_face = 0;
}

// insert object into view list
void view_insert(int Nv, int Nf, vector v[], face f[], short color, short backface_color){
  // add vertexs at end 
  int current_Nv = view.N_vertex ;
  int current_Nf = view.N_face ;
  int j = 0;
  for (int i=current_Nv; i<Nv+current_Nv; i++){
    view.vertex[i].x = v[j].x ;
    view.vertex[i].y = v[j].y ;
    view.vertex[i].z = v[j].z ;
    j++ ;
  }
  j = 0 ;
  for (int i=current_Nf; i<Nf+current_Nf; i++){
    view.face[i].v0 = f[j].v0 + current_Nv;
    view.face[i].v1 = f[j].v1 + current_Nv;
    view.face[i].v2 = f[j].v2 + current_Nv ;
    view.color[i] = color ;
    view.backface_color[i] = backface_color ;
    j++ ;
  }
  view.N_vertex += Nv;
  view.N_face += Nf ;
}

// ===== build view matrix
void build_Tview(vector* from, vector* to, vector* approxUp, matrix* Tview){
  // view direction: N = LookTo - LookFrom (normalized) 
  // approx UP: V' convert to true UP: V = V'- (V' dot N) N (normalized) 
  // screen-right U = N cross V (normalized) 
  // camera location: cam_x, cam_y, cam_z == LookFrom
  // the projection distance (LookFrom to screen) proj_d
  //
  // view matrix = (tranlate camera) * (rotate camera)
  matrix Ttrans;
  build_Translate(-from->x, -from->y, -from->z, &Ttrans) ;

  // rotate matrix
  //  Ux  Vx  Nx  0
  //  Uy  Vy  Ny  0
  //  Uz  Vz  Nz  0
  //  0   0   0   1
  vector N, V, U ;
  vector temp1, temp2, temp3 ;
  // get N
  Vsub(to, from, &N);
  Vnorm(&N) ;
  // get V
  Vnorm(approxUp) ;
  //s15x16 dot = Vdot(approxUp, &N) ;
  // void VxS(vector* v1, s15x16 scale, vector* result)
  VxS(&N, Vdot(approxUp, &N), &temp1) ;
  Vsub(approxUp, &temp1, &V ) ;
  Vnorm(&V) ;
  // get U
  Vcross(&N, &V, &U);
  Vnorm(&U);
  // build the rotation
  matrix T;
  T.m11 = U.x ;
  T.m12 = V.x ;
  T.m13 = N.x ;
  T.m14 = 0;
  T.m21 = U.y ;
  T.m22 = V.y ;
  T.m23 = N.y ;
  T.m24 = 0 ;
  T.m31 = U.z ;
  T.m32 = V.z ;
  T.m33 = N.z ;
  T.m34 = 0 ;
  T.m41 = 0 ;
  T.m42 = 0 ;
  T.m43 = 0 ;
  T.m44 = one ;
  // combine 
  MxM(&Ttrans, &T, Tview);
}

// ===== build_Tpersp()
// d is distance for eye to projection plane
// f is the far render limit
// h is half-height of the screen
// d   0   0        0
// 0   d   0        0
// 0   0  f/(f-d)   1
// 0   0 -fd/(f-d)  0
void build_Tpersp(s15x16 d, s15x16 h, s15x16 f, matrix* Tpersp){
  Tpersp->m11 = divs15x16(d, h)  ;
  Tpersp->m12 = 0 ;
  Tpersp->m13 = 0 ;
  Tpersp->m14 = 0;
  Tpersp->m21 = 0 ;
  Tpersp->m22 = divs15x16(d, h)  ;
  Tpersp->m23 = 0 ;
  Tpersp->m24 = 0 ;
  Tpersp->m31 = 0 ;
  Tpersp->m32 = 0 ;
  Tpersp->m33 = divs15x16(f, f-d) ;
  Tpersp->m34 = one ;
  Tpersp->m41 = 0 ;
  Tpersp->m42 = 0 ;
  Tpersp->m43 = -divs15x16(muls15x16(f,d), f-d) ; ;
  Tpersp->m44 = 0 ;

}
// =================
// test view/persp matrices
// cam=2,0,0; N=-1,0,0; V=0,1,0, U=0,0,-1
// looking toward the origin from x=2 with up in y direction, with screen-right along neg z
//matrix Tview = {0, 0, -one, 0, 
//                0, one, 0, 0,
 //               -one, 0, 0, 0,
  //              0, 0, 4*one, one} ;

// d=1, h=200 pixels, 
// matrix Tpersp = {one, 0, 0, 0,
//                0, one, 0, 0,
//                0, 0, one, one,
//               0, 0, -one, one} ;
 // =================              

// =====
void build_Tetrahedron(object* tetrahedron, short color, short backface_color){
    tetrahedron->N_vertex = 4 ;
    tetrahedron->N_face = 4 ;
    tetrahedron->color = color ;
    tetrahedron->backface_color = backface_color ;
    vector v[4] = {one/2,one/2,one/2,  one/2,-one/2,-one/2,  -one/2,-one/2,one/2, -one/2,one/2,-one/2};
    face f[4] =  {0,2,1,  0,1,3,  0,3,2,  1,2,3} ;
   // printf("%d %d\n\r",  o3->N_vertex, o3->N_face) ;
    tetrahedron->vertex = (vector*) malloc((tetrahedron->N_vertex)*sizeof(vector));
    for(int i=0; i<tetrahedron->N_vertex; i++){
      tetrahedron->vertex[i].x = v[i].x ;
      tetrahedron->vertex[i].y = v[i].y ;
      tetrahedron->vertex[i].z = v[i].z ;
    }
    //
   tetrahedron->face = (face*) malloc((tetrahedron->N_face)*sizeof(face));
   for(int i=0; i<tetrahedron->N_vertex; i++){
      tetrahedron->face[i].v0 = f[i].v0 ;
      tetrahedron->face[i].v1 = f[i].v1 ;
      tetrahedron->face[i].v2 = f[i].v2 ;
    }
}
 // ======
void build_Cube(object* cube, short color, short backface_color){
    cube->N_vertex = 8 ;
    cube->N_face = 12 ;
    cube->color = color ;
    cube->backface_color = backface_color ;
    vector v[8] = { 0,0,0, one,0,0, one,one,0, 0,one,0, 0,0,one, one,0,one, one,one,one, 0,one,one};
    face f[12] =   {0,1,4, 1,5,4, 1,2,5, 2,6,5, 2,3,6, 3,7,6, 
                    3,0,7, 0,4,7, 0,3,1, 1,3,2, 4,5,7, 5,6,7} ;
   // printf("%d %d\n\r",  o3->N_vertex, o3->N_face) ;
    cube->vertex = (vector*) malloc((cube->N_vertex)*sizeof(vector));
    for(int i=0; i<cube->N_vertex; i++){
      cube->vertex[i].x = v[i].x ;
      cube->vertex[i].y = v[i].y ;
      cube->vertex[i].z = v[i].z ;
    }
    //
   cube->face = (face*) malloc((cube->N_face)*sizeof(face));
   for(int i=0; i<cube->N_face; i++){
      cube->face[i].v0 = f[i].v0 ;
      cube->face[i].v1 = f[i].v1 ;
      cube->face[i].v2 = f[i].v2 ;
    }
}

// === 2D grid ==============
void build_Mesh(object* mesh, short size, short color, short backface_color){
    mesh->N_vertex = size*size ;
    mesh->N_face = 2 * (size-1) * (size-1) ;
    mesh->color = color ;
    mesh->backface_color = backface_color ;
   // 
    mesh->face = (face*) malloc((mesh->N_face)*sizeof(face));
    mesh->vertex = (vector*) malloc((mesh->N_vertex)*sizeof(vector));
    
    for(int i=0; i<size; i++){ //x
       for(int j=0; j<size; j++){ //y
          int k = j + (i * size) ;
          mesh->vertex[k].x = muls15x16(int_to_s15x16(i), divs15x16(one, int_to_s15x16(size-1) )) ;
          mesh->vertex[k].y = muls15x16(int_to_s15x16(j), divs15x16(one, int_to_s15x16(size-1) )) ;
          mesh->vertex[k].z =  0 ;
          //printf("%d %d %d %f %f %f \n\r", i, j, k, s15x16_to_float(mesh.vertex[k].x), s15x16_to_float(mesh.vertex[k].y), s15x16_to_float(mesh.vertex[k].z)) ;
       }
    }
    //
    // polygon input index
    int inputVertex = 0  ;
   for(int i=0; i<(size-1); i++){
      for(int j=0; j<(size); j++){
        // output index
        int index = j + i * (size-1);
        // test for end of row and skip if at end
        if (j == size-1){
          inputVertex++;
          continue;
        }
        // draw two triangle faces for each surface square
        mesh->face[index].v0 = inputVertex ;
        mesh->face[index].v2 = inputVertex + 1 ;
        mesh->face[index].v1 = inputVertex + 1 + (size) ;
        //printf("%d %d %d %d %d %d \n\r", i, j, index, mesh.face[index].v0 , mesh.face[index].v2 , mesh.face[index].v1 );
        //
        mesh->face[index+mesh->N_face/2].v0 = inputVertex ;
        mesh->face[index+mesh->N_face/2].v2 =  inputVertex + 1 + (size) ;
        mesh->face[index+mesh->N_face/2].v1 = inputVertex + (size) ;
        //
        inputVertex++ ;
      }
    }
}

// =========== merge two objects
void merge_object(object* o1, object* o2, object* o3, short color, short backface_color){
    o3->N_vertex = o1->N_vertex + o2->N_vertex ;
    o3->N_face = o1->N_face + o2->N_face ;
    o3->color = color ;
    o3->backface_color = backface_color ;
   // printf("%d %d\n\r",  o3->N_vertex, o3->N_face) ;
    o3->face = (face*) malloc((o3->N_face)*sizeof(face));
    o3->vertex = (vector*) malloc((o3->N_vertex)*sizeof(vector));
    // now do the vertex copies
    int j = 0 ;
    for (int i=0; i<(o1->N_vertex); i++){
      o3->vertex[j].x = o1->vertex[i].x ;
      o3->vertex[j].y = o1->vertex[i].y ;
      o3->vertex[j].z = o1->vertex[i].z ;
      j++ ;
    }
     int o1_count = j ;
    for (int i=0; i<(o2->N_vertex); i++){
      o3->vertex[j].x = o2->vertex[i].x ;
      o3->vertex[j].y = o2->vertex[i].y ;
      o3->vertex[j].z = o2->vertex[i].z ;
      j++ ;
    }
   
    //printf("%d\n\r", j);
    // the face copies
    j = 0 ;
    for (int i=0; i<(o1->N_face); i++){
      o3->face[j].v0 = o1->face[i].v0 ;
      o3->face[j].v1 = o1->face[i].v1 ;
      o3->face[j].v2 = o1->face[i].v2 ;
      j++ ;
    }
    //int o1_count = j ;
    for (int i=0; i<(o2->N_face); i++){
      o3->face[j].v0 = o2->face[i].v0 + o1_count ;
      o3->face[j].v1 = o2->face[i].v1 + o1_count ;
      o3->face[j].v2 = o2->face[i].v2 + o1_count ;
      j++ ;
    }
  }

void build_Fruit(object* fruit, short color){
    short size = 8;
    build_Mesh(fruit, size, color, color);

    for(int i=0; i<fruit->N_vertex; i++){
        float x = s15x16_to_float(fruit->vertex[i].x) - 0.5f;
        float y = s15x16_to_float(fruit->vertex[i].y) - 0.5f;
        float z = 0.5f * (1.0f - (x*x + y*y));
        fruit->vertex[i].x = float_to_s15x16(x);
        fruit->vertex[i].y = float_to_s15x16(y);
        fruit->vertex[i].z = float_to_s15x16(z);
    }
}

  // =======================================
  // quicksort from 
  // https://stackoverflow.com/questions/55976487/get-the-sorted-indices-of-an-array-using-quicksort
  // ========================================
void QuickSort(s15x16 A[], int I[], int lo, int hi)
{
    if (lo < hi)
    {
        float pivot = A[I[lo + (hi - lo) / 2]];
        int t;
        int i = lo - 1;
        int j = hi + 1;
        while (1)
        {
            while (A[I[++i]] < pivot);
            while (A[I[--j]] > pivot);
            if (i >= j)
                break;
            t = I[i];
            I[i] = I[j];
            I[j] = t;
        }
        QuickSort(A, I, lo, j);
        QuickSort(A, I, j + 1, hi);
    }
}

void init3D() {
  for (int i=0; i<360; i++){
    sine[i] = float_to_s15x16(sin(2*3.14159*(float)i/360)) ;
    cosine[i] = float_to_s15x16(cos(2*3.14159*(float)i/360)) ;
  }
}

void free_obj(object* obj) {
  free(obj->face);
  free(obj->vertex);
}