/* $Id: RawWidget.cpp,v 1.9 2001/08/07 12:32:21 aspert Exp $ */
#include <RawWidget.h>

// 
// This is a derived class from QGLWidget used to render models
//

RawWidget::RawWidget(model *raw_model, QWidget *parent, const char *name) 
  : QGLWidget( parent, name) { 
  
  int i;
  vertex center;

  // Fixed size widget
  setMinimumSize(512, 512);
  setMaximumSize(512, 512);

  // 0 is not a valid display list index
  model_list = 0;
  
  // Build the colormap used to display the mean error onto the surface of
  // the model
  colormap = HSVtoRGB();

  // Get the structure containing the model
  rawModelStruct = raw_model;

  // Initialize the state
  move_state=0;


  // Compute the center of the bounding box of the model
  center.x = 0.5*(rawModelStruct->bBox[1].x + rawModelStruct->bBox[0].x);
  center.y = 0.5*(rawModelStruct->bBox[1].y + rawModelStruct->bBox[0].y);
  center.z = 0.5*(rawModelStruct->bBox[1].z + rawModelStruct->bBox[0].z);

  // Center the model around (0, 0, 0)
  for (i=0; i<rawModelStruct->num_vert; i++) {
    rawModelStruct->vertices[i].x -= center.x;
    rawModelStruct->vertices[i].y -= center.y;
    rawModelStruct->vertices[i].z -= center.z;
  }
  
  // This should be enough to see the whole model when starting
  distance = dist(rawModelStruct->bBox[0], rawModelStruct->bBox[1])/
    tan(FOV*M_PI_2/180.0);

  // This is the increment used when moving closer/farther from the object
  dstep = distance*0.01;

}

void RawWidget::aslot() {
  if(move_state==0){
    move_state = 1;
    emit(transfervalue(distance,mvmatrix));
  }
  else
    move_state = 0;
}


void RawWidget::transfer(double dist,double *mvmat) {

  distance = dist;
  // Copy the 4x4 transformation matrix
  memcpy(mvmatrix, mvmat, 16*sizeof(double)); 
  // update display
  glDraw();
}

void RawWidget::setLine() {
  GLint line_state;

  glGetIntegerv(GL_POLYGON_MODE,&line_state);
  if (line_state==GL_FILL) {
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    printf("FILL->LINE\n");
    glDraw();
  }
  else if (line_state==GL_LINE) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    printf("LINE->FILL\n");
    glDraw(); 
  } else 
    printf("Invalid state value found for GL_POLYGON_MODE: %d\n", line_state);
  
}

void RawWidget::setLight() {

  GLboolean light_state;
  GLfloat amb[] = {0.5, 0.5, 0.5, 1.0};
  GLfloat dif[] = {0.5, 0.5, 0.5, 1.0};
  GLfloat spec[] = {0.5, 0.5, 0.5, 0.5};
  GLfloat ldir[] = {0.0, 0.0, 0.0, 0.0};
  GLfloat amb_light[] = {0.6, 0.6, 0.6, 1.0};
  GLfloat lpos[] = {0.0, 0.0, -distance, 0.0} ;
  info_vertex *curv; /* used for normal computation */
  int i;

  // Get state from renderer
  light_state = glIsEnabled(GL_LIGHTING);

  if (light_state==GL_FALSE){ // We are now switching to lighted mode
    if (rawModelStruct->normals !=NULL){// Are these ones computed ?
      glEnable(GL_LIGHTING);
      glLightfv(GL_LIGHT0, GL_AMBIENT, amb); 
      glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
      glLightfv(GL_LIGHT0, GL_SPECULAR, spec);
      glLightfv(GL_LIGHT0, GL_POSITION, lpos);  
      glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, ldir);  
      glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
      glLightModelfv(GL_LIGHT_MODEL_AMBIENT,amb_light);
      glEnable(GL_LIGHT0);
      glFrontFace(GL_CCW);
    } else {// Attempt to compute normals
      if (rawModelStruct->area==NULL)
	rawModelStruct->area = 
	  (double*)malloc(rawModelStruct->num_vert*sizeof(double));
      curv = 
	(info_vertex*)malloc(rawModelStruct->num_vert*sizeof(info_vertex));
      if (rawModelStruct->face_normals==NULL)
	rawModelStruct->face_normals = compute_face_normals(rawModelStruct, 
							    curv);
      if (rawModelStruct->face_normals==NULL) {// No way...
	fprintf(stderr, "Unable to compute normals\n");
	free(rawModelStruct->area);
	glDisable(GL_LIGHTING); // Just to be sure
      }
      else { // Compute a normal for each vertex
	compute_vertex_normal(rawModelStruct, curv, 
			      rawModelStruct->face_normals);

	for (i=0; i<rawModelStruct->num_vert; i++) 
	  free(curv[i].list_face);
	free(curv);	
	free(rawModelStruct->face_normals);
	free(rawModelStruct->area);

	glEnable(GL_LIGHTING);
	glLightfv(GL_LIGHT0, GL_AMBIENT, amb); 
	glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
	glLightfv(GL_LIGHT0, GL_SPECULAR, spec);
	glLightfv(GL_LIGHT0, GL_POSITION, lpos);  
	glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, ldir);  
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT,amb_light);
	glEnable(GL_LIGHT0);
	glFrontFace(GL_CCW);
      }
    }
  }
  else if (light_state==GL_TRUE){// We are now switching to wireframe mode
    glDisable(GL_LIGHTING);
    glFrontFace(GL_CCW);
  }
  glDraw();
}


// display callback
void RawWidget::paintGL() {
  display(distance);
}

// resize callback
void RawWidget::resizeGL(int width ,int height) {
  glViewport(0, 0, width, height);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(FOV, (GLdouble)width/(GLdouble)height, distance/10.0, 
		 10.0*distance);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  
}

// Initializations for the renderer
void RawWidget::initializeGL() { 
  glDepthFunc(GL_LESS);
  glEnable(GL_DEPTH_TEST);
  glShadeModel(GL_SMOOTH);


  glClearColor(0.0, 0.0, 0.0, 0.0);


  rebuild_list();
  glEndList();
  
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(FOV, 1.0, distance/10.0, 10.0*distance);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glGetDoublev(GL_MODELVIEW_MATRIX, mvmatrix); /* Initialize the temp matrix */
}  


// 'display' function called by the paintGL call back
// clears the buffers, computes correct transformation matrix
// and calls the model's display list
void RawWidget::display(double distance) {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glLoadIdentity();
  glTranslated(0.0, 0.0, -distance); /* Translate the object along z axis */
  glMultMatrixd(mvmatrix); /* Perform rotation */
  glCallList(model_list);
}


// This function generates the model's display list, depending on the
// viewing parameters (light...)
void RawWidget::rebuild_list() {
  int i;
  face *cur_face;

  if (glIsList(model_list)==GL_TRUE) 
    glDeleteLists(model_list, 1);

  model_list=glGenLists(1);

  if(rawModelStruct->normals!=NULL){
    glNewList(model_list, GL_COMPILE);
    glBegin(GL_TRIANGLES);  
    for (i=0; i<rawModelStruct->num_faces; i++) {
      cur_face = &(rawModelStruct->faces[i]);
      glColor3f(1.0,1.0,1.0);

      glNormal3d(rawModelStruct->normals[cur_face->f0].x,
		 rawModelStruct->normals[cur_face->f0].y,
		 rawModelStruct->normals[cur_face->f0].z);
      glVertex3d(rawModelStruct->vertices[cur_face->f0].x,
		 rawModelStruct->vertices[cur_face->f0].y,
		 rawModelStruct->vertices[cur_face->f0].z); 
      
      glNormal3d(rawModelStruct->normals[cur_face->f1].x,
	       rawModelStruct->normals[cur_face->f1].y,
		 rawModelStruct->normals[cur_face->f1].z);  
      glVertex3d(rawModelStruct->vertices[cur_face->f1].x,
		 rawModelStruct->vertices[cur_face->f1].y,
		 rawModelStruct->vertices[cur_face->f1].z); 
      
      glNormal3d(rawModelStruct->normals[cur_face->f2].x,
		 rawModelStruct->normals[cur_face->f2].y,
		 rawModelStruct->normals[cur_face->f2].z); 
      glVertex3d(rawModelStruct->vertices[cur_face->f2].x,
	       rawModelStruct->vertices[cur_face->f2].y,
		 rawModelStruct->vertices[cur_face->f2].z);       
    }
    glEnd();
    glEndList();
  } else {
    glNewList(model_list, GL_COMPILE);
    glBegin(GL_TRIANGLES);  
    for (i=0; i<rawModelStruct->num_faces; i++) {
      cur_face = &(rawModelStruct->faces[i]);
      glColor3f(colormap[rawModelStruct->error[cur_face->f0]][0],
		colormap[rawModelStruct->error[cur_face->f0]][1],
		colormap[rawModelStruct->error[cur_face->f0]][2]);  
      glVertex3d(rawModelStruct->vertices[cur_face->f0].x,
		 rawModelStruct->vertices[cur_face->f0].y,
		 rawModelStruct->vertices[cur_face->f0].z); 
      
      glColor3f(colormap[rawModelStruct->error[cur_face->f1]][0],
		colormap[rawModelStruct->error[cur_face->f1]][1],
		colormap[rawModelStruct->error[cur_face->f1]][2]);
      glVertex3d(rawModelStruct->vertices[cur_face->f1].x,
		 rawModelStruct->vertices[cur_face->f1].y,
		 rawModelStruct->vertices[cur_face->f1].z); 
      
      glColor3f(colormap[rawModelStruct->error[cur_face->f2]][0],
		colormap[rawModelStruct->error[cur_face->f2]][1],
		colormap[rawModelStruct->error[cur_face->f2]][2]);
      glVertex3d(rawModelStruct->vertices[cur_face->f2].x,
		 rawModelStruct->vertices[cur_face->f2].y,
		 rawModelStruct->vertices[cur_face->f2].z);       
    }
    glEnd();
    glEndList();
  }    

}


/* ************************************************************ */
/* Here is the callback function when mouse buttons are pressed */
/* or released. It does nothing else than store their state     */
/* ************************************************************ */
void RawWidget::mousePressEvent(QMouseEvent *event)
{
  if(event->button() & LeftButton){
    left_button_state=1;
    oldx=event->x();
    oldy=event->y();
  }
else 
  left_button_state=0;
  if(event->button() & RightButton){
    right_button_state=1;
    oldx=event->x();
    oldy=event->y();
  }
else 
  right_button_state=0;
  if(event->button() & MidButton){
    middle_button_state=1;
    oldx=event->x();
    oldy=event->y();
  }
else 
  middle_button_state=0;
}

/* ********************************************************* */
/* Callback function when the mouse is dragged in the window */
/* Only does sthg when a button is pressed                   */
/* ********************************************************* */
void RawWidget::mouseMoveEvent(QMouseEvent *event) {
  int dx,dy;

  dx= event->x() - oldx;
  dy= event->y() - oldy;



  if(left_button_state==1){  
    dth = dx*0.5; 
    dph = dy*0.5;
    glPushMatrix(); 
    glLoadIdentity();
    glRotated(dth, 0.0, 1.0, 0.0);
    glRotated(dph, 1.0, 0.0, 0.0);
    glMultMatrixd(mvmatrix); 
    glGetDoublev(GL_MODELVIEW_MATRIX, mvmatrix); 
    glPopMatrix(); 
    glDraw();
  }
  else if (middle_button_state == 1) {
    distance += dy*dstep;
    glDraw();
  }
  else if (right_button_state == 1) { 
    dpsi = -dx*0.5;
    glPushMatrix(); /* Save transform context */
    glLoadIdentity();
    glRotated(dpsi, 0.0, 0.0, 1.0); /* Modify roll angle */
    glMultMatrixd(mvmatrix);
    glGetDoublev(GL_MODELVIEW_MATRIX, mvmatrix); /* Get the final matrix */
    glPopMatrix(); /* Reload previous transform context */
    glDraw(); 
  }

  if(move_state==1)
    emit(transfervalue(distance, mvmatrix));
  oldx = event->x();
  oldy = event->y();

}

void RawWidget::keyPressEvent(QKeyEvent *k) {
  GLint line_state;
  GLboolean light_state;
  GLfloat amb[] = {0.5, 0.5, 0.5, 1.0};
  GLfloat dif[] = {0.5, 0.5, 0.5, 1.0};
  GLfloat spec[] = {0.5, 0.5, 0.5, 0.5};
  GLfloat ldir[] = {0.0, 0.0, 0.0, 0.0};
  GLfloat amb_light[] = {0.6, 0.6, 0.6, 1.0};
  GLfloat lpos[] = {0.0, 0.0, -distance, 0.0} ;
  info_vertex *curv;
  int i;
  
  if (k->key()==Key_F1){
    glGetIntegerv(GL_POLYGON_MODE, &line_state);
    if (line_state==GL_FILL) {
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      glDraw();
    }
    else if (line_state==GL_LINE) {
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      glDraw(); 
    } else 
      printf("Invalid value in GL_POLYGON_MODE : %d\n", line_state);
    
  } 
  if (k->key()==Key_F2){
    light_state = glIsEnabled(GL_LIGHTING);
    if (light_state==GL_FALSE){
      if (rawModelStruct->normals !=NULL){	
	glEnable(GL_LIGHTING);
	glLightfv(GL_LIGHT0, GL_AMBIENT, amb); 
	glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
	glLightfv(GL_LIGHT0, GL_SPECULAR, spec);
	glLightfv(GL_LIGHT0, GL_POSITION, lpos);  
	glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, ldir);  
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT,amb_light);
	glEnable(GL_LIGHT0);
	glFrontFace(GL_CCW);
      } else { // Attempt to compute normals
	if (rawModelStruct->area==NULL)
	  rawModelStruct->area = 
	    (double*)malloc(rawModelStruct->num_vert*sizeof(double));
	curv = 
	  (info_vertex*)malloc(rawModelStruct->num_vert*sizeof(info_vertex));
	if (rawModelStruct->face_normals==NULL)
	  rawModelStruct->face_normals = compute_face_normals(rawModelStruct, 
							      curv);
	if (rawModelStruct->face_normals == NULL) {
	  fprintf(stderr, "Unable to compute normals\n");
	  free(rawModelStruct->area);
	  glDisable(GL_LIGHTING); // Just to be sure
	}
	else { // Compute a normal for each vertex
	  compute_vertex_normal(rawModelStruct, curv, 
				rawModelStruct->face_normals);
	  for (i=0; i<rawModelStruct->num_vert; i++) 
	    free(curv[i].list_face);
	  free(curv);	
	  free(rawModelStruct->face_normals);
	  free(rawModelStruct->area);
	  
	  glEnable(GL_LIGHTING);
	  glLightfv(GL_LIGHT0, GL_AMBIENT, amb); 
	  glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
	  glLightfv(GL_LIGHT0, GL_SPECULAR, spec);
	  glLightfv(GL_LIGHT0, GL_POSITION, lpos);  
	  glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, ldir);  
	  glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
	  glLightModelfv(GL_LIGHT_MODEL_AMBIENT,amb_light);
	  glEnable(GL_LIGHT0);
	  glFrontFace(GL_CCW);
	}
      }
    }
    else if (light_state==GL_TRUE){
      glDisable(GL_LIGHTING);
      glFrontFace(GL_CCW);
    }
    glDraw();
  }
  if (k->key()==Key_F3){
    if (move_state==0) {
      move_state=1;
      emit(transfervalue(distance,mvmatrix));
    }
    else
      move_state=0;
  }  
  
}


