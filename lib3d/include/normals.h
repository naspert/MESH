/* $Id: normals.h,v 1.19 2003/03/04 14:44:00 aspert Exp $ */
#include <3dmodel.h>
#include <ring.h>

#ifndef _NORMALS_PROTO
#define _NORMALS_PROTO

typedef int bitmap_t;
#define BITMAP_T_SZ (sizeof(bitmap_t))
#define BITMAP_T_BITS (8*BITMAP_T_SZ)
#define BITMAP_T_MASK (BITMAP_T_BITS-1)
#define BITMAP_TEST_BIT(bm, i) \
        ((bm)[(i)/BITMAP_T_BITS] & (1 << ((i) & BITMAP_T_MASK)))
#define BITMAP_SET_BIT(bm, i) \
        ((bm)[(i)/BITMAP_T_BITS] |= 1 << ((i) & BITMAP_T_MASK))


struct edge_dual {
  int face0;
  int face1;
  struct edge_prim common;
};

struct dual_graph_info {
  int num_edges_dual;
  struct edge_dual *edges;
  bitmap_t *done;
};

struct dual_graph_index {
  int *ring; 
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
  struct face_tree** bfs_build_spanning_tree(const struct model*, 
					     struct ring_info*);
  vertex_t* compute_face_normals(const struct model*, struct ring_info*);
  void compute_vertex_normal(struct model*, const struct ring_info*, 
                             const vertex_t*);
#ifdef __cplusplus
}
#endif


#endif
