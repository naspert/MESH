/* $Id: normals.h,v 1.3 2001/04/25 11:29:56 aspert Exp $ */
#include <3dmodel.h>

#ifndef _NORMALS_PROTO
#define _NORMALS_PROTO

#ifdef _CPLUSPLUS
extern "C" {
#endif
  int list_face_excl(model*, int, int, int);
  int* list_faces(model*, int, int);
  ring_info build_star2(model*, int);
  int manifold_edges(model*, face_tree_ptr*);
  int test_cycle(face_tree_ptr, int);
  edge_list_ptr add_edge(edge_list_ptr, int, int, int, int);
  int find_adj_edges(model*, int, edge_list_ptr);
  void destroy_tree(face_tree_ptr);
  face_tree_ptr* bfs_build_spanning_tree(model*);
  int find_center(face*, int, int);
  void swap_vert(edge_v*);
  void update_child_edges(face_tree_ptr, int, int, int);
  void build_normals(model*, face_tree_ptr, vertex*);
  vertex* compute_face_normals(model*);
  void compute_vertex_normal(model*, info_vertex*, vertex*);
#ifdef _CPLUSPLUS
}
#endif

#endif
