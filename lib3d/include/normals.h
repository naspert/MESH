/* $Id: normals.h,v 1.8 2001/09/27 11:44:45 aspert Exp $ */
#include <3dmodel.h>

#ifndef _NORMALS_PROTO
#define _NORMALS_PROTO


#ifdef __cplusplus
extern "C" {
#endif
  void build_star(struct model*, int, struct ring_info*);
  void destroy_tree(struct face_tree*);
  int compar(const void*, const void*);
  struct edge_list* add_edge_dg(struct edge_list*, const struct edge_sort*, 
				const struct edge_sort*);
  int build_edge_list(struct model*, struct  edge_list**, struct info_vertex*, 
		      struct dual_graph_info**);
  struct edge_list* find_dual_edges(int, int*, int*, struct edge_list**, 
				struct edge_list*, struct dual_graph_info*);
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
