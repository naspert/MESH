/* $Id: 3dmodel.h,v 1.16 2001/09/27 08:56:59 aspert Exp $ */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
/* What follows is to be able to compile */
/* with '-ansi' flag and still use the */
/* useful constants from 'math.h' */
#ifndef __USE_BSD
# define __USE_BSD
#endif
#include <math.h>
#include <limits.h>
#include <float.h>

/* Those constants are not in math.h under Window$ */
#ifdef _WIN32 
# define M_E		2.7182818284590452354	/* e */
# define M_LOG2E	1.4426950408889634074	/* log_2 e */
# define M_LOG10E	0.43429448190325182765	/* log_10 e */
# define M_LN2		0.69314718055994530942	/* log_e 2 */
# define M_LN10		2.30258509299404568402	/* log_e 10 */
# define M_PI		3.14159265358979323846	/* pi */
# define M_PI_2		1.57079632679489661923	/* pi/2 */
# define M_PI_4		0.78539816339744830962	/* pi/4 */
# define M_1_PI		0.31830988618379067154	/* 1/pi */
# define M_2_PI		0.63661977236758134308	/* 2/pi */
# define M_2_SQRTPI	1.12837916709551257390	/* 2/sqrt(pi) */
# define M_SQRT2	1.41421356237309504880	/* sqrt(2) */
# define M_SQRT1_2	0.70710678118654752440	/* 1/sqrt(2) */
#endif

#ifndef _3D_MODEL
#define _3D_MODEL

typedef struct {
  double x;
  double y;
  double z;
}vertex;

typedef struct {
  int f0;
  int f1;
  int f2;
}face;




typedef struct {
  int outdegree;
  int *list;
}star;

typedef struct {
  int num_faces;
  int *list_face; /* Index of faces containing the vertex */
  int outdegree;
  int *list_vertex; /* Index of vertices neighbouring the current vertex */
  double mixed_area;
  double gauss_curv;
  vertex mean_curv_normal;
  double c[3];
  double k1, k2; /* principal curvature */
  vertex t1,t2; /* principal directions (if any) */
}info_vertex;


typedef struct {
  int v0;
  int v1;
}edge_v;



typedef struct {
  int face0;
  int face1;
  edge_v common;
}edge_dual;


typedef struct {
  int s,t; /* An edge contains 2 vertices */
  int l,r;/*left & right faces index*/
  /* l,r = -1 -> undefined face...*/
}edge_tr;

typedef struct {
  edge_v prim;
  int face;
}edge_sort;


typedef struct fnode *face_tree_ptr;

typedef struct fnode {
  int face_idx;
  int visited;
  int node_type; /* 0 -> left_child 1 -> right_child */
  edge_v prim_left;
  edge_v prim_right;
  face_tree_ptr left;
  face_tree_ptr right;
  face_tree_ptr parent;
  int v0,v1,v2;
}face_tree;


typedef struct {
  int num_faces;
  int num_vert;
  int builtin_normals; /* 1 if normals are already in the file 0 otherwise */
  vertex* vertices;
  vertex* normals; /* Normals for each vertex of the model */
  vertex *face_normals;
  face* faces;
  double* area; /* area of each face */
  double total_area;
  vertex bBox[2]; /* bBox[0] is the min  bBox[1] is the max */
  face_tree_ptr *tree; /* spanning tree of the dual graph */
#ifdef EST_NORMALS
  vertex *est_normals;
#endif
}model;


struct dual_graph_info {
  int num_edges_dual;
  edge_dual *edges;
  int *done;
};

struct dual_graph_index {
  int ring[3]; 
  int face_info; /* number of neighb. faces */
};

typedef struct dual_list *edge_list_ptr;

typedef struct dual_list {
  edge_dual edge;
  edge_list_ptr next;
  edge_list_ptr prev;
}edge_list;

typedef struct {
  int *ord_vert; /* ordered list of vertex */
  int type; /* 0=regular 1=boundary 2=non-manifold */
  int size;
}ring_info;


typedef struct {
  edge_v edge;
  vertex p;
#ifdef EST_NORMALS
  vertex n;
#endif
}edge_sub;


#endif
