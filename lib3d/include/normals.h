/* $Id: normals.h,v 1.9 2001/10/23 09:29:36 aspert Exp $ */
#include <3dmodel.h>

#ifndef _NORMALS_PROTO
#define _NORMALS_PROTO


#ifdef __cplusplus
extern "C" {
#endif
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
