/* $Id: curvature.h,v 1.2 2002/06/04 14:39:11 aspert Exp $ */

#include <3dmodel.h>
#include <ring.h>

#ifndef _CURV_PROTO
#define _CURV_PROTO


#ifdef __cplusplus
extern "C" {
#endif


  void compute_curvature_with_rings(const struct model*, struct info_vertex*,
                                    const struct ring_info*);
  void compute_curvature(const struct model*, struct info_vertex*);

#ifdef __cplusplus
}
#endif

#endif
