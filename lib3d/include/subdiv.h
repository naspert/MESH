/* $Id: subdiv.h,v 1.3 2002/05/13 13:50:46 aspert Exp $ */

#include <3dmodel.h>
#include <ring.h>

#ifdef __cplusplus
extern "C" {
#endif

  struct model* subdiv(struct model*, 
		       void (*midpoint_func)(struct ring_info*, int, int, 
					     struct model*, vertex_t*), 
                        void (*midpoint_func_bound)(struct ring_info*, int, 
                                                    int, struct model*, 
                                                    vertex_t*), 
		       void (*update_func)(struct model*, struct model*, 
					   struct ring_info*));

#ifdef __cplusplus
}
#endif
