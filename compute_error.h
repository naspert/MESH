/* $Id: compute_error.h,v 1.6 2001/08/07 15:16:53 dsanta Exp $ */
#include <3dmodel.h>
#include <geomutils.h>

#ifndef _COMPUTE_ERROR_PROTO
#define _COMPUTE_ERROR_PROTO

#ifdef __cplusplus
#define BEGIN_DECL extern "C" {
#define END_DECL }
#else
#define BEGIN_DECL
#define END_DECL
#endif

BEGIN_DECL
#undef BEGIN_DECL

/* A list of samples of a surface in 3D space. */
struct sample_list {
  vertex* sample; /* Array of sample 3D coordinates */
  int n_samples;  /* The number of samples in the array */
};

/* A list of cells */
struct cell_list {
  int *cell;   /* The array of the linear indices of the cells in the list */
  int n_cells; /* The number of elemnts in the array */
};

/* A integer size in 3D */
struct size3d {
  int x; /* Number of elements in the X direction */
  int y; /* Number of elements in the Y direction */
  int z; /* Number of elements in the Z direction */
};

/* A list of model faces */
struct face_list {
  int *face;   /* Array of indices of the faces in the list */
  int n_faces; /* Number of faces in the array */
};

/* A list of triangles with their associated information */
struct triangle_list {
  int n_triangles;                 /* The number of triangles */
  struct triangle_info *triangles; /* The triangles */
};

/* A triangle and useful associated information. If a vertex of the triangle
 * has an angle of 90 degrees or more, that vertex is C. That way the
 * projection of C on AB is always inside AB. */
struct triangle_info {
  vertex a;            /* The A vertex of the triangle */
  vertex b;            /* The B vertex of the triangle */
  vertex c;            /* The C vertex of the triangle. The projection of C
                        * on AB is always inside the AB segment. */
  vertex ab;           /* The AB vector */
  vertex ac;           /* The AC vector */
  vertex bc;           /* The BC vector */
  double ab_len_sqr;   /* The square of the length of AB */
  double ac_len_sqr;   /* The square of the length of AC */
  double bc_len_sqr;   /* The square of the length of BC */
  double ab_1_len_sqr; /* One over the square of the length of AB */
  double ac_1_len_sqr; /* One over the square of the length of AC */
  double bc_1_len_sqr; /* One over the square of the length of BC */
  vertex d;            /* The perpendicular projection of C on AB. Always in
                        * the AB segment. */
  vertex dc;           /* The DC vector */
  double dc_1_len_sqr; /* One over the square of the length of DC */
  double da_1_max_coord; /* One over the max (in absolute value) coordinate of
                          * DA.  */
  double db_1_max_coord; /* One over the max (in absolute value) coordinate of
                          * DB */
  int da_max_c_idx;    /* The index of the coordinate corresponding to
                        * da_max_coord: 0 for X, 1 for Y, 2 for Z*/
  int db_max_c_idx;    /* The index of the coordinate corresponding to
                        * db_max_coord: 0 for X, 1 for Y, 2 for Z*/
  vertex normal;       /* The (unit length) normal of the ABC triangle
                        * (orinted with the right hand rule turning from AB to
                        * AC). */
};

struct triangle_list* model_to_triangle_list(const model *m);

void sample_triangle(const vertex *a, const vertex *b, const vertex *c,
                     int n, struct sample_list* s);

struct cell_list *cells_in_triangles(struct triangle_list *tl,
                                     struct size3d grid_sz, double cell_sz,
                                     vertex bbox_min);

int** triangles_in_cells(const struct cell_list *cl,
                         const struct triangle_list *tl,
                         const struct size3d grid_sz);

double dist_pt_surf(vertex p, const struct triangle_list *tl,
                    int **faces_in_cell, struct size3d grid_sz,
                    double cell_sz, vertex bbox_min);

double error_stat_triag(double **mem_err, int n);

struct face_list *faces_of_vertex(model *m);

END_DECL
#undef END_DECL

#endif /* _COMPUTE_ERROR_PROTO */
