/* $Id: compute_error.h,v 1.13 2001/09/10 15:07:43 dsanta Exp $ */
#ifndef _COMPUTE_ERROR_PROTO
#define _COMPUTE_ERROR_PROTO

#ifdef __cplusplus
#define BEGIN_DECL extern "C" {
#define END_DECL }
#else
#define BEGIN_DECL
#define END_DECL
#endif

/* --------------------------------------------------------------------------*
 *                         External includes                                 *
 * --------------------------------------------------------------------------*/

#include <3dmodel.h>

BEGIN_DECL
#undef BEGIN_DECL

/* --------------------------------------------------------------------------*
 *                       Exported data types                                 *
 * --------------------------------------------------------------------------*/

/* A integer size in 3D */
struct size3d {
  int x; /* Number of elements in the X direction */
  int y; /* Number of elements in the Y direction */
  int z; /* Number of elements in the Z direction */
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
  int m1_samples;   /* Total number of samples taken on model 1 */
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

/* --------------------------------------------------------------------------*
 *                       Exported functions                                  *
 * --------------------------------------------------------------------------*/

/* Calculates the distance from model m1 to model m2. The triangles of m1 are
 * sampled so that the maximum distance between samples in any triangle side
 * is sampling_step. The per face (of m1) error metrics are returned in a new
 * array (of length m1->num_faces) allocated at *fe_ptr. The overall distance
 * metrics and other statistics are returned in stats. Optionally, if
 * calc_normals is non-zero and m2 has no normals or face normals, the normals
 * will be calculated and added to m2 (only normals, not face normals). The
 * normals are calculated assuming that the model m2 is oriented, if it is not
 * the case the resulting normals can be incorrect. Information already used
 * to calculate the distance is reused to compute the normals, so it is very
 * fast. If quiet is zero a progress meter is displayed in stdout. The memory
 * allocated at *fe_ptr should be freed by calling
 * free_face_error(*fe_ptr). */
void dist_surf_surf(const model *m1, model *m2, double sampling_step,
                    struct face_error *fe_ptr[],
                    struct dist_surf_surf_stats *stats, int calc_normals,
                    int quiet);


/* Frees the memory allocated by dist_surf_surf() for the per face error
 * metrics. */
void free_face_error(struct face_error *fe);

END_DECL
#undef END_DECL

#endif /* _COMPUTE_ERROR_PROTO */
