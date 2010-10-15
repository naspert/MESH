/* $Id$ */

#include <3dmodel.h>
#include <ring.h>

#ifndef _CURV_PROTO
#define _CURV_PROTO

struct vertex_curvature {
  double mixed_area;
  double gauss_curv, mean_curv;
  vertex_t mean_curv_normal;
  double c[3];
  double k1, k2; /* principal curvature */
  vertex_t t1,t2; /* principal directions (if any) */
};

#ifdef __cplusplus
extern "C" {
#endif


  int compute_curvature_with_rings(const struct model*, 
                                   struct vertex_curvature*,
                                   const struct ring_info*);
  int compute_curvature(const struct model*, struct vertex_curvature*);

#ifdef __cplusplus
}
#endif

#endif
