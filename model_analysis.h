/* $Id: model_analysis.h,v 1.2 2001/08/18 15:59:26 dsanta Exp $ */

#ifndef _MODEL_ANALYSIS_PROTO
#define _MODEL_ANALYSIS_PROTO

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

/* A list of model faces */
struct face_list {
  int *face;   /* Array of indices of the faces in the list */
  int n_faces; /* Number of faces in the array */
};

/* Model analysis information */
struct model_info {
  int orientable;       /* The model is orientable, even if not currently
                         * oriented. */
  int oriented;         /* Model is currently oriented. */
  int orig_oriented;    /* Model is originally oriented. */
  int manifold;         /* Model is manifold. */
  int closed;           /* Model is closed (i.e. defines a volume) */
  int n_disjoint_parts; /* The number of disjoint (i.e. not connected) parts
                         * in the model. If there are more than one, the other
                         * fields refer to the union of all parts (i.e. if the
                         * model is manifold (or oriented, closed, etc.) all
                         * its parts are manifold, but if the model is
                         * non-manifold some parts might still be
                         * manifold). */
};

/* --------------------------------------------------------------------------*
 *                       Exported functions                                  *
 * --------------------------------------------------------------------------*/

/* Analyzes model m, returning the information in *info. The list of faces
 * incident on each vertex (as obtained by faces_of_vertex()) is given by
 * flist. If NULL a list is locally generated. If do_orient is non-zero and
 * the model is orientable, the model m will be modified so as to be oriented
 * (if the model is not orientable, no modification is done). */
void analyze_model(model *m, const struct face_list *flist,
                   struct model_info *info, int do_orient);

/* Returns an array of length m->num_vert with the list of faces incident on
 * each vertex. */
struct face_list *faces_of_vertex(const model *m);

/* Frees the storage for the array of face lists fl, of length n */
void free_face_lists(struct face_list *fl, int n);

END_DECL
#undef END_DECL

#endif /* _MODEL_ANALYSIS_PROTO */
