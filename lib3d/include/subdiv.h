/* $Id: subdiv.h,v 1.9 2003/03/24 12:16:37 aspert Exp $ */

#include <3dmodel.h>
#include <ring.h>
#include <normals.h>
#include <subdiv_methods.h>

#ifndef _SUBDIV_H_PROTO_
#define _SUBDIV_H_PROTO_
struct midpoint_info {
  int size;
  int *midpoint_idx; /* stores the midpoint idx for each edge of the
                      * 1-ring */
  bitmap_t *edge_subdiv_done;
};

struct subdiv_functions {
  int id;
  void (*midpoint_func)(const struct ring_info*, 
                        const int, const int, 
                        const struct model*, vertex_t*);
  void (*midpoint_func_bound)(const struct ring_info*, 
                              const int, const int, 
                              const struct model*, 
                              vertex_t*);
  void (*update_func)(const struct model*, struct model*, 
                      const struct ring_info*);
};


struct subdiv_sqrt3_functions {
  int id;
  void (*face_midpoint_func)(const struct ring_info*, 
                             const int, 
                             const struct model*, vertex_t*);
  void (*midpoint_func_bound)(const struct ring_info*, 
                              const int, const int, 
                              const struct model*, 
                              vertex_t*);
  void (*update_func)(const struct model*, struct model*, 
                      const struct ring_info*);
};


/* Current subdiv_methods */
struct subdiv_methods {
  struct subdiv_functions butterfly;
  struct subdiv_functions loop;
  struct subdiv_functions spherical;
  struct subdiv_sqrt3_functions kob_sqrt3;
};

#define INIT_SUBDIV_METHODS(sm)                                           \
  do {                                                                    \
    sm.butterfly.id = SUBDIV_BUTTERFLY;                                   \
    sm.butterfly.midpoint_func = compute_midpoint_butterfly;              \
    sm.butterfly.midpoint_func_bound = compute_midpoint_butterfly_crease; \
    sm.butterfly.update_func = NULL;                                      \
                                                                          \
                                                                          \
    sm.loop.id = SUBDIV_LOOP;                                             \
    sm.loop.midpoint_func = compute_midpoint_loop;                        \
    sm.loop.midpoint_func_bound = compute_midpoint_loop_crease;           \
    sm.loop.update_func = update_vertices_loop;                           \
                                                                          \
                                                                          \
    sm.spherical.id = SUBDIV_SPH;                                         \
    sm.spherical.midpoint_func = compute_midpoint_sph;                    \
    sm.spherical.midpoint_func_bound = compute_midpoint_sph_crease;       \
    sm.spherical.update_func = NULL;                                      \
                                                                          \
                                                                          \
    sm.kob_sqrt3.id = SUBDIV_KOB_SQRT3;                                   \
    sm.kob_sqrt3.face_midpoint_func = compute_face_midpoint_kobsqrt3;     \
    sm.kob_sqrt3.midpoint_func_bound = NULL;                              \
    sm.kob_sqrt3.update_func = update_vertices_kobsqrt3;                  \
  } while(0)

#ifdef __cplusplus
extern "C" {
#endif

  struct model* subdiv(struct model*, const struct subdiv_functions*);


  struct model* subdiv_sqrt3(struct model*,  
                             const struct subdiv_sqrt3_functions*);


#ifdef __cplusplus
}
#endif

#endif
