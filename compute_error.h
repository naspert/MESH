/* $Id: compute_error.h,v 1.22 2002/02/10 17:57:51 dsanta Exp $ */
#ifndef _COMPUTE_ERROR_PROTO
#define _COMPUTE_ERROR_PROTO

/* --------------------------------------------------------------------------*
 *                         External includes                                 *
 * --------------------------------------------------------------------------*/

#include <model_analysis.h>
#include <reporting.h>

#ifdef __cplusplus
#define BEGIN_DECL extern "C" {
#define END_DECL }
#else
#define BEGIN_DECL
#define END_DECL
#endif

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
  int n_samples;         /* Number of samples used to calculate error. If
                          * zero, no error was calculated */
};

/* Model and error, plus miscellaneous model properties */
struct model_error {
  struct model *mesh;     /* The 3D model mesh */
  float *verror;          /* The per vertex error array. NULL if not
                           * present. */
  float min_verror;       /* The minimum vertex error for the model */
  float max_verror;       /* The maximum vertex error for the model */

  struct model_info *info;/* The model information. NULL if not present. */
};

/* Statistics from the dist_surf_surf function */
struct dist_surf_surf_stats {
  int m1_samples;   /* Total number of samples taken on model 1 */
  double st_m1_area;/* Total area of sampled triangles of model 1 */
  double m1_area;   /* Area of model 1 surface */
  double m2_area;   /* Area of model 2 surface */
  double min_dist;  /* Minimum distance from model 1 to model 2 */
  double max_dist;  /* Maximum distance from model 1 to model 2 */
  double mean_dist; /* Mean distance from model 1 to model 2 */
  double rms_dist;  /* Root mean squared distance from model 1 to model 2 */
  double cell_sz;   /* The partitioning cubic cell side length */
  struct size3d grid_sz; /* The number of cells in the partitioning grid in
                          * each direction X,Y,Z */
  int n_ne_cells;   /* Number of non-empty cells */
  double n_t_p_nec; /* Average number of triangles per non-empty cell */
};

/* --------------------------------------------------------------------------*
 *                       Exported functions                                  *
 * --------------------------------------------------------------------------*/

/* Calculates the distance from model m1 to model m2. The triangles of m1 are
 * sampled so that the sampling density (number of samples per unit surface)
 * is sampling_density. The per face (of m1) error metrics are returned in a
 * new array (of length m1->num_faces) allocated at *fe_ptr. The overall
 * distance metrics and other statistics are returned in stats. Optionally, if
 * calc_normals is non-zero and m2 has no normals or face normals, the normals
 * will be calculated and added to m2 (only normals, not face normals). The
 * normals are calculated assuming that the model m2 is oriented, if it is not
 * the case the resulting normals can be incorrect. Information already used
 * to calculate the distance is reused to compute the normals, so it is very
 * fast. If prog in not NULL it is used for reporting progress. The memory
 * allocated at *fe_ptr should be freed by calling
 * free_face_error(*fe_ptr). */
void dist_surf_surf(const struct model *m1, struct model *m2, 
		    double sampling_step,
                    struct face_error *fe_ptr[],
                    struct dist_surf_surf_stats *stats, int calc_normals,
                    struct prog_reporter *prog);


/* Frees the memory allocated by dist_surf_surf() for the per face error
 * metrics. */
void free_face_error(struct face_error *fe);

/* Calculates the per vertex error and stores it in me, given the per face
 * error metrics in fe. The list of faces incident on each vertex should be
 * given in vfl. If NULL a temporary list is generated. */
void calc_vertex_error(struct model_error *me, const struct face_error *fe,
                       const struct face_list *vfl);

END_DECL
#undef END_DECL

#endif /* _COMPUTE_ERROR_PROTO */
