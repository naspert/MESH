/* $Id: Basic3DViewerWidget.cpp,v 1.3 2004/04/30 07:50:20 aspert Exp $ */

/*
 *
 *  Copyright (C) 2001-2004 EPFL (Swiss Federal Institute of Technology,
 *  Lausanne) This program is free software; you can redistribute it
 *  and/or modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2 of
 *  the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 *  USA.
 *
 *  In addition, as a special exception, EPFL gives permission to link
 *  the code of this program with the Qt non-commercial edition library
 *  (or with modified versions of Qt non-commercial edition that use the
 *  same license as Qt non-commercial edition), and distribute linked
 *  combinations including the two.  You must obey the GNU General
 *  Public License in all respects for all of the code used other than
 *  Qt non-commercial edition.  If you modify this file, you may extend
 *  this exception to your version of the file, but you are not
 *  obligated to do so.  If you do not wish to do so, delete this
 *  exception statement from your version.
 *
 *  Authors : Nicolas Aspert, Diego Santa-Cruz and Davy Jacquet
 *
 *  Web site : http://mesh.epfl.ch
 *
 *  Reference :
 *   "MESH : Measuring Errors between Surfaces using the Hausdorff distance"
 *   in Proceedings of IEEE Intl. Conf. on Multimedia and Expo (ICME) 2002, 
 *   vol. I, pp. 705-708, available on http://mesh.epfl.ch
 *
 */


#include <Basic3DViewerWidget.h>
#include <qmessagebox.h>
#include <qapplication.h>
#include <geomutils.h>




Basic3DViewerWidget::Basic3DViewerWidget(struct model *r_model, 
                                         QWidget *parent, const char *name)
  : QGLWidget(parent, name)
{
  int i;
  vertex_t center;

  // Get as big as possible screen space
  setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));

  // 0 is not a valid display list index
  model_list = 0;

  // Get the structure containing the model
  raw_model = r_model;

  // Initialize the state
  move_state=0;
  gl_initialized = FALSE;

   // Compute the center of the bounding box of the model
  add_v(&(raw_model->bBox[0]), &(raw_model->bBox[1]), &center);
  prod_v(0.5, &center, &center);

  // Center the model around (0, 0, 0)
  for (i=0; i<raw_model->num_vert; i++) 
    substract_v(&(raw_model->vertices[i]), &center, 
		&(raw_model->vertices[i]));

  
  
  // This should be enough to see the whole model when starting
  distance = dist_v(&(raw_model->bBox[0]), &(raw_model->bBox[1]))/
    tan(FOV*M_PI_2/180.0);
  tx = 0;
  ty = 0;
  vp_w = 1;

  // This is the increment used when moving closer/farther from the object
  dstep = distance*0.01;

  // Connect the timer stuff
  demo_mode_timer = new QTimer(this);
  timer_state = 0;
  timer_speed = 1;
  connect(demo_mode_timer, SIGNAL(timeout()), this,
          SLOT(handleTimerEvent()));
}

QSize Basic3DViewerWidget::sizeHint() const 
{
  return QSize(512, 512);
}

QSize Basic3DViewerWidget::minimumSizeHint() const
{
  return QSize(256, 256);
}


// slot to switch between wireframe and filled render modes
void Basic3DViewerWidget::setLine(bool state) 
{
  // state=TRUE -> switch to line
  // state=FALSE -> switch to fill
  GLint line_state[2]; // front and back values


  if (!gl_initialized) {
    fprintf(stderr,
            "Basic3DViewerWidget::setLine() called before GL context"
	    " is initialized!\n");
    return;
  }

  // Forces the widget to be the current context. Undefined otherwise
  // and this causes a _silly_ behaviour !
  makeCurrent();

  glGetIntegerv(GL_POLYGON_MODE,line_state);
  if (line_state[0]==GL_FILL && line_state[1]==GL_FILL && state==TRUE) {
    
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    QApplication::setOverrideCursor(Qt::waitCursor);
    rebuildList();
    QApplication::restoreOverrideCursor();
  } else if (line_state[0]==GL_LINE && line_state[1]==GL_LINE && 
	     state==FALSE) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    QApplication::setOverrideCursor(Qt::waitCursor);
    rebuildList();
    QApplication::restoreOverrideCursor();
  } else {
    printf("Invalid state value found for GL_POLYGON_MODE: %d %d\n",
           line_state[0],line_state[1]);
    return;
  }
  checkGLErrors("setLine(bool)");
  updateGL();
}

// Slot for synchronizing viewpoints between widgets
void Basic3DViewerWidget::switchSync(bool state) 
{
  if (state) {
    move_state = 1;
    emit(transferViewParams(distance, tx, ty, mvmatrix));
  } else
    move_state = 0;
}


// Sets the viewing parameters. mvmat is a pointer to a 4x4 matrix 
void Basic3DViewerWidget::setViewParams(double dist, double translX, 
				  	double translY, 
                                        double *mvmat)
{
  
  distance = dist;
  tx = translX;
  ty = translY;
  // Copy the 4x4 transformation matrix
  memcpy(mvmatrix, mvmat, 16*sizeof(double)); 
  // update display
  updateGL();

}

// Slot for starting/stopping the timer
void Basic3DViewerWidget::setTimer(bool state) 
{
  if (state) {
    timer_state = 1;
    if (move_state == 1)
      emit toggleSync(); // avoid to re-compute all parameters for
                         // all 3DWidget's (the slot is connected
                         // to the same signal from ScreenWidget)
    demo_mode_timer->start(100); // Set the timer step top 100ms
  }
  else {
    timer_state = 0;
    demo_mode_timer->stop();
  }
    
}

// Slot connected to the timer events
void Basic3DViewerWidget::handleTimerEvent() 
{
  makeCurrent();
  glPushMatrix(); 
  glLoadIdentity();
  glRotated(timer_speed*0.5, 0.0, 1.0, 0.0);
  glMultMatrixd(mvmatrix); 
  glGetDoublev(GL_MODELVIEW_MATRIX, mvmatrix); 
  glPopMatrix(); 
  updateGL();
  if(move_state==1)
    emit(transferViewParams(distance, tx, ty, mvmatrix));
}

// slot to change the frequency of the timer events
void Basic3DViewerWidget::changeSpeed(int value) 
{
  timer_speed = value;
}

// Performs the initialization of OpenGL parameters (lighting and
// initial viewing transformation)
void Basic3DViewerWidget::initializeGL()
{

  static const GLfloat amb_light0[] = {0.1f, 0.1f, 0.1f, 1.0f};
  static const GLfloat dif_light0[] = {1.0f, 1.0f, 1.0f, 1.0f};
  static const GLfloat spec_light0[] = {1.0f, 1.0f, 1.0f, 1.0f};
  static const GLfloat global_amb_light[] = {0.8f, 0.8f, 0.8f, 1.0f};

  glDepthFunc(GL_LESS);
  glEnable(GL_DEPTH_TEST);
  glShadeModel(GL_SMOOTH);


  glClearColor(0.0, 0.0, 0.0, 0.0);

  glLightfv(GL_LIGHT0, GL_AMBIENT, amb_light0); 
  glLightfv(GL_LIGHT0, GL_DIFFUSE, dif_light0);
  glLightfv(GL_LIGHT0, GL_SPECULAR, spec_light0);
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, global_amb_light);
  glEnable(GL_LIGHT0);
  glFrontFace(GL_CCW);


  rebuildList();
  
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(FOV, 1.0, distance/10.0, 10.0*distance);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glGetDoublev(GL_MODELVIEW_MATRIX, mvmatrix); // Initialize the temp matrix 
  checkGLErrors("initializeGL()");
  gl_initialized = TRUE;
}


// resize callback
void Basic3DViewerWidget::resizeGL(int width ,int height) 
{
  glViewport(0, 0, width, height);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(FOV, (GLdouble)width/(GLdouble)height, distance/10.0, 
		 10.0*distance);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  vp_w = width; //for panning
  checkGLErrors("resizeGL()");
}


// display callback
void Basic3DViewerWidget::paintGL() 
{
  display(distance, tx, ty);
  checkGLErrors("paintGL()");
}


void Basic3DViewerWidget::rebuildList()
{
  // Color for non-lighted mode
  static const float lighted_color[3] = {1.0f, 1.0f, 1.0f};
  // Local vars
  int i;
  face_t *cur_face;
  GLenum glerr;


  checkGLErrors("rebuildList() start");

  // Get a display list, if we don't have one yet.
  if (model_list == 0) {
    model_list=glGenLists(1);
    if (model_list == 0) {
      QMessageBox::critical(this,"GL error",
                            "No OpenGL display list available.\n"
                            "Cannot display model!");
      return;
    }
  }
  
  glColor3fv(lighted_color);
   
  if (raw_model->normals != NULL) {
    glNewList(model_list, GL_COMPILE);
    glBegin(GL_TRIANGLES);  
    for (i=0; i<raw_model->num_faces; i++) {
      cur_face = &(raw_model->faces[i]);
      
      
      glNormal3f(raw_model->normals[cur_face->f0].x,
		 raw_model->normals[cur_face->f0].y,
		 raw_model->normals[cur_face->f0].z);
      glVertex3f(raw_model->vertices[cur_face->f0].x,
		 raw_model->vertices[cur_face->f0].y,
		 raw_model->vertices[cur_face->f0].z); 
      
      glNormal3f(raw_model->normals[cur_face->f1].x,
		 raw_model->normals[cur_face->f1].y,
		 raw_model->normals[cur_face->f1].z);  
      glVertex3f(raw_model->vertices[cur_face->f1].x,
		 raw_model->vertices[cur_face->f1].y,
		 raw_model->vertices[cur_face->f1].z); 
      
      glNormal3f(raw_model->normals[cur_face->f2].x,
		 raw_model->normals[cur_face->f2].y,
		 raw_model->normals[cur_face->f2].z); 
      glVertex3f(raw_model->vertices[cur_face->f2].x,
		 raw_model->vertices[cur_face->f2].y,
		 raw_model->vertices[cur_face->f2].z);       
    }
    glEnd();
    glEndList();
  } else {
    glNewList(model_list, GL_COMPILE);
    glBegin(GL_TRIANGLES);  
    for (i=0; i<raw_model->num_faces; i++) {
      cur_face = &(raw_model->faces[i]);
      
      glVertex3f(raw_model->vertices[cur_face->f0].x,
		 raw_model->vertices[cur_face->f0].y,
		 raw_model->vertices[cur_face->f0].z); 
      
      glVertex3f(raw_model->vertices[cur_face->f1].x,
		 raw_model->vertices[cur_face->f1].y,
		 raw_model->vertices[cur_face->f1].z); 
      
      glVertex3f(raw_model->vertices[cur_face->f2].x,
		 raw_model->vertices[cur_face->f2].y,
		 raw_model->vertices[cur_face->f2].z);       
    }
    glEnd();
    glEndList();
  }

  while ((glerr = glGetError()) != GL_NO_ERROR) {
    if (glerr == GL_OUT_OF_MEMORY) {
      QMessageBox::critical(this,"GL error",
                            "Out of memory generating display list.\n"
                            "Cannot display");
      glDeleteLists(model_list,1);
      model_list = 0;
    } else {
      fprintf(stderr,"ERROR: OpenGL error while generating display list:\n%s",
              gluErrorString(glerr));
    }
  }
    
}

// Callback function when the mouse is dragged in the window 
// AND  a button is pressed
void Basic3DViewerWidget::mouseMoveEvent(QMouseEvent *event) 
{
  int dx,dy;

  dx = event->x() - oldx;
  dy = event->y() - oldy;

  if (!gl_initialized) {
    fprintf(stderr,"received mouseMoveEvent() before GL context "
            "is initialized!\n");
    return;
  }

  makeCurrent();
  if (timer_state == 0) { // When in "demo" mode, the mousemove events
                          // _have_ to be ignored
    if(left_button_state==1 && (event->state() & ControlButton)){  
      tx -= ((double)dx/vp_w)*2*distance*tan(FOV/360.0*M_PI);
      ty += ((double)dy/vp_w)*2*distance*tan(FOV/360.0*M_PI);
      updateGL();
    }
    else if(left_button_state==1) {  
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
    else if (middle_button_state==1) {
      distance += dy*dstep;
      updateGL();
    }
    else if (right_button_state==1) { 
      dpsi = -dx*0.5;
      glPushMatrix(); // Save transform context 
      glLoadIdentity();
      glRotated(dpsi, 0.0, 0.0, 1.0); // Modify roll angle 
      glMultMatrixd(mvmatrix);
      glGetDoublev(GL_MODELVIEW_MATRIX, mvmatrix); // Get the final matrix 
      glPopMatrix(); // Reload previous transform context 
      updateGL();
    }
  }
  checkGLErrors("keyPressEvent(QMouseEvent)");

  if(move_state==1)
    emit(transferViewParams(distance, tx, ty, mvmatrix));
  oldx = event->x();
  oldy = event->y();

}

// Callback called when a mouse button changes of state
void Basic3DViewerWidget::mousePressEvent(QMouseEvent *event)
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

// Key events handler (overloaded by derived classes)
void Basic3DViewerWidget::keyPressEvent(QKeyEvent *k)
{
  
  switch(k->key()) {
  case Key_T:
    emit toggleTimer();
    break;
  case Key_F1:
    emit toggleLine();
    break;
  case Key_F3:
    // if we are going to sync make sure other widgets get out transformation
    // matrix first
    if (move_state==0) emit transferViewParams(distance, tx, ty, mvmatrix);
    // now send the signal that we need to toggle synchronization
    emit toggleSync();
    break;
  default:
    break;
  }
}


// Check the OpenGl error state
void Basic3DViewerWidget::checkGLErrors(const char* where) const
{
  GLenum glerr;
  while ((glerr = glGetError()) != GL_NO_ERROR) {
    fprintf(stderr,"ERROR: at %s start: OpenGL: %s\n",where,
            gluErrorString(glerr));
  }
}

// Basic accessors
GLuint Basic3DViewerWidget::getModelList() const
{
  return model_list;
}

void Basic3DViewerWidget::setModelList(GLuint list)
{
    this->model_list = list;
}

bool Basic3DViewerWidget::getGLInitialized() const
{
  return gl_initialized;
}

// 'display' function called by the paintGL callback
// clears the buffers, computes correct transformation matrix
// and calls the model's display list
void Basic3DViewerWidget::display(double dist, double tX, double tY) 
{
  GLfloat lpos[] = {-1.0, 1.0, 1.0, 0.0} ;

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glLoadIdentity();
  // Set the light position relative to eye point 
  glLightfv(GL_LIGHT0, GL_POSITION, lpos);  
  glTranslated(-tX, -tY, -dist); // Translate the object along z axis and pan
  glMultMatrixd(mvmatrix); // Perform rotation 
  glCallList(model_list);
}
