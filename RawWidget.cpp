#include <RawWidget.h>



/***************************************************************************/
/*        definition de la classe RawWidget                                */
/***************************************************************************/

RawWidget::RawWidget(model *raw_model, QWidget *parent, const char *name) 
  : QGLWidget( parent, name)
{ 
  
  
  setMinimumSize( 512, 512 );
  setMaximumSize( 512, 512 );
  FOV = 40;
  model_list=0;
  colormap=HSVtoRGB();
  raw_model2=raw_model;
  move_state=0;
  line_state=0;
  light_mode=0;
  center.x = 0.5*(raw_model2->bBox[1].x + raw_model2->bBox[0].x);
  center.y = 0.5*(raw_model2->bBox[1].y + raw_model2->bBox[0].y);
  center.z = 0.5*(raw_model2->bBox[1].z + raw_model2->bBox[0].z);

  
  for (i=0; i<raw_model2->num_vert; i++) {
    raw_model2->vertices[i].x -= center.x;
    raw_model2->vertices[i].y -= center.y;
    raw_model2->vertices[i].z -= center.z;
  }
  
  distance = dist(raw_model2->bBox[0], raw_model2->bBox[1])/
    tan(FOV*M_PI_2/180.0);
  
  dstep = distance*0.01;

}

void RawWidget::aslot()
{
  if(move_state==0){
    move_state=1;
    emit transfervalue(distance,mvmatrix);
  }
  else
    move_state=0;
}


void RawWidget::transfer(double dist,double *mvmat)
{

  distance=dist;
  memcpy(mvmatrix, mvmat, 16*sizeof(double)); 
  glDraw();
}

void RawWidget::setLine()
{

  printf("\nsetline: appel no %d\n",j);
  j++;
  glGetIntegerv(GL_POLYGON_MODE,&state);
  if(state==GL_FILL) {
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    printf("FILL->LINE\n");
    glDraw();
  }
  else if(state==GL_LINE) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glGetIntegerv(GL_POLYGON_MODE,&state);
    printf("LINE->FILL\n");
    printf("state: %d\n",state);
    glDraw(); 
  } else {
    printf("Uh ?\n");
  }
}

void RawWidget::setLight()
{
  GLfloat amb[] = {0.5, 0.5, 0.5, 1.0};
  GLfloat dif[] = {0.5, 0.5, 0.5, 1.0};
  GLfloat spec[] = {0.5, 0.5, 0.5, 0.5};
  GLfloat ldir[] = {0.0, 0.0, 0.0, 0.0};
//   GLfloat mat_spec[] = {0.3, 0.7, 0.5, 0.5};
  GLfloat amb_light[] = {0.6, 0.6, 0.6, 1.0};
//   GLfloat shine[] = {0.6};
  GLfloat lpos[] = {0.0, 0.0, -distance, 0.0} ;

  
  light_state=glIsEnabled(GL_LIGHTING);
  if(light_state==FALSE){
    if(raw_model2->face_normals !=NULL){
//       printf("light state= %d\n",light_mode);

      glEnable(GL_LIGHTING);
      glLightfv(GL_LIGHT0, GL_AMBIENT, amb); 
      glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
      glLightfv(GL_LIGHT0, GL_SPECULAR, spec);
      glLightfv(GL_LIGHT0, GL_POSITION, lpos);  
      glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, ldir);  
      glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
      glLightModelfv(GL_LIGHT_MODEL_AMBIENT,amb_light);
//       glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_spec);
//       glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shine);
      glEnable(GL_LIGHT0);
      //     glColor3f(1.0, 1.0, 1.0);
      glFrontFace(GL_CCW);
//       glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
//       light_mode=1;
    } else {
//       printf("light state= %d\n",light_mode);
      light_mode=0;
      printf("Unable to compute normals... non_manifold model\n");
    }
  }
  else if(light_state==TRUE){
//     printf("light state= %d\n",light_mode);
//     light_mode=0;
    glDisable(GL_LIGHTING);
    glFrontFace(GL_CCW);
//     glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  }
  glDraw();
}


void RawWidget::paintGL()
{
  display(distance);
}

void RawWidget::resizeGL(int width ,int height)
{
  glViewport(0, 0, width, height);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(FOV, (GLdouble)width/(GLdouble)height, distance/10.0, 
		 10.0*distance);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  
}

void RawWidget::initializeGL()
{ 
  glDepthFunc(GL_LESS);
  glEnable(GL_DEPTH_TEST);
  glShadeModel(GL_SMOOTH);
//   glShadeModel(GL_FLAT);

  glClearColor(0.0, 0.0, 0.0, 0.0);
  /*glColor3f(1.0, 1.0, 1.0); Settings for wireframe model */
//   glFrontFace(GL_CCW);
//   glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  model_list=glGenLists(1);
  glNewList(model_list, GL_COMPILE);
  rebuild_list(colormap,raw_model2);
  glEndList();
  
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(FOV, 1.0, distance/10.0, 10.0*distance);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glGetDoublev(GL_MODELVIEW_MATRIX, mvmatrix); /* Initialize the temp matrix */
}  

/* ***************************************************************** */
/* Display function : clear buffers, build correct MODELVIEW matrix, */
/* call display list and swap the buffers                            */
/* ***************************************************************** */
void RawWidget::display(double distance) {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glLoadIdentity();
  glTranslated(0.0, 0.0, -distance); /* Translate the object along z */
  glMultMatrixd(mvmatrix); /* Perform rotation */
  glCallList(model_list);
}


/* ************************************************************* */
/* This functions rebuilds the display list of the current model */
/* It is called when the viewing setting (light...) are changed  */
/* ************************************************************* */
void RawWidget::rebuild_list(double **colormap,model *raw_model) {
  int i;
  face *cur_face;

  /*  if (glIsList(list) == GL_TRUE)
    glDeleteLists(list,1);
	       
    glNewList(list, GL_COMPILE);*/
  glBegin(GL_TRIANGLES);
  
  if(raw_model2->face_normals!=NULL){
    for (i=0; i<raw_model->num_faces; i++) {
      cur_face = &(raw_model->faces[i]);
      glColor3f(1.0,1.0,1.0);

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
  } else {
    for (i=0; i<raw_model->num_faces; i++) {
      cur_face = &(raw_model->faces[i]);
      
      glColor3f(colormap[raw_model->error[cur_face->f0]][0],
		colormap[raw_model->error[cur_face->f0]][1],
		colormap[raw_model->error[cur_face->f0]][2]);  
      glVertex3d(raw_model->vertices[cur_face->f0].x,
		 raw_model->vertices[cur_face->f0].y,
		 raw_model->vertices[cur_face->f0].z); 
      
      glColor3f(colormap[raw_model->error[cur_face->f1]][0],
		colormap[raw_model->error[cur_face->f1]][1],
		colormap[raw_model->error[cur_face->f1]][2]);
      glVertex3d(raw_model->vertices[cur_face->f1].x,
		 raw_model->vertices[cur_face->f1].y,
		 raw_model->vertices[cur_face->f1].z); 
      
      glColor3f(colormap[raw_model->error[cur_face->f2]][0],
		colormap[raw_model->error[cur_face->f2]][1],
		colormap[raw_model->error[cur_face->f2]][2]);
      glVertex3d(raw_model->vertices[cur_face->f2].x,
		 raw_model->vertices[cur_face->f2].y,
		 raw_model->vertices[cur_face->f2].z);       
    }
  }    
  glEnd();
  /*glEndList();*/
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
void RawWidget::mouseMoveEvent(QMouseEvent *event)
{
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
    emit transfervalue(distance,mvmatrix);
  oldx = event->x();
  oldy = event->y();

}

void RawWidget::keyPressEvent(QKeyEvent *k)
{
  GLfloat amb[] = {0.5, 0.5, 0.5, 1.0};
  GLfloat dif[] = {0.5, 0.5, 0.5, 1.0};
  GLfloat spec[] = {0.5, 0.5, 0.5, 0.5};
  GLfloat ldir[] = {0.0, 0.0, 0.0, 0.0};
//   GLfloat mat_spec[] = {0.3, 0.7, 0.5, 0.5};
  GLfloat amb_light[] = {0.6, 0.6, 0.6, 1.0};
//   GLfloat shine[] = {0.6};
  GLfloat lpos[] = {0.0, 0.0, -distance, 0.0} ;

  if(k->key()==Key_F1){
    glGetIntegerv(GL_POLYGON_MODE,&state);
    if(state==GL_FILL) {
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      printf("FILL->LINE\n");
      glDraw();
    }
    else if(state==GL_LINE) {
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      glGetIntegerv(GL_POLYGON_MODE,&state);
      printf("LINE->FILL\n");
      printf("state: %d\n",state);
      glDraw(); 
    } else {
      printf("Uh ?\n");
    }
  } 
  if(k->key()==Key_F2){
    light_state=glIsEnabled(GL_LIGHTING);
    if(light_state==FALSE){
      if(raw_model2->face_normals !=NULL){	
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
      } else {
	light_mode=0;
	printf("Unable to compute normals... non_manifold model\n");
      }
    }
    else if(light_state==TRUE){
    glDisable(GL_LIGHTING);
    glFrontFace(GL_CCW);
    }
    glDraw();
  }
  if(k->key()==Key_F3){
    if(move_state==0){
      move_state=1;
      emit transfervalue(distance,mvmatrix);
    }
    else
      move_state=0;
  }  

}


