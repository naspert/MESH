/* $Id: normals.h,v 1.16 2002/05/13 13:50:46 aspert Exp $ */
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
