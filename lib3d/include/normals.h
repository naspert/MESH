/* $Id: normals.h,v 1.7 2001/09/24 11:59:27 aspert Exp $ */
#include <3dmodel.h>

#ifndef _NORMALS_PROTO
#define _NORMALS_PROTO


#ifdef __cplusplus
extern "C" {
#endif
  void build_star(model*, int, ring_info*);
  void destroy_tree(face_tree_ptr);
  int compar(const void*, const void*);
  edge_list_ptr add_edge_dg(edge_list_ptr, const edge_sort*, const edge_sort*);
  int build_edge_list(model*, edge_list_ptr*, info_vertex*, 
		      struct dual_graph_info**);
  edge_list_ptr find_dual_edges(int, int*, int*, edge_list_ptr*, 
				edge_list_ptr, struct dual_graph_info*);
  face_tree_ptr* bfs_build_spanning_tree(model*, info_vertex*);
  int find_center(const face*, int, int);
  void swap_vert(edge_v*);
  void update_child_edges(face_tree_ptr, int, int, int);
  void build_normals(model*, face_tree_ptr, vertex*);
  vertex* compute_face_normals(model*, info_vertex*);
  void compute_vertex_normal(model*, info_vertex*, vertex*);
#ifdef __cplusplus
}
#endif


#endif
