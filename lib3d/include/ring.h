/* $Id: ring.h,v 1.4 2003/03/04 14:44:00 aspert Exp $ */


/*
 *
 *  Copyright (C) 2001-2003 EPFL (Swiss Federal Institute of Technology,
 *  Lausanne)
 *  You are not allowed to redistribute this program without the explicit
 *  permission of the author.
 *
 *  Author : Nicolas Aspert
 *
 *  Contact : 
 *     Nicolas Aspert
 *     Signal Processing Institute (ITS)
 *     Swiss Federal Institute of Technology (EPFL)
 *     1015 Lausanne
 *     Switzerland
 *
 *     Tel : +41 21 693 3632
 *     E-Mail : Nicolas.Aspert@epfl.ch
 *
 *
 */


#include <3dmodel.h>

#ifndef _RING_PROTO
#define _RING_PROTO

struct ring_info {
  int *ord_vert; /* ordered list of vertex */
  int type; /* 0=regular 1=boundary 2=non-manifold -1=emtpy*/
  int size;
  int n_faces;
  int *ord_face;
};

struct edge_v { 
   int v0; 
   int v1; 
  int face;
}; 

#ifdef __cplusplus
extern "C" {
#endif

  void build_star_global(const struct model*, struct ring_info*);
  void build_star(const struct model*, int, struct ring_info*);

#ifdef __cplusplus
}
#endif

#endif
