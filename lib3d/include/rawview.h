/* $Id: rawview.h,v 1.1 2002/06/04 11:54:57 aspert Exp $ */

#include <GL/gl.h>

#ifndef _RAWVIEW_PROTO_
#define _RAWVIEW_PROTO_

struct mouse_state {
  int oldx, oldy;
  int left_button_state;
  int middle_button_state;
  int right_button_state;
};

struct display_lists_indices {
  GLuint model_list; /* display list idx storage for the model itself*/
  GLuint normal_list; /* d.l. for the normals (if drawn) */
  GLuint tree_list; /* d.l. for the spanning tree (if drawn) */
  GLuint char_list; /* display lists for the fonts (when drawing vtx
                         * labels) */
};



struct gl_render_context {
  GLfloat distance, dstep; /* distance and incremental distance step */
  GLdouble mvmatrix[16]; /* Buffer for GL_MODELVIEW_MATRIX */

  int tr_mode; /* 1 -> draw triangles, 0 -> draw points */
  
  int draw_normals;
  int draw_vtx_labels;
  int draw_spanning_tree;
  int wf_bc; /* draw wireframe w. backface cull. */
  int ps_rend; /* 0 -> render to screen, 1 -> render to a PS */
  
  
  int normals_done;
  struct model *raw_model;
  char *in_filename;
  int grab_number, ps_number;
};

#endif
