/* $Id: compute_error.h,v 1.7 2001/08/08 13:18:49 dsanta Exp $ */
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

/* Per face error metrics */
struct face_error {
  double face_area;      /* Area of the face, for error weighting in averages */
  double min_error;      /* The minimum error for the face */
  double max_error;      /* The maximum error for the face */
  double mean_error;     /* The mean error for the face */
  double mean_sqr_error; /* The mean squared error for the face */
};

/* Statistics from the dist_surf_surf function */
struct dist_surf_surf_stats {
  double m1_area;   /* Area of model 1 surface */
  double m2_area;   /* Area of model 2 surface */
  double min_dist;  /* Minimum distance from model 1 to model 2 */
  double max_dist;  /* Maximum distance from model 1 to model 2 */
  double mean_dist; /* Mean distance from model 1 to model 2 */
  double rms_dist;  /* Root mean squared distance from model 1 to model 2 */
  double cell_sz;   /* The partitioning cubic cell side length */
  struct size3d grid_sz; /* The number of cells in the partitioning grid in
                          * each direction X,Y,Z */
};

/* Storage for triangle sample errors. */
struct triag_sample_error {
  double **err;      /* Error array with 2D addressing. Sample (i,j) has the
                      * error stored at err[i][j], where i varies betwen 0 and
                      * n_samples-1 inclusive and j varies between 0 and
                      * n_samples-i-1 inclusive. */
  int n_samples;     /* The number of samples in each triangle direction */
  double *err_lin;   /* Error array with 1D adressing, which varies from 0 to
                      * n_samples_tot-1 inclusive. It refers to the same
                      * location as err, thus any change to err is reflected
                      * in err_lin and vice-versa. The order in the 1D array
                      * is all errors for i equal 0 and j from 0 to
                      * n_samples-1, followed by errors for i equal 1 and j
                      * from 1 to n_samples-2, and so on. */
  int n_samples_tot; /* The total number of samples in the triangle */
};

/* A list of triangles with their associated information */
struct triangle_list {
  struct triangle_info *triangles; /* The triangles */
  int n_triangles;                 /* The number of triangles */
  double area;                     /* The total triangle area */
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

void error_stat_triag(const struct triag_sample_error *tse,
                      struct face_error *fe);

struct face_list *faces_of_vertex(model *m);

void dist_surf_surf(const model *m1, const model *m2, int n_spt,
                    struct face_error *fe_ptr[],
                    struct dist_surf_surf_stats *stats, int quiet);

END_DECL
#undef END_DECL

#endif /* _COMPUTE_ERROR_PROTO */
