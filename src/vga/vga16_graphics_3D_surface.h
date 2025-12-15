#ifndef VGA16_GRAPHICS_3D_SURFACE_H
#define VGA16_GRAPHICS_3D_SURFACE_H

#include <stdint.h>

// ==========================================
// === fixed point s15x16
// ==========================================
// s15x16 fixed point macros ==
// == resolution 2^-16 = 1.5e-5
// == dynamic range is 32767/-32768
typedef signed int s15x16;
//multiply two fixed 
#define muls15x16(a,b) (((a)==0 | (b)==0)?0: \
                        ((a)==0x10000)? (b): \
                        ((b)==0x10000)? (a): \
                        (s15x16)(((( signed long long )(a))*(( signed long long )(b)))>>16)) 
//#define muls15x16(a,b) ((s15x16)(((( signed long long )(a))*(( signed long long )(b)))>>16)) 
#define float_to_s15x16(a) ((s15x16)((a)*65536.0)) // 2^16
#define s15x16_to_float(a) ((float)(a)/65536.0)
#define s15x16_to_int(a)    ((int)((a)>>16))
#define int_to_s15x16(a)    ((s15x16)((a)<<16))
#define divs15x16(a,b) (((a)==0)?0:(s15x16)((((signed long long)(a)<<16)/(b)))) 
#define abss15x16(a) abs(a)
#define one  0x00010000
#define zeropt5 0x00008000
#define swap(a,b) do{ int t = a; a = b; b = t; }while(0)
#define min(X, Y) (((X) < (Y)) ? (X) : (Y))
#define max(X, Y) (((X) < (Y)) ? (Y) : (X))

#define screen_center_x 320
#define screen_center_y 240
#define screen_width 200

// ==========================================
// === graphics math
// ==========================================

// strucdtures for object definition
// vertices and faces for triangular faces only
typedef struct vector_s{
  s15x16 x, y, z ;
} vector;

// a face is dtermined by three vertices
typedef struct face_s{
  int v0, v1, v2 ;
} face;

extern s15x16 sine[360];
extern s15x16 cosine[360];

s15x16 sqrts15x16(s15x16 v);

void Vadd(vector *v1, vector *v2, vector *r);
void Vsub(vector *v1, vector *v2, vector *r);
void VxS(vector *v, s15x16 s, vector *r);
s15x16 Vmag(vector *v);
void Vnorm(vector *v);
s15x16 Vdot(vector *v1, vector *v2);
void Vcross(vector *v1, vector *v2, vector *r);

/////////////////////////////////////////////////////////////////////
// copied from
// https://ece4760.github.io/Projects/Fall2023/av522_dy245/code.html
/////////////////////////////////////////////////////////////////////
typedef struct matrix_s {
    s15x16 m11;
    s15x16 m12;
    s15x16 m13;
    s15x16 m14;
    s15x16 m21;
    s15x16 m22;
    s15x16 m23;
    s15x16 m24;
    s15x16 m31;
    s15x16 m32;
    s15x16 m33;
    s15x16 m34;
    s15x16 m41;
    s15x16 m42;
    s15x16 m43;
    s15x16 m44;
} matrix;

void MxM(matrix *A, matrix *B, matrix *R);
int  VxM(vector *v, matrix *m, vector *out);

void build_RotX(int angle, matrix *M);
void build_RotY(int angle, matrix *M);
void build_RotZ(int angle, matrix *M);
void build_Translate(s15x16 x, s15x16 y, s15x16 z, matrix *M);
void build_Scale(s15x16 sx, s15x16 sy, s15x16 sz, matrix *M);

// define view list
// this list is traversed to actually draw
// staticllay allocated so make it big enough
#define max_view 800
typedef struct {
  int N_vertex ;
  int N_face ;
  vector vertex[max_view] ;
  face face[max_view] ;
  short color[max_view] ;
  short backface_color[max_view];
} view_s ;

extern view_s view;

void build_Tview(vector *from, vector *to, vector *approxUp, matrix *Tview);
void build_Tpersp(s15x16 d, s15x16 h, s15x16 f, matrix *Tpersp);

void view_init(void);
void view_insert(int Nv, int Nf, vector v[], face f[],
                 short color, short backface_color);

// ===== generic object template
#define CULL  -1
// using CULL as backface color surpresses backface drawing
typedef struct object_s {
  int N_vertex ;
  int N_face ;
  vector *vertex ;
  face *face ;
  short color ;
  short backface_color ;
} object ;

void build_Tetrahedron(object* tetrahedron, short color, short backface_color);
void build_Cube(object* cube, short color, short backface_color);
void build_Mesh(object* mesh, short size, short color, short backface_color);
void merge_object(object *o1, object *o2, object *dst, short color, short backface_color);

void QuickSort(s15x16 A[], int I[], int lo, int hi);

void fillTri(s15x16 x0, s15x16 y0,
             s15x16 x1, s15x16 y1,
             s15x16 x2, s15x16 y2,
             char color);

void init3D();

void free_obj(object* obj);

#endif