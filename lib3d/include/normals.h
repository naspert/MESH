/* $Id: normals.h,v 1.11 2002/02/21 13:35:24 aspert Exp $ */
#include <3dmodel.h>

#ifndef _NORMALS_PROTO
#define _NORMALS_PROTO

struct ring_info {
  int *ord_vert; /* ordered list of vertex */
  int type; /* 0=regular 1=boundary 2=non-manifold */
  int size;
  int n_faces;
  int *ord_face;
};

struct edge_sort {
  struct edge_v prim;
  int face;
};

struct edge_dual {
  int face0;
  int face1;
  struct edge_v common;
};

struct dual_graph_info {
  int num_edges_dual;
  struct edge_dual *edges;
  unsigned char *done;
};

struct dual_graph_index {
  int ring[3]; 
  int face_info; /* number of neighb. faces */
};


struct edge_list {
  struct edge_dual edge;
  struct edge_list *next;
  struct edge_list *prev;
};

#ifdef __cplusplus
extern "C" {
#endif
  void build_star_global(struct model*, struct ring_info**);
  void build_star(struct model*, int, struct ring_info*);
  void destroy_tree(struct face_tree*);
  int compar(const void*, const void*);
  void add_edge_dg(struct dual_graph_info*, const struct edge_sort*, 
		   const struct edge_sort*);
  int build_edge_list(struct model*, struct dual_graph_info*, 
		      struct info_vertex*, struct dual_graph_index **);
  struct edge_list* find_dual_edges(int, int*, struct dual_graph_info*, 
				struct edge_list*, struct dual_graph_index*);
  struct face_tree** bfs_build_spanning_tree(struct model*, 
					     struct info_vertex*);
  int find_center(const face_t*, int, int);
  void swap_vert(struct edge_v*);
  void update_child_edges(struct face_tree*, int, int, int);
  void build_normals(struct model*, struct face_tree*, vertex_t*);
  vertex_t* compute_face_normals(struct model*, struct info_vertex*);
  void compute_vertex_normal(struct model*, struct info_vertex*, vertex_t*);
#ifdef __cplusplus
}
#endif


#endif
