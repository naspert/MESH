/* $Id: rawview.h,v 1.3 2002/06/05 09:30:55 aspert Exp $ */

#include <GL/gl.h>

#ifndef _RAWVIEW_PROTO_
#define _RAWVIEW_PROTO_

/* Global parameters */

/* display normals with length=5% of the bounding box */
#define NORMALS_DISPLAY_FACTOR 5.0e-2 
/* step for rotation motion */
#define ANGLE_STEP 0.5
/* step for forward/backward motion */
#define TRANSL_STEP 1.0e-2
/* Field of view (in degrees) */
#define FOV 40.0


/* Light specification */
static const GLfloat amb[] = {0.1, 0.1, 0.1, 1.0};
static const GLfloat dif[] = {1.0, 1.0, 1.0, 1.0};
static const GLfloat spec[] = {1.0, 1.0, 1.0, 1.0};
/* Global ambient light */
static const GLfloat amb_light[] = {0.9, 0.9, 0.9, 1.0};
/* Material specifications */
static const GLfloat mat_spec[] = {0.33, 0.33, 0.52, 1.0};
static const GLfloat mat_diff[] = {0.43, 0.47, 0.54, 1.0};
static const GLfloat mat_amb[] = {0.11, 0.06, 0.11, 1.0};  
static const GLfloat shine[] = {10.0};


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
  int disp_curv; /* 0 -> normal 1 -> Gauss. curv. 2 -> Mean curv. */
  int wf_bc; /* draw wireframe w. backface cull. */
  int ps_rend; /* 0 -> render to screen, 1 -> render to a PS */
  
  
  int normals_done;
  int curv_done;
  double max_kg;
  double min_kg;
  double max_km;
  double min_km;
  struct model *raw_model;
  struct info_vertex *info;
  char *in_filename;
  int grab_number, ps_number;
};

#endif
