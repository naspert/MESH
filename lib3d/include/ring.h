/* $Id: ring.h,v 1.1 2002/05/13 13:50:46 aspert Exp $ */
#include <3dmodel.h>

#ifndef _RING_PROTO
#define _RING_PROTO

struct ring_info {
  int *ord_vert; /* ordered list of vertex */
  int type; /* 0=regular 1=boundary 2=non-manifold */
  int size;
  int n_faces;
  int *ord_face;
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
