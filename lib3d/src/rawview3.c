/* $Id: rawview3.c,v 1.12 2001/06/12 08:33:45 aspert Exp $ */
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <3dutils.h>
#include <image.h>

/* ****************** */
/* Useful Global vars */
/* ****************** */
GLfloat FOV = 40.0; /* vertical field of view */
GLdouble distance, dstep; /* distance and incremental distance step */
GLdouble mvmatrix[16]; /* Buffer for GL_MODELVIEW_MATRIX */
GLuint model_list = 0; /* display lists idx storage */
GLuint normal_list = 0;
GLuint char_list = 0;
GLuint tree_list = 0;


int oldx, oldy;
int left_button_state;
int middle_button_state;
int right_button_state;
int tr_mode = 1; /* Default = draw triangles */
int light_mode = 0;
int draw_normals = 0;
int draw_vtx_labels = 0;
int draw_spanning_tree = 0;

vertex center;
int normals_done = 0;
model *raw_model;
char *in_filename;
int grab_number = 0;
int mesa_minor = -1; /* Used 'cause Mesa > 3.1 is not trusted when rendering */
/* in the 'lighted' mode */

model *r_model;


/* *********************************************************** */
/* This is a _basic_ frame grabber -> copy all the RGB buffers */
/* and put'em into grab$$$.ppm where $$$ is [000; 999]         */
/* This may kill the X server under undefined conditions       */
/* This function is called by pressing the 'F6' key            */
/* *********************************************************** */
void frame_grab() {
  int w,h,i;
  unsigned char *r_buffer, *g_buffer, *b_buffer;
  image_uchar *frame;
  char filename[11];
  int nbytes;
  FILE *pf;

  w = glutGet(GLUT_WINDOW_WIDTH);
  h = glutGet(GLUT_WINDOW_HEIGHT);
  r_buffer = (unsigned char*)malloc(w*h*sizeof(unsigned char));
  g_buffer = (unsigned char*)malloc(w*h*sizeof(unsigned char));
  b_buffer = (unsigned char*)malloc(w*h*sizeof(unsigned char));
  glReadBuffer(GL_FRONT);
  glReadPixels(0, 0, w, h, GL_RED, GL_UNSIGNED_BYTE, r_buffer);
  glReadPixels(0, 0, w, h, GL_GREEN, GL_UNSIGNED_BYTE, g_buffer);
  glReadPixels(0, 0, w, h, GL_BLUE, GL_UNSIGNED_BYTE, b_buffer);
  frame = image_uchar_alloc(w, h, 3, 255);
  nbytes = w*sizeof(unsigned char);
  for (i=0; i<h; i++) {
    memcpy(frame->data[0][i], &(r_buffer[w*(h-i)]), nbytes);
    memcpy(frame->data[1][i], &(g_buffer[w*(h-i)]), nbytes);
    memcpy(frame->data[2][i], &(b_buffer[w*(h-i)]), nbytes);
  }
  sprintf(filename,"grab%03d.ppm",grab_number);
  grab_number++;
  pf = fopen(filename,"w");
  image_uchar_write(frame, pf);
  fclose(pf);
  free(r_buffer);
  free(g_buffer);
  free(b_buffer);
  free_image_uchar(frame);
}


/* ************************************************************ */
/* Here is the callback function when mouse buttons are pressed */
/* or released. It does nothing else than store their state     */
/* ************************************************************ */
void mouse_button(int button, int state, int x, int y) {
switch(button) {
  case GLUT_LEFT_BUTTON:
    if (state==GLUT_DOWN) {
      oldx = x;
      oldy = y;
      left_button_state = 1;
    } else if (state == GLUT_UP)
      left_button_state = 0;
    break;
  case GLUT_MIDDLE_BUTTON:
    if (state==GLUT_DOWN) {
      oldx = x;
      oldy = y;
      middle_button_state = 1;
    } else if (state == GLUT_UP)
      middle_button_state = 0;
    break;
  case GLUT_RIGHT_BUTTON:
    if (state==GLUT_DOWN) {
      oldx = x;
      oldy = y;
      right_button_state = 1;
    } else if (state == GLUT_UP)
      right_button_state = 0;
    break;
  }
}


/* ********************************************************* */
/* Callback function when the mouse is dragged in the window */
/* Only does sthg when a button is pressed                   */
/* ********************************************************* */
void motion_mouse(int x, int y) {
  int dx, dy;
  GLdouble dth, dph, dpsi;

  dx = x - oldx;
  dy = y - oldy;

  if (left_button_state == 1) {
    dth = dx*0.5; /* Yes, 0.5 is arbitrary */
    dph = dy*0.5;
    glPushMatrix(); /* Save transform context */
    glLoadIdentity();
    glRotated(dth, 0.0, 1.0, 0.0); /* Compute new rotation matrix */
    glRotated(dph, 1.0, 0.0, 0.0);
    glMultMatrixd(mvmatrix); /* Add the sum of the previous ones */
    glGetDoublev(GL_MODELVIEW_MATRIX, mvmatrix); /* Get the final matrix */
    glPopMatrix(); /* Reload previous transform context */
    glutPostRedisplay();
  }
  else if (middle_button_state == 1) {
    distance += dy*dstep;
    glutPostRedisplay();
  }
  else if (right_button_state == 1) { 
    dpsi = -dx*0.5;
    glPushMatrix(); /* Save transform context */
    glLoadIdentity();
    glRotated(dpsi, 0.0, 0.0, 1.0); /* Modify roll angle */
    glMultMatrixd(mvmatrix);
    glGetDoublev(GL_MODELVIEW_MATRIX, mvmatrix); /* Get the final matrix */
    glPopMatrix(); /* Reload previous transform context */
    glutPostRedisplay();
  }
  oldx = x;
  oldy = y;
}

/* ********************************************************** */
/* Reshape callbak function. Only sets correct values for the */
/* viewport and projection matrices.                          */
/* ********************************************************** */
void reshape(int width, int height) {
  glViewport(0, 0, width, height);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(FOV, (GLdouble)width/(GLdouble)height, distance/10.0, 
		 10.0*distance);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  
}

/* ************************************************************* */
/* This functions rebuilds the display list of the current model */
/* It is called when the viewing setting (light...) are changed  */
/* ************************************************************* */
void rebuild_list(model *raw_model) {
  int i;
  face *cur_face;

#ifdef FACE_NORM_DRAW_DEBUG
  vertex center; 
#endif

  vertex center1, center2;
  face *cur_face2;
  int j, face1=-1, face2=-1;



  if (glIsList(model_list) == GL_TRUE)
    glDeleteLists(model_list, 1);

  if (glIsList(normal_list) == GL_TRUE)
    glDeleteLists(normal_list, 1);
  model_list = glGenLists(1);


  if (glIsList(char_list) == GL_TRUE)
    glDeleteLists(char_list, 256);



  if (glIsList(tree_list) == GL_TRUE)
    glDeleteLists(tree_list, 1);
  tree_list = glGenLists(1);


  if (draw_normals)
    normal_list = glGenLists(1);


  if (draw_vtx_labels) {
    char_list = glGenLists(256);
    if (char_list == 0)
      printf("Unable to create char_list\n");

    for (i=32; i<127; i++) { /* build display lists for labels */
      glNewList(char_list+i, GL_COMPILE);
      glutBitmapCharacter(GLUT_BITMAP_8_BY_13, i);
      glEndList();
    }
  }

  
#ifdef DEBUG
  printf("dn = %d lm = %d nf = %d\n", draw_normals, light_mode, 
	 raw_model->num_faces); 
#endif
  

/*   printf("[rebuild_list]: raw_model->tree = 0x%x\n",  */
/* 	 (unsigned int)raw_model->tree); */
  if (draw_spanning_tree == 1) {
    glNewList(tree_list, GL_COMPILE);
    glColor3f(1.0, 1.0, 0.0);
    glBegin(GL_LINES);
    for (j=0; j<raw_model->num_faces; j++) {
      face1 = (raw_model->tree)[j]->face_idx;
      cur_face = &(raw_model->faces[face1]);
      center1.x = (raw_model->vertices[cur_face->f0].x +
		   raw_model->vertices[cur_face->f1].x +
		   raw_model->vertices[cur_face->f2].x)/3.0;
      center1.y = (raw_model->vertices[cur_face->f0].y +
		   raw_model->vertices[cur_face->f1].y +
		   raw_model->vertices[cur_face->f2].y)/3.0;
      center1.z = (raw_model->vertices[cur_face->f0].z +
		   raw_model->vertices[cur_face->f1].z +
		   raw_model->vertices[cur_face->f2].z)/3.0;
      if ((raw_model->tree)[j]->left != NULL) {
	face2 = ((raw_model->tree)[j]->left)->face_idx;
	cur_face2 = &(raw_model->faces[face2]);
	center2.x = (raw_model->vertices[cur_face2->f0].x +
		     raw_model->vertices[cur_face2->f1].x +
		     raw_model->vertices[cur_face2->f2].x)/3.0;
	center2.y = (raw_model->vertices[cur_face2->f0].y +
		     raw_model->vertices[cur_face2->f1].y +
		     raw_model->vertices[cur_face2->f2].y)/3.0;
	center2.z = (raw_model->vertices[cur_face2->f0].z +
		     raw_model->vertices[cur_face2->f1].z +
		     raw_model->vertices[cur_face2->f2].z)/3.0;
	glVertex3d(center1.x, center1.y, center1.z);
	glVertex3d(center2.x, center2.y, center2.z);

      }
      if ((raw_model->tree)[j]->right != NULL) {
	face2 = ((raw_model->tree)[j]->right)->face_idx;
	cur_face2 = &(raw_model->faces[face2]);
	center2.x = (raw_model->vertices[cur_face2->f0].x +
		     raw_model->vertices[cur_face2->f1].x +
		     raw_model->vertices[cur_face2->f2].x)/3.0;
	center2.y = (raw_model->vertices[cur_face2->f0].y +
		     raw_model->vertices[cur_face2->f1].y +
		     raw_model->vertices[cur_face2->f2].y)/3.0;
	center2.z = (raw_model->vertices[cur_face2->f0].z +
		     raw_model->vertices[cur_face2->f1].z +
		     raw_model->vertices[cur_face2->f2].z)/3.0;
	glVertex3d(center1.x, center1.y, center1.z);
	glVertex3d(center2.x, center2.y, center2.z);

      }
    }
    glEnd();
    glColor3f(1.0, 1.0, 1.0);
    glEndList();

  }

  if (light_mode == 0) { /* Store a wireframe model */
    glNewList(model_list, GL_COMPILE);
    if (tr_mode == 1)
      glBegin(GL_TRIANGLES);
    else 
      glBegin(GL_POINTS);
    for (i=0; i<raw_model->num_faces; i++) {
      cur_face = &(raw_model->faces[i]);
      glVertex3d(raw_model->vertices[cur_face->f0].x,
		 raw_model->vertices[cur_face->f0].y,
		 raw_model->vertices[cur_face->f0].z); 

      glVertex3d(raw_model->vertices[cur_face->f1].x,
		 raw_model->vertices[cur_face->f1].y,
		 raw_model->vertices[cur_face->f1].z); 

      glVertex3d(raw_model->vertices[cur_face->f2].x,
		 raw_model->vertices[cur_face->f2].y,
		 raw_model->vertices[cur_face->f2].z); 
    }
    glEnd();
    glEndList();
    if (draw_normals) {
      glNewList(normal_list, GL_COMPILE);
      glColor3f(1.0, 0.0, 0.0);
      glBegin(GL_LINES);

#ifdef FACE_NORM_DRAW_DEBUG	
      for (i=0; i<raw_model->num_faces; i++) {
	cur_face = &(raw_model->faces[i]);
	center.x = (raw_model->vertices[cur_face->f0].x +
		    raw_model->vertices[cur_face->f1].x +
		    raw_model->vertices[cur_face->f2].x)/3.0;

	center.y = (raw_model->vertices[cur_face->f0].y +
		    raw_model->vertices[cur_face->f1].y +
		    raw_model->vertices[cur_face->f2].y)/3.0;

	center.z = (raw_model->vertices[cur_face->f0].z +
		    raw_model->vertices[cur_face->f1].z +
		    raw_model->vertices[cur_face->f2].z)/3.0;
	glVertex3d(center.x, center.y, center.z);
	glVertex3d(center.x + 0.1*raw_model->face_normals[i].x, 
		   center.y + 0.1*raw_model->face_normals[i].y, 
		   center.z + 0.1*raw_model->face_normals[i].z);
      }
#else
      for (i=0; i<raw_model->num_vert; i++) {
	glVertex3d(raw_model->vertices[i].x, 
		   raw_model->vertices[i].y,
		   raw_model->vertices[i].z);

	glVertex3d(raw_model->vertices[i].x + 0.1*raw_model->normals[i].x,
		   raw_model->vertices[i].y + 0.1*raw_model->normals[i].y,
		   raw_model->vertices[i].z + 0.1*raw_model->normals[i].z);
      }
#endif

      glEnd();
      glColor3f(1.0, 1.0, 1.0);
      glEndList();
    }
  } else {
    glNewList(model_list, GL_COMPILE);
    if (tr_mode == 1)
      glBegin(GL_TRIANGLES);
    else
      glBegin(GL_POINTS);
    for (i=0; i<raw_model->num_faces; i++) {
      cur_face = &(raw_model->faces[i]);
      glNormal3d(raw_model->normals[cur_face->f0].x,
		 raw_model->normals[cur_face->f0].y,
		 raw_model->normals[cur_face->f0].z);
      glVertex3d(raw_model->vertices[cur_face->f0].x,
		 raw_model->vertices[cur_face->f0].y,
		 raw_model->vertices[cur_face->f0].z);
      glNormal3d(raw_model->normals[cur_face->f1].x,
		 raw_model->normals[cur_face->f1].y,
		 raw_model->normals[cur_face->f1].z); 
      glVertex3d(raw_model->vertices[cur_face->f1].x,
		 raw_model->vertices[cur_face->f1].y,
		 raw_model->vertices[cur_face->f1].z);
      glNormal3d(raw_model->normals[cur_face->f2].x,
		 raw_model->normals[cur_face->f2].y,
		 raw_model->normals[cur_face->f2].z); 
      glVertex3d(raw_model->vertices[cur_face->f2].x,
		 raw_model->vertices[cur_face->f2].y,
		 raw_model->vertices[cur_face->f2].z); 
    }
    glEnd();
    glEndList();
    if (draw_normals) {
      glNewList(normal_list, GL_COMPILE);
      glColor3f(1.0, 0.0, 0.0);
      glBegin(GL_LINES);
      for (i=0; i<raw_model->num_vert; i++) {
	glVertex3d(raw_model->vertices[i].x, 
		   raw_model->vertices[i].y,
		   raw_model->vertices[i].z);
	glVertex3d(raw_model->vertices[i].x + raw_model->normals[i].x,
		   raw_model->vertices[i].y + raw_model->normals[i].y,
		   raw_model->vertices[i].z + raw_model->normals[i].z);
      }
      glEnd();
      glColor3f(1.0, 1.0, 1.0);
      glEndList();
    }
  }
}

/* ******************************************** */
/* Initial settings of the rendering parameters */
/* ******************************************** */
void gfx_init(model *raw_model) {
  const char *glverstr;
  char *mesa, *irix, *hp;
  int mesa_major;
  
  glverstr = (const char*)glGetString(GL_VERSION);
  printf("GL_VERSION = %s\n", glverstr);
  /* Now we try to identify what kind of OpenGL implementation we have */
  

  mesa = strstr(glverstr, "Mesa");
  irix = strstr(glverstr, "Irix");
  hp = strstr(glverstr, "Revision");

  
  if (mesa != NULL) {
    mesa += 5;
    if (sscanf(mesa, "%i.%i", &mesa_major, &mesa_minor) != 2) {
      printf("Error checking Mesa version\n");
      free(raw_model->vertices);
      free(raw_model->faces);
      if (raw_model->normals != NULL)
	free(raw_model->normals);
      if (raw_model->face_normals != NULL)
	free(raw_model->face_normals);
      if (raw_model->area != NULL)
	free(raw_model->area);
      free(raw_model);
      exit(1);
    }
    if (mesa_major != 3) {
      printf("Incorrect (too old ?) Mesa version found ?\n");
    }
  } else if (irix != NULL) /* SGI OpenGL found */
    mesa_minor = 0; /* should be OK with this */
  else if (hp != NULL) /* HP (Vis') OpenGL */
    mesa_minor = 4; /* same behaviour as recent Mesa version */
  else { /* Unknown OpenGL */
    printf("Error checking OpenGL version\n");
    free(raw_model->vertices);
    free(raw_model->faces);
    if (raw_model->normals != NULL)
      free(raw_model->normals);
    if (raw_model->face_normals != NULL)
      free(raw_model->face_normals);
    if (raw_model->area != NULL)
      free(raw_model->area);
    free(raw_model);
    exit(1);
  }


  glEnable(GL_DEPTH_TEST);
  glShadeModel(GL_SMOOTH); 

  glClearColor(0.0, 0.0, 0.0, 0.0);
  glColor3f(1.0, 1.0, 1.0); /* Settings for wireframe model */
  glFrontFace(GL_CCW);
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);


  rebuild_list(raw_model);
    
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(FOV, 1.0, distance/10.0, 10.0*distance);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glGetDoublev(GL_MODELVIEW_MATRIX, mvmatrix); /* Initialize the temp matrix */
}

/* **************************** */
/* Callback for the normal keys */
/* **************************** */
void norm_key_pressed(unsigned char key, int x, int y) {
  switch(key) {
  case 'q':
  case 'Q':
    free(raw_model->vertices);
    free(raw_model->faces);
    if (raw_model->normals != NULL)
      free(raw_model->normals);
    if (raw_model->face_normals != NULL)
      free(raw_model->face_normals);
    if (raw_model->area != NULL)
      free(raw_model->area);
    free(raw_model);
    exit(0);
    break;
  }
}

/* ********************************************************* */
/* Callback function for the special keys (arrows, function) */
/* ********************************************************* */
void sp_key_pressed(int key, int x, int y) {
  GLdouble tmp[16];
  GLfloat amb[] = {0.5, 0.5, 0.5, 1.0};
  GLfloat dif[] = {0.5, 0.5, 0.5, 1.0};
  GLfloat spec[] = {0.5, 0.5, 0.5, 0.5};
  GLfloat ldir[] = {0.0, 0.0, 0.0, 0.0};
  GLfloat mat_spec[] = {0.3, 0.7, 0.5, 0.5};
  GLfloat amb_light[] = {0.6, 0.6, 0.6, 1.0};
  GLfloat shine[] = {0.6};
  GLfloat lpos[4];
  info_vertex *curv;
  face_tree_ptr top;
  int i;



  
  /* This one must be handled 'by hand' to please the MIPS compiler on SGI */
  lpos[0] = 0.0;
  lpos[1] = 0.0;
  if (mesa_minor <= 1) 
    /* This causes trouble w. Mesa > 3.1 when distance is too large */
    lpos[2] = -distance;  
  else
    lpos[2] = 1.0; 
  lpos[3] = 0.0;
  

  switch(key) {
  case GLUT_KEY_F1:/* Print MODELVIEW matrix */
    glGetDoublev(GL_MODELVIEW_MATRIX, tmp);
    printf("\n");
    for (i=0; i<4; i++)
      printf("%f\t%f\t%f\t%f\n", tmp[4*i], tmp[4*i+1], tmp[4*i+2], 
	     tmp[4*i+3]); 
    break;
  case GLUT_KEY_F2: /* Toggle Light+filled mode */
    if (light_mode == 0) {
      light_mode = 1;
      printf("Lighted mode\n");
      if (normals_done != 1) {/* We have to build the normals */
	printf("Computing normals...");
	fflush(stdout);
	raw_model->area = (double*)malloc(raw_model->num_faces*sizeof(double));
	curv = (info_vertex*)malloc(raw_model->num_vert*sizeof(info_vertex));

	raw_model->face_normals = compute_face_normals(raw_model, curv);
	printf("raw_model->tree = 0x%x\n", (unsigned int)raw_model->tree);
#ifdef FACE_NORM_DRAW_DEBUG	
	for (i=0; i<raw_model->num_faces; i++)
	  printf("%d: %f %f %f\n",i, model_normals[i].x, model_normals[i].y, 
		 model_normals[i].z);
#endif

	if (raw_model->face_normals != NULL){
	  compute_vertex_normal(raw_model, curv, raw_model->face_normals);
	  for (i=0; i<raw_model->num_vert; i++) 
	    free(curv[i].list_face);
	  free(curv);
	  normals_done = 1;
	  printf("done\n");
	
	  glEnable(GL_LIGHTING);
 	  glLightfv(GL_LIGHT0, GL_AMBIENT, amb); 
	  glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
	  glLightfv(GL_LIGHT0, GL_SPECULAR, spec);
  	  glLightfv(GL_LIGHT0, GL_POSITION, lpos);  
  	  glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, ldir);  
	  glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
	  glLightModelfv(GL_LIGHT_MODEL_AMBIENT,amb_light);
	  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_spec);
	  glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shine);
	  glEnable(GL_LIGHT0);
	  glColor3f(1.0, 1.0, 1.0);
	  glFrontFace(GL_CCW);
	  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	  printf("Rebuild display list\n"); 
	  rebuild_list(raw_model);
	  glutPostRedisplay();
	} else {
	  printf("Unable to compute normals... non-manifold model\n");
	  light_mode = 0;
	}
      } else {
	glEnable(GL_LIGHTING);
	glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
	glLightfv(GL_LIGHT0, GL_SPECULAR, spec);
	glLightfv(GL_LIGHT0, GL_POSITION, lpos);
	glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, ldir);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_spec);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shine);
	glEnable(GL_LIGHT0);
	glColor3f(1.0, 1.0, 1.0);
	glFrontFace(GL_CCW);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	printf("Rebuild display list\n"); 
	rebuild_list(raw_model);
	glutPostRedisplay();
      }
      break;
    } else if (light_mode == 1) {
      light_mode = 0;
      printf("Wireframe mode\n");
      glDisable(GL_LIGHTING);
      glColor3f(1.0, 1.0, 1.0);
      glFrontFace(GL_CCW);
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      rebuild_list(raw_model);
      glutPostRedisplay();
      break;
    }
    printf("Something sucks...\n");
    break;
  case GLUT_KEY_F3: /* invert normals */
    if (light_mode || draw_normals) {
      printf("Inverting normals\n");
      for (i=0; i<raw_model->num_vert; i++) {
	raw_model->normals[i].x = -raw_model->normals[i].x;
	raw_model->normals[i].y = -raw_model->normals[i].y;
	raw_model->normals[i].z = -raw_model->normals[i].z;
      }

#ifdef FACE_NORM_DRAW_DEBUG
      for (i=0; i<raw_model->num_faces; i++) {
	raw_model->face_normals[i].x = -raw_model->face_normals[i].x;
	raw_model->face_normals[i].y = -raw_model->face_normals[i].y;
	raw_model->face_normals[i].z = -raw_model->face_normals[i].z;
      }
#endif

      rebuild_list(raw_model);
      glutPostRedisplay();
    }
    break;
  case GLUT_KEY_F4: /* draw normals */
    if (draw_normals == 0) {
      draw_normals = 1;
      printf("Draw normals\n");
      if (normals_done != 1) {/* We have to build the normals */
	printf("Computing normals...");
	fflush(stdout);
	raw_model->area = (double*)malloc(raw_model->num_faces*sizeof(double));
	curv = (info_vertex*)malloc(raw_model->num_vert*sizeof(info_vertex));
	raw_model->face_normals = compute_face_normals(raw_model, curv);
	if (raw_model->face_normals != NULL) {
	  compute_vertex_normal(raw_model, curv, raw_model->face_normals);
	  for (i=0; i<raw_model->num_vert; i++) 
	    free(curv[i].list_face);
	  free(curv);
	  normals_done = 1;
	  printf("done\n");	  
	} else {
	  printf("Unable to build normals (non-manifold model ?)\n");
	  draw_normals = 0;
	  break;
	}
      }
      printf("Rebuild display list\n"); 
      rebuild_list(raw_model);
      glutPostRedisplay();
      break;
    }

    else if (draw_normals == 1) {
      draw_normals = 0;
      printf("Rebuild display list\n"); 
      rebuild_list(raw_model);
      glutPostRedisplay();
      break;
    }
  case GLUT_KEY_F5: /* Save model... useful for normals */
    printf("Write model...\n");
    write_raw_model(raw_model, in_filename);
    break;
  case GLUT_KEY_F6: /* Frame grab */
    frame_grab();
    break;
  case GLUT_KEY_F7: /* switch from triangle mode to point mode */
    if(tr_mode == 1) {/*go to point mode*/
      tr_mode = 0;
      printf("Going to point mode\n");

    } else if(tr_mode == 0) {
      tr_mode = 1;
      printf("Going to triangle mode\n");
    }
    rebuild_list(raw_model);
    glutPostRedisplay();
    break;
  case GLUT_KEY_F8: /* draw labels for vertices */
    if (draw_vtx_labels == 1) {
      draw_vtx_labels = 0;
      printf("Stop drawing labels\n");
    } else if (draw_vtx_labels == 0) {
      draw_vtx_labels = 1;
      printf("Drawing labels\n");
    }
    rebuild_list(raw_model);
    glutPostRedisplay();
    break;

  case GLUT_KEY_F9: /* Draw the spanning tree */
    if(draw_spanning_tree == 1) {
      draw_spanning_tree = 0;
      printf("Stop drawing spanning tree\n");
    } else if (draw_spanning_tree == 0) {
      draw_spanning_tree = 1;
      printf("Drawing spanning tree\n");
      if (raw_model->tree == NULL) { /* We need to build this ...*/
	curv = (info_vertex*)malloc(raw_model->num_vert*sizeof(info_vertex));
	for(i=0; i<raw_model->num_vert; i++) {
	  curv[i].list_face = (int*)malloc(sizeof(int));
	  curv[i].num_faces = 0;
	}
	printf("Building spanning tree\n");
	/* Compute spanning tree of the dual graph */
	raw_model->tree = bfs_build_spanning_tree(raw_model, curv); 
	if (raw_model->tree == NULL)
	  printf("Uh oh... unable to build spanning tree\n");
	top = raw_model->tree[0];
	while (top->parent != NULL)
	  top = top->parent;
	printf("Spanning tree done\n");
	for(i=0; i<raw_model->num_vert; i++) 
	  free(curv[i].list_face);
	free(curv);
      }
    }
    rebuild_list(raw_model);
    glutPostRedisplay();
    break;

  case GLUT_KEY_UP:
    glPushMatrix(); /* Save transform context */
    glLoadIdentity();
    glRotated(-1.0, 1.0, 0.0, 0.0);
    glMultMatrixd(mvmatrix); /* Add the sum of the previous ones */
    glGetDoublev(GL_MODELVIEW_MATRIX, mvmatrix); /* Get the final matrix */
    glPopMatrix(); /* Reload previous transform context */
    glutPostRedisplay();
    break;
  case GLUT_KEY_DOWN:
    glPushMatrix(); 
    glLoadIdentity();
    glRotated(1.0, 1.0, 0.0, 0.0);
    glMultMatrixd(mvmatrix); 
    glGetDoublev(GL_MODELVIEW_MATRIX, mvmatrix); 
    glPopMatrix(); 
    glutPostRedisplay();
    break;
  case GLUT_KEY_LEFT:
    glPushMatrix();
    glLoadIdentity();
    glRotated(-1.0, 0.0, 1.0, 0.0);
    glMultMatrixd(mvmatrix); 
    glGetDoublev(GL_MODELVIEW_MATRIX, mvmatrix); 
    glPopMatrix(); 
    glutPostRedisplay();
    break;
  case GLUT_KEY_RIGHT:
    glPushMatrix();
    glLoadIdentity();
    glRotated(1.0, 0.0, 1.0, 0.0);
    glMultMatrixd(mvmatrix); 
    glGetDoublev(GL_MODELVIEW_MATRIX, mvmatrix); 
    glPopMatrix(); 
    glutPostRedisplay();
    break;
  case GLUT_KEY_PAGE_DOWN:
    glPushMatrix();
    glLoadIdentity();
    glRotated(-1.0, 0.0, 0.0, 1.0);
    glMultMatrixd(mvmatrix); 
    glGetDoublev(GL_MODELVIEW_MATRIX, mvmatrix); 
    glPopMatrix(); 
    glutPostRedisplay();
    break;
  case GLUT_KEY_END:
    glPushMatrix();
    glLoadIdentity();
    glRotated(1.0, 0.0, 0.0, 1.0);
    glMultMatrixd(mvmatrix); 
    glGetDoublev(GL_MODELVIEW_MATRIX, mvmatrix); 
    glPopMatrix(); 
    glutPostRedisplay();
    break;
  }
}


void display_vtx_labels() {
  int i, len;
  char str[42],fmt[24];

  glPushAttrib(GL_ALL_ATTRIB_BITS);
  glDisable(GL_LIGHTING);
  glColor3f(0.0, 1.0, 0.0);

  glListBase(char_list);
  strcpy(fmt, "%i");
  for (i=0; i<r_model->num_vert; i++) {
    glRasterPos3f(r_model->vertices[i].x,
		  r_model->vertices[i].y,
		  r_model->vertices[i].z);
    glBitmap(0,0,0,0,7,7,NULL); /* Add an offset to avoid drawing the label on the vertex*/
    len = sprintf(str, fmt, i);
    glCallLists(len, GL_UNSIGNED_BYTE, str);
  }
  
  glPopAttrib();
}


/* ***************************************************************** */
/* Display function : clear buffers, build correct MODELVIEW matrix, */
/* call display list and swap the buffers                            */
/* ***************************************************************** */
void display() {
  GLenum errorCode;

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glLoadIdentity();
  glTranslated(0.0, 0.0, -distance); /* Translate the object along z */
  glMultMatrixd(mvmatrix); /* Perform rotation */
  glCallList(model_list);
  if (draw_normals)
    glCallList(normal_list);

  if (draw_vtx_labels)
    display_vtx_labels();


  if(draw_spanning_tree)
    glCallList(tree_list);

 /* Check for errors (leave at the end) */
  while ((errorCode = glGetError()) != GL_NO_ERROR) {
    fprintf(stderr,"GL error: %s\n",(const char *)gluErrorString(errorCode));
  }
  glutSwapBuffers();


}

/* ************************************************************ */
/* Main function : read model, compute initial bounding box/vp, */
/* perform callback registration and go into the glutMainLoop   */
/* ************************************************************ */
int main(int argc, char **argv) {

  int i;


  if (argc != 2) {
    printf("rawview file.raw\n");
    exit(0);
  }
  

  in_filename = argv[1]; 
  raw_model = read_raw_model(in_filename);

  if (raw_model->builtin_normals == 1) {
    normals_done = 1;
    printf("The model has builtin normals\n");
  }

  
#ifdef DEBUG
  printf("bbox_min = %f %f %f\n", raw_model->bBox[0].x, 
	 raw_model->bBox[0].y, raw_model->bBox[0].z);
  printf("bbox_max = %f %f %f\n", raw_model->bBox[1].x, 
	 raw_model->bBox[1].y, raw_model->bBox[1].z);
#endif

  center.x = 0.5*(raw_model->bBox[1].x + raw_model->bBox[0].x);
  center.y = 0.5*(raw_model->bBox[1].y + raw_model->bBox[0].y);
  center.z = 0.5*(raw_model->bBox[1].z + raw_model->bBox[0].z);


  for (i=0; i<raw_model->num_vert; i++) {
    raw_model->vertices[i].x -= center.x;
    raw_model->vertices[i].y -= center.y;
    raw_model->vertices[i].z -= center.z;
  }
  


  distance = dist(raw_model->bBox[0], raw_model->bBox[1])/
    tan(FOV*M_PI_2/180.0);
  
  dstep = distance*0.01;


  r_model = raw_model;


  /* Init the rendering window */
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  glutInitWindowSize(500, 500);
  glutCreateWindow("Raw Mesh Viewer v3.0");

  /* Callback registration */
  glutDisplayFunc(display);
  glutReshapeFunc(reshape);
  glutSpecialFunc(sp_key_pressed);
  glutKeyboardFunc(norm_key_pressed); 
  glutMouseFunc(mouse_button);
  glutMotionFunc(motion_mouse);

  /* 1st frame + build model */
  gfx_init(raw_model);

  /* Go for it */
  glutMainLoop();

  /* should never get here */
  return 0;
}
