/* $Id: subdiv_methods.h,v 1.13 2002/10/31 10:26:12 aspert Exp $ */
#include <3dmodel.h>
#include <ring.h>

#ifndef __SUBDIV_METHODS_PROTO
# define __SUBDIV_METHODS_PROTO

/* Bitmasks for the midpoint search */
# define U0_FOUND 0x01
# define U1_FOUND 0x02
# define U2_FOUND 0x04

/* Identifier for subdivision method */
# define SUBDIV_BUTTERFLY          0x01
# define SUBDIV_SPH                0x02
# define SUBDIV_LOOP               0x03
# define SUBDIV_BUTTERFLY_BOUNDARY 0x11
# define SUBDIV_LOOP_BOUNDARY      0x13




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
  void update_vertices_loop_crease(const struct model*, struct model*, 
				   const struct ring_info*);

# ifdef __cplusplus
}
# endif

#endif
