/* $Id: rawview_disp.c,v 1.1 2002/06/04 13:06:40 aspert Exp $ */

#include <rawview_misc.h>

/* ************************************************************* */
/* This functions rebuilds the display list of the current model */
/* It is called when the viewing setting (light...) are changed  */
/* ************************************************************* */
void rebuild_list(struct gl_render_context *gl_ctx,
                  struct display_lists_indices *dl_idx) {
  int i;
  GLboolean light_mode;
  float scale_fact;
  face_t *cur_face;
  vertex_t center1, center2;
  face_t *cur_face2;
  int j, face1=-1, face2=-1;

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
      printf("Unable to create dl_idx->char_list\n");
    
    for (i=32; i<127; i++) { /* build display lists for labels */
      glNewList(dl_idx->char_list+i, GL_COMPILE);
      glutBitmapCharacter(GLUT_BITMAP_8_BY_13, i);
      glEndList();
    }
  }


  /* Get the state of the lighting */
  light_mode = glIsEnabled(GL_LIGHTING);
  if (light_mode && !glIsEnabled(GL_NORMAL_ARRAY)) {
    glEnableClientState(GL_NORMAL_ARRAY);
    glNormalPointer(GL_FLOAT, 0, (float*)(gl_ctx->raw_model->normals));
  }
  if (!glIsEnabled(GL_VERTEX_ARRAY)) {
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, (float*)(gl_ctx->raw_model->vertices));
  }
  

#ifdef DEBUG
  printf("dn = %d lm = %d nf = %d\n", gl_ctx->draw_normals, light_mode, 
	 gl_ctx->raw_model->num_faces); 
#endif
  

  if (gl_ctx->draw_spanning_tree == 1) {

    glNewList(dl_idx->tree_list, GL_COMPILE);
    glColor3f(1.0, 1.0, 0.0);
    glBegin(GL_LINES);
    for (j=0; j<gl_ctx->raw_model->num_faces; j++) {
      face1 = (gl_ctx->raw_model->tree)[j]->face_idx;
      cur_face = &(gl_ctx->raw_model->faces[face1]);
      add3_sc_v(0.333, &(gl_ctx->raw_model->vertices[cur_face->f0]), 
		&(gl_ctx->raw_model->vertices[cur_face->f1]), 
		&(gl_ctx->raw_model->vertices[cur_face->f2]), &center1);
      if ((gl_ctx->raw_model->tree)[j]->left != NULL) {
	face2 = ((gl_ctx->raw_model->tree)[j]->left)->face_idx;
	cur_face2 = &(gl_ctx->raw_model->faces[face2]);
	add3_sc_v(0.333, &(gl_ctx->raw_model->vertices[cur_face2->f0]), 
		  &(gl_ctx->raw_model->vertices[cur_face2->f1]), 
		  &(gl_ctx->raw_model->vertices[cur_face2->f2]), &center2);
	glVertex3fv((float*)(&center1));
	glVertex3fv((float*)(&center2));

      }
      if ((gl_ctx->raw_model->tree)[j]->right != NULL) {
	face2 = ((gl_ctx->raw_model->tree)[j]->right)->face_idx;
	cur_face2 = &(gl_ctx->raw_model->faces[face2]);
	add3_sc_v(0.333, &(gl_ctx->raw_model->vertices[cur_face2->f0]), 
		  &(gl_ctx->raw_model->vertices[cur_face2->f1]), 
		  &(gl_ctx->raw_model->vertices[cur_face2->f2]), &center2);
	glVertex3fv((float*)(&center1));
	glVertex3fv((float*)(&center2));

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
  for (i=0; i<gl_ctx->raw_model->num_faces; i++) {
    cur_face = &(gl_ctx->raw_model->faces[i]);
    glArrayElement(cur_face->f0);
    glArrayElement(cur_face->f1);
    glArrayElement(cur_face->f2);
  }
  glEnd();
  glEndList();



  if (gl_ctx->draw_normals) {
    scale_fact = NORMALS_DISPLAY_FACTOR*dist_v(&(gl_ctx->raw_model->bBox[0]), 
					       &(gl_ctx->raw_model->bBox[1]));

    glNewList(dl_idx->normal_list, GL_COMPILE);
    glColor3f(1.0, 0.0, 0.0);
    glBegin(GL_LINES);
    for (i=0; i<gl_ctx->raw_model->num_vert; i++) {
      glArrayElement(i);
      glVertex3f(gl_ctx->raw_model->vertices[i].x + 
		 scale_fact*gl_ctx->raw_model->normals[i].x,
		 gl_ctx->raw_model->vertices[i].y + 
		 scale_fact*gl_ctx->raw_model->normals[i].y,
		 gl_ctx->raw_model->vertices[i].z + 
		 scale_fact*gl_ctx->raw_model->normals[i].z);
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
  if (!light_mode)
    for (i=0; i<=gl_ctx->wf_bc; i++) {
      switch (i) {
      case 0:
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	if (!gl_ctx->ps_rend)
	  glColor3f(1.0, 1.0, 1.0);
	else
	  glColor3f(0.0, 0.0, 0.0);
	break;
      case 1:
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	if (!gl_ctx->ps_rend) {
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
  else
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
