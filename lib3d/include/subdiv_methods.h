/* $Id: subdiv_methods.h,v 1.17 2003/03/12 17:54:59 aspert Exp $ */
#include <3dmodel.h>
#include <ring.h>

#ifndef __SUBDIV_METHODS_PROTO
# define __SUBDIV_METHODS_PROTO


/* Identifier for subdivision method */
# define SUBDIV_BUTTERFLY          0x01
# define SUBDIV_SPH                0x02
# define SUBDIV_LOOP               0x03
# define SUBDIV_KOB_SQRT3          0x11

# ifdef __cplusplus
extern "C" {
# endif

  /* spherical subdivision */
  void compute_midpoint_sph(const struct ring_info*, const int, const int, 
                            const struct model*, vertex_t*);
  void compute_midpoint_sph_crease(const struct ring_info*, const int, 
                                   const int, const struct model*,
                                   vertex_t*);

  /* Butterfly subdivision */
  void compute_midpoint_butterfly(const struct ring_info*, const int, 
                                  const int, const struct model*, 
				  vertex_t*);
  void compute_midpoint_butterfly_crease(const struct ring_info*, const int, 
                                         const int, const struct model*, 
                                         vertex_t*);


  /* Loop subdivision */
  void compute_midpoint_loop(const struct ring_info*, const int, const int, 
                             const struct model*, vertex_t*);
  void update_vertices_loop(const struct model*, struct model*, 
                            const struct ring_info*);

  void compute_midpoint_loop_crease(const struct ring_info*, const int, 
                                    const int, const struct model*,
				    vertex_t*);

  /* kobbelt-sqrt3 subdivision */
  void compute_face_midpoint_kobsqrt3(const struct ring_info*, const int, 
                                      const struct model*, vertex_t*);
  void update_vertices_kobsqrt3(const struct model*, struct model*, 
                                const struct ring_info*);
 
# ifdef __cplusplus
}
# endif

#endif
