/* $Id: normals.h,v 1.6 2001/09/03 11:14:44 aspert Exp $ */
#include <3dmodel.h>

#ifndef _NORMALS_PROTO
#define _NORMALS_PROTO


#ifdef __cplusplus
extern "C" {
#endif
  ring_info build_star2(model*, int);
  void destroy_tree(face_tree_ptr);
  int compar(const void*, const void*);
  edge_list_ptr add_edge_dg(edge_list_ptr, edge_sort, edge_sort);
  int build_edge_list(model*, edge_list_ptr*, info_vertex*, 
		      struct dual_graph_info**);
  edge_list_ptr find_dual_edges(int, int*, int*, edge_list_ptr*, 
				edge_list_ptr, struct dual_graph_info*);
  face_tree_ptr* bfs_build_spanning_tree(model*, info_vertex*);
  int find_center(face*, int, int);
  void swap_vert(edge_v*);
  void update_child_edges(face_tree_ptr, int, int, int);
  void build_normals(model*, face_tree_ptr, vertex*);
  vertex* compute_face_normals(model*, info_vertex*);
  void compute_vertex_normal(model*, info_vertex*, vertex*);
#ifdef __cplusplus
}
#endif


#endif
