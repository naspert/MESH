/* $Id: subdiv_methods.h,v 1.2 2001/10/16 15:31:05 aspert Exp $ */


#ifndef __SUBDIV_METHODS_PROTO
# define __SUBDIV_METHODS_PROTO

/* Bitmasks for the midpoint search */
# define U0_FOUND 0x01
# define U1_FOUND 0x02
# define U2_FOUND 0x04

/* Identifier for subdivision method */
# define SUBDIV_BUTTERFLY 0x01
# define SUBDIV_SPH       0x02
# define SUBDIV_LOOP      0x03
# define SUBDIV_LOOP_BOUNDARY 0x04

# ifdef __cplusplus
extern "C" {
# endif

  void compute_midpoint_sph(struct ring_info*, int, int, struct model*, 
			    vertex_t*);
  void compute_midpoint_butterfly(struct ring_info*, int, int, struct model*, 
				  vertex_t*);
  void compute_midpoint_loop(struct ring_info*, int, int, struct model*,
			     vertex_t*);
  void update_vertices_loop(struct model*, struct model*, struct ring_info*);
  void compute_midpoint_loop_crease(struct ring_info*, int, int, struct model*,
				    vertex_t*);
  void update_vertices_loop_crease(struct model*, struct model*, 
				   struct ring_info*);

# ifdef __cplusplus
}
# endif

#endif
