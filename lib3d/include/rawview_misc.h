/* $Id: rawview_misc.h,v 1.6 2002/10/16 12:35:02 aspert Exp $ */

#include <3dutils.h>
#include <rawview.h>
#include <gl2ps.h>
#include <image.h>
#include <curvature.h>

#ifndef _RAWVIEW_MISC_PROTO
#define _RAWVIEW_MISC_PROTO

#ifdef __cplusplus
extern "C" {
#endif

  void frame_grab(struct gl_render_context*);
  void ps_grab(struct gl_render_context*, struct display_lists_indices*, int);
  float** colormap_hsv(int);
  void free_colormap(float**);
  void set_light_on(void);
  void set_light_off(void);
  int do_normals(struct model*);
  int do_spanning_tree(struct model*);
  void setGlColor(int, float **, struct gl_render_context*);
  void rebuild_list(struct gl_render_context*, struct display_lists_indices*);
  void display_wrapper(struct gl_render_context*, 
                       struct display_lists_indices*);
  void destroy_tree(struct face_tree**, int);
  int do_curvature(struct gl_render_context*);
  
#ifdef __cplusplus
}
#endif

#endif
