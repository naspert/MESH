/* $Id: compute_error.h,v 1.25 2002/02/20 23:43:50 dsanta Exp $ */
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
  double *serror;        /* The error at each sample of the face. For a
                          * triangle with vertices v0 v1 and v2 (in that
                          * order) the samples (i,j) appear in the following
                          * order. First the errors at samples with i equal 0
                          * and j from 0 to sample_freq-1, followed by errors
                          * at samples i equal 1 and j from 0 to
                          * sample_freq-2, and so on. The index i corresponds
                          * to the v0-v1 direction and j to the v0-v2
                          * direction. If sample_freq is larger than 1, the
                          * error at v0 is serror[0], at v1 is
                          * serror[sample_freq*(sample_freq+1)/2-1] and at v2
                          * is serror[sample_freq-1]. */
  int sample_freq;       /* The sampling frequency for this triangle. If zero,
                          * no error was calculated. The number of samples is
                          * sample_freq*(sample_freq+1)/2. */
};

/* Model and error, plus miscellaneous model properties */
struct model_error {
  struct model *mesh;     /* The 3D model mesh */
  int n_samples;          /* Number of samples used to calculate the error */
  struct face_error *fe;  /* The per-face error metrics. NULL if not
                           * present. The fe[i].serror arrays are all parts of
                           * one array, starting at fe[0].serror and can thus
                           * be accessed linearly. */
  double min_error;       /* The minimum error value (at sample) */
  double max_error;       /* The maximum error value (at sample) */
  double mean_error;      /* The mean error value */
  float *verror;          /* The per vertex error array. NULL if not
                           * present. */
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

/* Calculates the distance from model me1->mesh (m1) to model m2. The
 * triangles of m1 are sampled so that the sampling density (number of samples
 * per unit surface) is sampling_density. If min_sample_freq is non-zero, all
 * triangles must have at least min_sample_freq as their sample frequency,
 * even if the specified sampling density is too low for that. The per face
 * (of m1) error metrics are returned in a new array (of length m1->num_faces)
 * allocated at me1->fe. The overall distance metrics and other statistics are
 * returned in stats. Optionally, if calc_normals is non-zero and m2 has no
 * normals or face normals, the normals will be calculated and added to m2
 * (only normals, not face normals). The normals are calculated assuming that
 * the model m2 is oriented, if it is not the case the resulting normals can
 * be incorrect. Information already used to calculate the distance is reused
 * to compute the normals, so it is very fast. If prog in not NULL it is used
 * for reporting progress. The memory allocated at me1->fe should be freed by
 * calling free_face_error(me1->fe). Note that non-zero values for
 * min_sample_freq distort the uniform distribution of error samples. */
void dist_surf_surf(struct model_error *me1, struct model *m2, 
		    double sampling_density, int min_sample_freq,
                    struct dist_surf_surf_stats *stats, int calc_normals,
                    struct prog_reporter *prog);


/* Frees the memory allocated by dist_surf_surf() for the per face error
 * metrics. */
void free_face_error(struct face_error *fe);

/* Stores the error values at each vertex in the me->verror array (realloc'ed
 * to the correct size), given the per face error metrics in me->fe. A
 * negative error (special flag) is assigned to vertices for which there are
 * no sample points. The number of vertices and faces without error samples is
 * returned in *nv_empty and *nf_empty, respectively. */
void calc_vertex_error(struct model_error *me, int *nv_empty, int *nf_empty);

END_DECL
#undef END_DECL

#endif /* _COMPUTE_ERROR_PROTO */
