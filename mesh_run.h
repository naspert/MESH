/* $Id: mesh_run.h,v 1.4 2001/11/06 10:38:26 dsanta Exp $ */

#ifndef _MESH_RUN_PROTO
#define _MESH_RUN_PROTO

#include <compute_error.h>
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

/* To store the parsed arguments */
struct args {
  char *m1_fname; /* filename of model 1 */
  char *m2_fname; /* filename of model 2 */
  int  no_gui;    /* text only flag */
  int quiet;      /* do not display extra info flag*/
  double sampling_step; /* The sampling step, as fraction of the bounding box
                         * diagonal of model 2. */
  int do_symmetric; /* do symmetric error measure */
  int do_curvature;
  int do_wlog;
};

/* Runs the mesh program, given the parsed arguments in *args. The models and
 * their respective errors are returned in *model1 and *model2. If
 * args->no_gui is zero a QT window is opened to display the visual
 * results. All normal (non error) output is printed through the output buffer
 * out. If not NULL the progress object is used to report the progress. */
void mesh_run(const struct args *args, struct model_error *model1,
              struct model_error *model2, struct outbuf *out,
              struct prog_reporter *progress);

END_DECL
#undef END_DECL

#endif /* _MESH_RUN_PROTO */
