/* $Id: subdiv.h,v 1.5 2002/10/31 10:26:11 aspert Exp $ */

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
		       void (*midpoint_func)(const struct ring_info*, 
                                             const int, const int, 
					     const struct model*, vertex_t*), 
                        void (*midpoint_func_bound)(const struct ring_info*, 
                                                    const int, const int, 
                                                    const struct model*, 
                                                    vertex_t*), 
		       void (*update_func)(const struct model*, struct model*, 
					   const struct ring_info*));

#ifdef __cplusplus
}
#endif
