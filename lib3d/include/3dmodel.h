/* $Id: 3dmodel.h,v 1.2 2001/03/20 11:38:34 aspert Exp $ */
#ifdef MEM_DEBUG
#include <purify.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include <float.h>

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
  int num_faces;
  int num_vert;
  int builtin_normals; /* 1 if normals are already in the file 0 otherwise */
  vertex* vertices;
  vertex* normals; /* Normals for each vertex of the model */
  vertex *face_normals;
  face* faces;
  double* area; /* area of each face */
  vertex bBox[2]; /* bBox[0] is the min  bBox[1] is the max */
}model;


typedef struct {
  int outdegree;
  int *list;
}star;

typedef struct {
  int num_faces;
  int *list_face; /* Index of faces containing the vertex */
  double *weight; /* weight of neighbouring vertices */
  int outdegree;
  int *list_vertex; /* Index of vertices neighbouring the current vertex */
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
}edge;


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

typedef struct dual_list *edge_list_ptr;

typedef struct dual_list {
  edge_dual edge;
  edge_list_ptr next;
}edge_list;

typedef struct {
  int *ord_vert; /* ordered list of vertex */
  int type; /* 0=regular 1=boundary 2=non-manifold */
  int size;
}ring_info;

typedef struct {
  edge_v edge;
  vertex p;
}edge_sub;

#endif
