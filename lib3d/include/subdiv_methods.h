/* $Id: subdiv_methods.h,v 1.5 2002/02/13 12:02:40 aspert Exp $ */


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

/* These are parameters for Butterfly subdivision */
# define __QT    0.75
# define __QS    0.75
# define __2W   -0.125           /* w = -1.0/16.0 */
# define _M1_12 -0.0833333333333 /* -1.0/12.0 */
# define _5_12   0.4166666666666 /* 5.0/12.0 */

/* Precomputed regular stencil */
const static float r_sten[6] = {0.25 - __2W,  0.125 + __2W, -0.125 - __2W, 
                                __2W, -0.125 - __2W, 0.125 + __2W};

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
  void compute_midpoint_butterfly_crease(struct ring_info*, int, int, 
					 struct model*, vertex_t*);
  void compute_midpoint_loop_crease(struct ring_info*, int, int, struct model*,
				    vertex_t*);
  void update_vertices_loop_crease(struct model*, struct model*, 
				   struct ring_info*);

# ifdef __cplusplus
}
# endif

#endif
