/* $Id: geomutils.h,v 1.7 2001/07/02 09:22:43 aspert Exp $ */
#include <3dmodel.h>

#ifndef _GEOMUTILS_PROTO
#define _GEOMUTILS_PROTO

#ifdef __cplusplus
extern "C" {
#endif

/* a few useful macros */
#ifndef min
#define min(__X, __Y) ((__X)<(__Y)?(__X):(__Y))
#endif
#ifndef max
#define max(__X, __Y) ((__X)>(__Y)?(__X):(__Y))
#endif
#ifndef max3
#define max3(__X,__Y,__Z) max((__X), max((__Y), (__Z)))
#endif
#ifndef min3
#define min3(__X,__Y,__Z) min((__X), min((__Y),(__Z)))
#endif


  double dist(vertex, vertex);
  vertex ncrossp(vertex, vertex, vertex);
  vertex crossprod(vertex, vertex);
  double cross_product2d(vertex, vertex, vertex);
  double scalprod(vertex, vertex);
  double norm(vertex);
  void normalize(vertex*);
  vertex rotate_3d(vertex, vertex, double);
  int inside(vertex, vertex, double);
  void compute_circle2d(vertex, vertex, vertex, double*, vertex*);
  void compute_circle3d(vertex, vertex, vertex, double*, vertex*);
  double tri_area(vertex, vertex, vertex);
#ifdef __cplusplus
}
#endif

 
#endif
