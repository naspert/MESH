/* $Id: subdiv.h,v 1.4 2002/05/27 15:52:05 aspert Exp $ */

#include <3dmodel.h>
#include <ring.h>

struct midpoint_info {
  int size;
  int *midpoint_idx; /* stores the midpoint idx for each edge of the 1-ring */
  vertex_t *midpoint;
};

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
