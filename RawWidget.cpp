#include <RawWidget.h>



/***************************************************************************/
/*        definition de la classe RawWidget                                */
/***************************************************************************/

RawWidget::RawWidget(model *raw_model, QWidget *parent, const char *name) 
  : QGLWidget( parent, name)
{ 
  
  
  setMinimumSize( 500, 500 );
  setMaximumSize( 500, 500 );
  
  FOV = 40;
  model_list=0;
  colormap=HSVtoRGB();
  raw_model2=raw_model;
  line_fill_state=0;
  move_state=0;
 
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
  if(move_state==0)
    move_state=1;
  else
    move_state=0;
  emit transfervalue(distance,mvmatrix);
}

void RawWidget::transfer(double dist,double *mvmat)
{
int i;

distance=dist;

for(i=0;i<16;i++)
  mvmatrix[i]=mvmat[i];
//   glMatrixMode(GL_MODELVIEW);
//   glLoadIdentity();
//   glMultMatrixd(mvmatrix);
 
  updateGL();
}

void RawWidget::setLine()
{
  if(line_fill_state==0){
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    line_fill_state=1;
  }
  else {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    line_fill_state=0;
  }
  updateGL();
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
  glFrontFace(GL_CCW);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

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
    updateGL();
  }
  else if (middle_button_state == 1) {
    distance += dy*dstep;
    updateGL();
  }
  else if (right_button_state == 1) { 
    dpsi = -dx*0.5;
    glPushMatrix(); /* Save transform context */
    glLoadIdentity();
    glRotated(dpsi, 0.0, 0.0, 1.0); /* Modify roll angle */
    glMultMatrixd(mvmatrix);
    glGetDoublev(GL_MODELVIEW_MATRIX, mvmatrix); /* Get the final matrix */
    glPopMatrix(); /* Reload previous transform context */
    updateGL(); 
  }

  if(move_state==1)
    emit transfervalue(distance,mvmatrix);
  oldx = event->x();
  oldy = event->y();

}

void RawWidget::keyPressEvent(QKeyEvent *event)
{
  printf("%d ",event->key());
  event->accept();
}


