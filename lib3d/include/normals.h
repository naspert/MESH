/* $Id: normals.h,v 1.15 2002/04/16 06:42:23 aspert Exp $ */
#include <3dmodel.h>

#ifndef _NORMALS_PROTO
#define _NORMALS_PROTO

typedef int bitmap_t;
#define BITMAP_T_SZ (sizeof(bitmap_t))
#define BITMAP_T_BITS (8*BITMAP_T_SZ)
#define BITMAP_T_MASK (BITMAP_T_BITS-1)
#define BITMAP_TEST_BIT(bm, i) \
        ((bm)[(i)/BITMAP_T_BITS] & (1 << ((i)&BITMAP_T_MASK)))
#define BITMAP_SET_BIT(bm, i) \
        ((bm)[(i)/BITMAP_T_BITS] |= 1 << ((i)&BITMAP_T_MASK))

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
  bitmap_t *done;
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
  void build_star_global(const struct model*, struct ring_info*);
  void build_star(const struct model*, int, struct ring_info*);
  int compar(const void*, const void*);
  void add_edge_dg(struct dual_graph_info*, const struct edge_sort*, 
		   const struct edge_sort*);
  int build_edge_list(const struct model*, struct dual_graph_info*, 
		      struct info_vertex*, struct dual_graph_index **);
  struct edge_list* find_dual_edges(const int, int*, struct dual_graph_info*, 
                                    struct edge_list*, 
                                    const struct dual_graph_index*);
  struct face_tree** bfs_build_spanning_tree(const struct model*, 
					     struct info_vertex*);
  int find_center(const face_t*, const int, const int);
  void update_child_edges(struct face_tree*, const int, 
                          const int, const int);
  void build_normals(const struct model*, struct face_tree*, vertex_t*);
  vertex_t* compute_face_normals(const struct model*, struct info_vertex*);
  void compute_vertex_normal(struct model*, const struct info_vertex*, 
                             const vertex_t*);
#ifdef __cplusplus
}
#endif


#endif
