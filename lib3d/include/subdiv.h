/* $Id: subdiv.h,v 1.10 2003/03/27 09:42:28 aspert Exp $ */

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

#define BUTTERFLY_SUBDIV_FUNCTIONS                      \
{ SUBDIV_BUTTERFLY, compute_midpoint_butterfly,         \
    compute_midpoint_butterfly_crease, NULL }

#define LOOP_SUBDIV_FUNCTIONS                                   \
{ SUBDIV_LOOP, compute_midpoint_loop,                           \
    compute_midpoint_loop_crease, update_vertices_loop }

#define SPHERICAL_SUBDIV_FUNCTIONS                                      \
{ SUBDIV_SPH, compute_midpoint_sph, compute_midpoint_sph_crease, NULL}

#define KOBBELTSQRT3_SUBDIV_FUNCTIONS                   \
{ SUBDIV_KOB_SQRT3, compute_face_midpoint_kobsqrt3,     \
    NULL, update_vertices_kobsqrt3 }


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
