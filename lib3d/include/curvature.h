/* $Id: curvature.h,v 1.1 2002/06/04 09:17:33 aspert Exp $ */

#ifndef _CURV_PROTO
#define _CURV_PROTO

#ifdef __cplusplus
extern "C" {
#endif


  void compute_curvature(const struct model*, struct info_vertex*,
                         const struct ring_info*);

#ifdef __cplusplus
}
#endif

#endif
