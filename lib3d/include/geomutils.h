/* $Id: geomutils.h,v 1.4 2001/04/25 11:29:56 aspert Exp $ */
#include <3dmodel.h>

#ifndef _GEOMUTILS_PROTO
#define _GEOMUTILS_PROTO

#ifdef _CPLUSPLUS
extern "C" {
#endif
  double dist(vertex, vertex);
  vertex ncrossp(vertex, vertex, vertex);
  vertex crossprod(vertex, vertex);
  double cross_product2d(vertex, vertex, vertex);
  double scalprod(vertex, vertex);
  double norm(vertex);
  void normalize(vertex*);
  int inside(vertex, vertex, double);
  void compute_circle2d(vertex, vertex, vertex, double*, vertex*);
  void compute_circle3d(vertex, vertex, vertex, double*, vertex*);
  double tri_area(vertex, vertex, vertex);
#ifdef _CPLUSPLUS
}
#endif

 
#endif
