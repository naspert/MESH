/* $Id: subdiv.h,v 1.8 2003/03/12 17:54:58 aspert Exp $ */

#include <3dmodel.h>
#include <ring.h>
#include <normals.h>

struct midpoint_info {
  int size;
  int *midpoint_idx; /* stores the midpoint idx for each edge of the
                      * 1-ring */
  bitmap_t *edge_subdiv_done;
};

#ifdef __cplusplus
extern "C" {
#endif

  struct model* subdiv(struct model*, const int,
		       void (*midpoint_func)(const struct ring_info*, 
                                             const int, const int, 
					     const struct model*, vertex_t*), 
                        void (*midpoint_func_bound)(const struct ring_info*, 
                                                    const int, const int, 
                                                    const struct model*, 
                                                    vertex_t*), 
		       void (*update_func)(const struct model*, struct model*, 
					   const struct ring_info*));


  struct model* subdiv_sqrt3(struct model*, const int,
                             void (*face_midpoint_func)(const struct ring_info*,
                                                        const int, 
                                                        const struct model*, 
                                                        vertex_t*), 
                             void (*midpoint_func_bound)(const struct ring_info*,
                                                         const int, const int, 
                                                         const struct model*, 
                                                         vertex_t*), 
                             void (*update_func)(const struct model*, 
                                                 struct model*, 
                                                 const struct ring_info*));
#ifdef __cplusplus
}
#endif
