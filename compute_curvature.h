/* $Id: compute_curvature.h,v 1.1 2001/09/27 08:56:58 aspert Exp $ */
#ifndef _COMPUTE_CURVATURE_PROTO
#define _COMPUTE_CURVATURE_PROTO

#ifdef __cplusplus
extern "C" {
#endif

  double get_top_angle(const vertex*, const vertex*, const vertex*);
  double get_top_angle2(const vertex*, const vertex*, const vertex*);
  int obtuse_triangle(const vertex*, const vertex*, const vertex*);
  void compute_mean_curvature_normal(const model*, info_vertex*, int, 
				     const ring_info*, vertex*, double*,
				     double*);
  void compute_curvature(const model*, info_vertex*, const ring_info*);
  void compute_curvature_error(struct model_error*, struct model_error*);
#ifdef __cplusplus
}
#endif

#endif
