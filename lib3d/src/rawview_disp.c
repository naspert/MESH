/* $Id: rawview_disp.c,v 1.9 2002/11/14 16:45:02 aspert Exp $ */

#include <rawview_misc.h>
#ifdef DEBUG
# include <debug_print.h>
#endif

#define CMAP_LENGTH 256
void setGlColor(int vidx, float **cmap, 
                struct gl_render_context *gl_ctx) {
  double range;
  int cidx=-1;
  
  assert (cmap != NULL);

  switch (gl_ctx->disp_curv) {
  case 1: /* Gauss curv */
    range = gl_ctx->max_kg - gl_ctx->min_kg;
    cidx = (int)(CMAP_LENGTH*(gl_ctx->info[vidx].gauss_curv - gl_ctx->min_kg)/
               range);
  case 2: /* Mean curv */
    range = gl_ctx->max_km - gl_ctx->min_km;
    cidx = (int)(CMAP_LENGTH*(gl_ctx->info[vidx].mean_curv - gl_ctx->min_km)/
               range);
    break;
  default: /* should never get here */
    abort();
  }

  if (cidx >= CMAP_LENGTH)
    cidx = CMAP_LENGTH - 1;
  assert(cidx >=0 && cidx < CMAP_LENGTH); 
  glColor3fv(cmap[cidx]);

}

/* ************************************************************* */
/* This functions rebuilds the display list of the current model */
/* It is called when the viewing setting (light...) are changed  */
/* ************************************************************* */
void rebuild_list(struct gl_render_context *gl_ctx,
                  struct display_lists_indices *dl_idx) {
  int i;
  GLboolean light_mode;
  float scale_fact;
  float **cmap=NULL;
  face_t *cur_face;
  vertex_t center1, center2;
  face_t *cur_face2;
  int j, face1=-1, face2=-1;
  const struct model *r_m = gl_ctx->raw_model; /* just to make it more
                                                * readable ... */


  /* delete all lists */
  if (glIsList(dl_idx->model_list) == GL_TRUE)
    glDeleteLists(dl_idx->model_list, 1);
  
  if (glIsList(dl_idx->normal_list) == GL_TRUE)
    glDeleteLists(dl_idx->normal_list, 1);
  
  if (glIsList(dl_idx->char_list) == GL_TRUE)
    glDeleteLists(dl_idx->char_list, 256);
  
  if (glIsList(dl_idx->tree_list) == GL_TRUE)
    glDeleteLists(dl_idx->tree_list, 1);
  
  /* create display lists indices */
  dl_idx->model_list = glGenLists(1);
  if (gl_ctx->draw_spanning_tree)
    dl_idx->tree_list = glGenLists(1);
  
  if (gl_ctx->draw_normals)
    dl_idx->normal_list = glGenLists(1);

  
  if (gl_ctx->draw_vtx_labels) {
    dl_idx->char_list = glGenLists(256);
    if (dl_idx->char_list == 0)
      verbose_printf(gl_ctx->verbose, "Unable to create dl_idx->char_list\n");
    
    for (i=32; i<127; i++) { /* build display lists for labels */
      glNewList(dl_idx->char_list+i, GL_COMPILE);
      glutBitmapCharacter(GLUT_BITMAP_8_BY_13, i);
      glEndList();
    }
  }

  /* if we display curvature we need the colormap */
  if (gl_ctx->disp_curv == 1 || gl_ctx->disp_curv == 2) 
    cmap = colormap_hsv(CMAP_LENGTH);
  
  /* Get the state of the lighting */
  light_mode = glIsEnabled(GL_LIGHTING);
  

#ifdef DEBUG
  DEBUG_PRINT("dn = %d lm = %d nf = %d\n", gl_ctx->draw_normals, light_mode, 
              r_m->num_faces); 
#endif

  if (gl_ctx->draw_spanning_tree == 1) {

    glNewList(dl_idx->tree_list, GL_COMPILE);
    glColor3f(1.0, 1.0, 0.0);
    glBegin(GL_LINES);
    for (j=0; j<r_m->num_faces; j++) {
      face1 = (r_m->tree)[j]->face_idx;
      cur_face = &(r_m->faces[face1]);
      add3_sc_v(1.0/3.0, &(r_m->vertices[cur_face->f0]), 
		&(r_m->vertices[cur_face->f1]), 
		&(r_m->vertices[cur_face->f2]), &center1);
      if ((r_m->tree)[j]->left != NULL) {
	face2 = ((r_m->tree)[j]->left)->face_idx;
	cur_face2 = &(r_m->faces[face2]);
	add3_sc_v(0.333, &(r_m->vertices[cur_face2->f0]), 
		  &(r_m->vertices[cur_face2->f1]), 
		  &(r_m->vertices[cur_face2->f2]), &center2);
	glVertex3f(center1.x, center1.y, center1.z);
	glVertex3f(center2.x, center2.y, center2.z);

      }
      if ((r_m->tree)[j]->right != NULL) {
	face2 = ((r_m->tree)[j]->right)->face_idx;
	cur_face2 = &(r_m->faces[face2]);
	add3_sc_v(1.0/3.0, &(r_m->vertices[cur_face2->f0]), 
		  &(r_m->vertices[cur_face2->f1]), 
		  &(r_m->vertices[cur_face2->f2]), &center2);
	glVertex3f(center1.x, center1.y, center1.z);
	glVertex3f(center2.x, center2.y, center2.z);

      }
    }
    glEnd();
    glColor3f(1.0, 1.0, 1.0);
    glEndList();


  }

  /* Model drawing (w. or wo. lighting) */
  glNewList(dl_idx->model_list, GL_COMPILE);
  if (gl_ctx->tr_mode == 1)
    glBegin(GL_TRIANGLES);
  else 
    glBegin(GL_POINTS);
  switch (gl_ctx->disp_curv) {
  case 0:
    for (i=0; i<r_m->num_faces; i++) {
      cur_face = &(r_m->faces[i]);
      if (light_mode)
        glNormal3f(r_m->normals[cur_face->f0].x, 
                   r_m->normals[cur_face->f0].y, 
                   r_m->normals[cur_face->f0].z);
      glVertex3f(r_m->vertices[cur_face->f0].x, 
                 r_m->vertices[cur_face->f0].y,
                 r_m->vertices[cur_face->f0].z);
      if (light_mode)
        glNormal3f(r_m->normals[cur_face->f1].x, 
                   r_m->normals[cur_face->f1].y, 
                   r_m->normals[cur_face->f1].z);
      glVertex3f(r_m->vertices[cur_face->f1].x, 
                 r_m->vertices[cur_face->f1].y,
                 r_m->vertices[cur_face->f1].z);
      if (light_mode)
        glNormal3f(r_m->normals[cur_face->f2].x, 
                   r_m->normals[cur_face->f2].y, 
                   r_m->normals[cur_face->f2].z);
      glVertex3f(r_m->vertices[cur_face->f2].x, 
                 r_m->vertices[cur_face->f2].y,
                 r_m->vertices[cur_face->f2].z);
    }
    break;
  case 1:
  case 2:
    for (i=0; i<r_m->num_faces; i++) {
      cur_face = &(r_m->faces[i]);
      setGlColor(cur_face->f0, cmap, gl_ctx);
      glVertex3f(r_m->vertices[cur_face->f0].x, 
                 r_m->vertices[cur_face->f0].y,
                 r_m->vertices[cur_face->f0].z);
      setGlColor(cur_face->f1, cmap, gl_ctx);
      glVertex3f(r_m->vertices[cur_face->f1].x, 
                 r_m->vertices[cur_face->f1].y,
                 r_m->vertices[cur_face->f1].z);
      setGlColor(cur_face->f2, cmap, gl_ctx);
      glVertex3f(r_m->vertices[cur_face->f2].x, 
                 r_m->vertices[cur_face->f2].y,
                 r_m->vertices[cur_face->f2].z);
    }
    break;
  default: /* should never get here */
    abort();
  }

  glEnd();
  glEndList();

  if (gl_ctx->disp_curv)
    free_colormap(cmap);

  if (gl_ctx->draw_normals) {
    scale_fact = NORMALS_DISPLAY_FACTOR*dist_v(&(r_m->bBox[0]), 
					       &(r_m->bBox[1]));

    glNewList(dl_idx->normal_list, GL_COMPILE);
    glColor3f(1.0, 0.0, 0.0);
    glBegin(GL_LINES);
    for (i=0; i<r_m->num_vert; i++) {
      glVertex3f(r_m->vertices[i].x, 
                 r_m->vertices[i].y, 
                 r_m->vertices[i].z);
      glVertex3f(r_m->vertices[i].x + 
		 scale_fact*r_m->normals[i].x,
		 r_m->vertices[i].y + 
		 scale_fact*r_m->normals[i].y,
		 r_m->vertices[i].z + 
		 scale_fact*r_m->normals[i].z);
    }
    glEnd();
    glColor3f(1.0, 1.0, 1.0);
    glEndList();
  }
  
}

static void display_vtx_labels(struct gl_render_context *gl_ctx, 
                               struct display_lists_indices *dl_idx) {
  int i, len;
  char str[42],fmt[24];

  glPushAttrib(GL_ALL_ATTRIB_BITS);
  glDisable(GL_LIGHTING);
  glColor3f(0.0, 1.0, 0.0);

  glListBase(dl_idx->char_list);
  strcpy(fmt, "%i");
  for (i=0; i<gl_ctx->raw_model->num_vert; i++) {
    glRasterPos3f(gl_ctx->raw_model->vertices[i].x,
		  gl_ctx->raw_model->vertices[i].y,
		  gl_ctx->raw_model->vertices[i].z);
    /* Add an offset to avoid drawing the label on the vertex*/
    glBitmap(0, 0, 0, 0, 7, 7, NULL); 
    len = sprintf(str, fmt, i);
    glCallLists(len, GL_UNSIGNED_BYTE, str);
  }
  
  glPopAttrib();
}

/* ***************************************************************** */
/* Display function : clear buffers, build correct MODELVIEW matrix, */
/* call display list and swap the buffers                            */
/* ***************************************************************** */
void display_wrapper(struct gl_render_context *gl_ctx, 
                     struct display_lists_indices *dl_idx) {
  GLenum errorCode;
  GLboolean light_mode;
  GLfloat lpos[] = {-1.0, 1.0, 1.0, 0.0};
  int i;
  
  light_mode = glIsEnabled(GL_LIGHTING);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glLoadIdentity();
  glLightfv(GL_LIGHT0, GL_POSITION, lpos);
  glTranslated(0.0, 0.0, -gl_ctx->distance); /* Translate the object along z */
  glMultMatrixd(gl_ctx->mvmatrix); /* Perform rotation */
  if (!light_mode) {
    for (i=0; i<=gl_ctx->wf_bc; i++) {
      switch (i) {
      case 0:
        if (gl_ctx->disp_curv > 0)
          glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        else if (gl_ctx->wf_bc)
          glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        
/*         else  */
/*           glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); */

	if (gl_ctx->ps_rend%2 == 0) /* rendering to buffer or to a PS
                                       file in negative */
	  glColor3f(1.0, 1.0, 1.0);
	else 
	  glColor3f(0.0, 0.0, 0.0);
        
	break;
      case 1:
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	if (gl_ctx->ps_rend%2 == 0) {/* rendering to buffer or to a PS
                                        file in negative */
	  glEnable(GL_POLYGON_OFFSET_FILL);
	  glPolygonOffset(1.0, 1.0);
	  glColor4f(0.0, 0.0, 0.0, 0.0);
	}
	else {
	  gl2psEnable(GL2PS_POLYGON_OFFSET_FILL);
	  glPolygonOffset(1.0, 1.0);
	  glColor4f(1.0, 1.0, 1.0, 0.0);
	}
	break;
      }

      glCallList(dl_idx->model_list);
  
      glDisable(GL_POLYGON_OFFSET_FILL);
      if (gl_ctx->ps_rend)
	gl2psDisable(GL2PS_POLYGON_OFFSET_FILL);
    }
  } else
    glCallList(dl_idx->model_list);
  
  if (gl_ctx->draw_normals)
    glCallList(dl_idx->normal_list);


  if (gl_ctx->draw_vtx_labels)
    display_vtx_labels(gl_ctx, dl_idx);


  if(gl_ctx->draw_spanning_tree)
    glCallList(dl_idx->tree_list);
  
  
 /* Check for errors (leave at the end) */
  while ((errorCode = glGetError()) != GL_NO_ERROR) {
    fprintf(stderr,"GL error: %s\n",(const char *)gluErrorString(errorCode));
  }
  glutSwapBuffers();

}
