/* $Id: curvature.h,v 1.3 2002/06/05 09:28:09 aspert Exp $ */

#include <3dmodel.h>
#include <ring.h>

#ifndef _CURV_PROTO
#define _CURV_PROTO


#ifdef __cplusplus
extern "C" {
#endif


  int compute_curvature_with_rings(const struct model*, struct info_vertex*,
                                    const struct ring_info*);
  int compute_curvature(const struct model*, struct info_vertex*);

#ifdef __cplusplus
}
#endif

#endif
