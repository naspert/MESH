/* $Id: RawWidget.cpp,v 1.39 2002/02/21 09:26:25 dsanta Exp $ */

#include <RawWidget.h>
#include <qmessagebox.h>
#include <qapplication.h>
#include <colormap.h>
#include <geomutils.h>
#include <xalloc.h>

// 
// This is a derived class from QGLWidget used to render models
// 

RawWidget::RawWidget(struct model_error *model, int renderType, 
		     QWidget *parent, const char *name)
  :QGLWidget(parent, name), no_err_value(0.25) { 
  
  int i;
  vertex_t center;

  // Get as big as possible screen space
  setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));

  // 0 is not a valid display list index
  model_list = 0;

  // Build the colormap used to display the mean error onto the surface of
  // the model
  colormap = colormap_hsv(CMAP_LENGTH);

  // Get the structure containing the model
  this->model = model;

  // Get the flags
  renderFlag = renderType;
  error_mode = VERTEX_ERROR;

  // Initialize the state
  move_state=0;
  not_orientable_warned = 0;
  two_sided_material = 1;
  etex_id = NULL;
  etex_sz = NULL;

  // Compute the center of the bounding box of the model
  add_v(&(model->mesh->bBox[0]), &(model->mesh->bBox[1]), &center);
  prod_v(0.5, &center, &center);

  // Center the model around (0, 0, 0)
  for (i=0; i<model->mesh->num_vert; i++) 
    substract_v(&(model->mesh->vertices[i]), &center, 
		&(model->mesh->vertices[i]));

  
  
  // This should be enough to see the whole model when starting
  distance = dist_v(&(model->mesh->bBox[0]), &(model->mesh->bBox[1]))/
    tan(FOV*M_PI_2/180.0);


  // This is the increment used when moving closer/farther from the object
  dstep = distance*0.01;




}

QSize RawWidget::sizeHint() const {
  return QSize(512,512);
}

QSize RawWidget::minimumSizeHint() const {
  return QSize(256,256);
}

RawWidget::~RawWidget() {
  // NOTE: Don't delete display lists and/or texture bindings here, it
  // sometimes leads to coredumps on some combinations of QT and libGL. In any
  // case those resources will be freed when the GL context is finally
  // destroyed (normally just before this method returns). In addition, it is
  // much faster to let the GL context do it (especially with a large number
  // of textures, as is often the case in sample error texture mode).
  free_colormap(colormap);
  free(etex_id);
  free(etex_sz);
}

void RawWidget::transfer(double dist,double *mvmat) {

  distance = dist;
  // Copy the 4x4 transformation matrix
  memcpy(mvmatrix, mvmat, 16*sizeof(double)); 
  // update display
  updateGL();
}

void RawWidget::switchSync(bool state) {
  if (state) {
    move_state = 1;
    emit(transfervalue(distance, mvmatrix));
  } else
    move_state = 0;
}


void RawWidget::setLine(bool state) {
  // state=TRUE -> switch to line
  // state=FALSE -> switch to fill
  GLint line_state[2]; // front and back values

  // Forces the widget to be the current context. Undefined otherwise
  // and this causes a _silly_ behaviour !
  makeCurrent();

  glGetIntegerv(GL_POLYGON_MODE,line_state);
  if (line_state[0]==GL_FILL && line_state[1]==GL_FILL && state==TRUE) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  } else if (line_state[0]==GL_LINE && line_state[1]==GL_LINE && 
	     state==FALSE) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  } else {
    printf("Invalid state value found for GL_POLYGON_MODE: %d %d\n",
           line_state[0],line_state[1]);
    return;
  }
  check_gl_errors("setLine(bool)");
  updateGL();
}

void RawWidget::setLight() {
  GLboolean light_state;

  // Get state from renderer
  if ((renderFlag & RW_CAPA_MASK) ==  RW_LIGHT_TOGGLE) {
    makeCurrent();
    light_state = glIsEnabled(GL_LIGHTING);

    if (light_state==GL_FALSE){ // We are now switching to lighted mode
      if (model->mesh->normals !=NULL){// Are these ones computed ?
	glEnable(GL_LIGHTING);
      } else {// Normals should have been computed
        fprintf(stderr,"ERROR: normals where not computed!\n");
      }
    }
    else if (light_state==GL_TRUE){// We are now switching to wireframe mode
      glDisable(GL_LIGHTING);
    }
    check_gl_errors("setLight()");
    updateGL();
  }
}

// Returns the ceil(log(v)/log(2)), if v is zero or less it returns zero
int RawWidget::ceil_log2(int v) {
  int i;
  i = 0;
  v -= 1;
  while ((v >> i) > 0) {
    i++;
  }
  return i;
}

// creates the error texture for fe and stores it in texture
int RawWidget::fillTexture(const struct face_error *fe,
                           GLubyte *texture) const {
  int i,j,i2,j2,k,n,sz,cidx;
  GLubyte r,g,b;
  float drange;
  float e1,e2,e3;

  n = fe->sample_freq;
  if (n == 0) { /* no samples, using no error value gray */
    for (i=0, k=0; i<9; i++) { /* set border and center to no_err_value */
      texture[k++] = (GLubyte) (255*no_err_value);
      texture[k++] = (GLubyte) (255*no_err_value);
      texture[k++] = (GLubyte) (255*no_err_value);
    }
    return 1;
  } else {
    sz = 1<<ceil_log2(n);
    drange = model->max_error-model->min_error;
    if (drange < FLT_MIN*100) drange = 1;
    r = g = b = 0; // to keep compiler happy
    for (i2=-1, k=0; i2<=sz; i2++) {
      i = (i2 >= 0) ? ((i2 < sz) ? i2 : sz-1) : 0;
      for (j2=-1; j2<=sz; j2++) {
        j = (j2 >= 0) ? ((j2 < sz) ? j2 : sz-1) : 0;
        if (i<n && j<(n-i)) { /* sample point */
          cidx = (int) floor((CMAP_LENGTH-1)*(fe->serror[j+i*(2*n-i+1)/2]-
                                              model->min_error)/drange+0.5);
          r = (GLubyte) (255*colormap[cidx][0]);
          g = (GLubyte) (255*colormap[cidx][1]);
          b = (GLubyte) (255*colormap[cidx][2]);
        } else if (j == n-i) {
          /* diagonal border texel, can be used in GL_LINEAR texture mode */
          e1 = (i>0&&j>0) ? fe->serror[(j-1)+(i-1)*(2*n-(i-1)+1)/2] : 0;
          e2 = (j>0) ? fe->serror[(j-1)+i*(2*n-i+1)/2] : 0;
          e3 = (i>0) ? fe->serror[j+(i-1)*(2*n-(i-1)+1)/2] : 0;
          cidx = (int) floor((CMAP_LENGTH-1)*(e2+e3-e1-
                                              model->min_error)/drange+0.5);\
          if (cidx < 0) {
            cidx = 0;
          } else if (cidx > CMAP_LENGTH-1) {
            cidx = CMAP_LENGTH-1;
          }
          r = (GLubyte) (255*colormap[cidx][0]);
          g = (GLubyte) (255*colormap[cidx][1]);
          b = (GLubyte) (255*colormap[cidx][2]);
        } else { /* out of triangle point, this texel will never be used */
          r = g = b = 0; /* black */
        }
        texture[k++] = r;
        texture[k++] = g;
        texture[k++] = b;
      }
    }
    return sz;
  }
}

void RawWidget::genErrorTextures() {
  static const GLint internalformat = GL_R3_G3_B2;
  GLubyte *texture;
  GLint tw,max_n;
  int i;

  // Only in error mapping mode and if not disabled
  if ((renderFlag & RW_CAPA_MASK) != RW_ERROR_ONLY) return;
  // Allocate texture names (IDs) if not present
  if (etex_id == NULL) {
    etex_id = (GLuint*) xa_malloc(sizeof(*etex_id)*model->mesh->num_faces);
    etex_sz = (int*) xa_malloc(sizeof(*etex_sz)*model->mesh->num_faces);
    glGenTextures(model->mesh->num_faces,etex_id);
  }
  // Get maximum texture size
  max_n = 0;
  for (i=0; i<model->mesh->num_faces; i++) {
    if (max_n < model->fe[i].sample_freq) max_n = model->fe[i].sample_freq;
  }
  max_n = 1<<ceil_log2(max_n); // round (towards infinity) to power of two
  // Test if OpenGL implementation can deal with maximum texture size
  glTexImage2D(GL_PROXY_TEXTURE_2D,0,internalformat,max_n+2,max_n+2,1,GL_RGB,
               GL_UNSIGNED_BYTE,NULL);
  glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D, 0,GL_TEXTURE_WIDTH, &tw);
  check_gl_errors("error texture size check");
  if (tw == 0) {
    QString tmps;
    tmps.sprintf("The OpenGL implementation does not support\n"
                 "the required texture size (%ix%i).\n"
                 "Using plain white color",max_n,max_n);
    QMessageBox::critical(this,"OpenGL texture size exceeded",tmps);
    for (i=0; i<model->mesh->num_faces; i++) {
      etex_sz[i] = 1; // avoid having divide by zero texture coords
    }
    return;
  }
  // What follows is a potentially slow operation
  QApplication::setOverrideCursor(Qt::waitCursor);
  // Allocate temporary texture storage
  texture = (GLubyte*) xa_malloc(sizeof(*texture)*3*(max_n+2)*(max_n+2));
  glPixelStorei(GL_UNPACK_ALIGNMENT,1); /* pixel rows aligned on bytes only */
  for (i=0; i<model->mesh->num_faces; i++) {
    glBindTexture(GL_TEXTURE_2D,etex_id[i]);
    etex_sz[i] = fillTexture(&(model->fe[i]),texture);
    glTexImage2D(GL_TEXTURE_2D,0,internalformat,etex_sz[i]+2,etex_sz[i]+2,1,
                 GL_RGB,GL_UNSIGNED_BYTE,texture);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
    // Default GL_TEXTURE_MIN_FILTER requires mipmaps!
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
  }
  check_gl_errors("error texture generation");
  free(texture);
  QApplication::restoreOverrideCursor();
}

void RawWidget::setErrorMode(int emode) {
  if ((renderFlag & RW_CAPA_MASK) == RW_ERROR_ONLY) {
    if (emode == VERTEX_ERROR || emode == MEAN_FACE_ERROR ||
        emode == SAMPLE_ERROR) {
      error_mode = emode;
      makeCurrent();
      rebuild_list();
      updateGL();
    } else {
      fprintf(stderr,"invalid mode in setErrorMode()\n");
    }
  }
}

// display callback
void RawWidget::paintGL() {
  display(distance);
  check_gl_errors("paintGL()");
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
  check_gl_errors("resizeGL()");
}

// Initializations for the renderer
void RawWidget::initializeGL() { 
  static const GLfloat amb[] = {0.1f, 0.1f, 0.1f, 1.0f};
  static const GLfloat dif[] = {0.3f, 0.3f, 0.3f, 1.0f};
  static const GLfloat spec[] = {0.3f, 0.3f, 0.3f, 0.3f};
  static const GLfloat amb_light[] = {0.8f, 0.8f, 0.8f, 1.0f};

  glDepthFunc(GL_LESS);
  glEnable(GL_DEPTH_TEST);
  glShadeModel(GL_SMOOTH);


  glClearColor(0.0, 0.0, 0.0, 0.0);

  glLightfv(GL_LIGHT0, GL_AMBIENT, amb); 
  glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
  glLightfv(GL_LIGHT0, GL_SPECULAR, spec);
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT,amb_light);
  if (!model->info->closed) glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,GL_TRUE);
  glEnable(GL_LIGHT0);
  glFrontFace(GL_CCW);

  rebuild_list();
  
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(FOV, 1.0, distance/10.0, 10.0*distance);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glGetDoublev(GL_MODELVIEW_MATRIX, mvmatrix); /* Initialize the temp matrix */
  if (renderFlag == RW_LIGHT_TOGGLE) {
    if (model->mesh->normals !=NULL){// Are these ones computed ?
      if (!model->info->orientable && !not_orientable_warned) {
        not_orientable_warned = 1;
        QMessageBox::warning(this,"Not orientable model",
                             "Model is not orientable.\n"
                             "Surface shading is probably incorrect.");
      }
      glEnable(GL_LIGHTING);
    } else {// Normals should have been computed
      fprintf(stderr,"ERROR: normals where not computed!\n");
    }
  }
  check_gl_errors("initializeGL()");
}


// 'display' function called by the paintGL call back
// clears the buffers, computes correct transformation matrix
// and calls the model's display list
void RawWidget::display(double distance) {
  GLfloat lpos[] = {-1.0, 1.0, 1.0, 0.0} ;

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glLoadIdentity();
  /* Set the light position relative to eye point */
  glLightfv(GL_LIGHT0, GL_POSITION, lpos);  
  glTranslated(0.0, 0.0, -distance); /* Translate the object along z axis */
  glMultMatrixd(mvmatrix); /* Perform rotation */
  glCallList(model_list);
}


// This function generates the model's display list, depending on the
// viewing parameters (light...)
void RawWidget::rebuild_list() {
  // Surface material characteristics for lighted mode
  static const float front_amb_mat[4] = {0.5f, 0.5f, 0.5f, 1.0f};
  static const float front_diff_mat[4] = {0.7f, 0.7f, 0.7f, 1.0f};
  static const float front_spec_mat[4] = {0.3f, 0.3f, 0.3f, 1.0f};
  static const float front_mat_shin = 30.0f;
  static const float back_amb_mat[4] = {0.3f, 0.3f, 0.3f, 1.0f};
  static const float back_diff_mat[4] = {0.5f, 0.5f, 0.5f, 1.0f};
  static const float back_spec_mat[4] = {0.2f, 0.2f, 0.2f, 1.0f};
  static const float back_mat_shin = 10.0f;
  // Color for non-lighted mode
  static const float lighted_color[3] = {1.0f, 1.0f, 1.0f};
  // Local vars
  int i,cidx;
  float drange;
  face_t *cur_face;
  GLenum glerr;

  check_gl_errors("rebuild_list() start");

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
  
  switch(renderFlag & RW_CAPA_MASK) {
  case RW_LIGHT_TOGGLE:
    glNewList(model_list, GL_COMPILE);
    glColor3fv(lighted_color);
    if (two_sided_material) {
      glMaterialfv(GL_FRONT,GL_AMBIENT,front_amb_mat);
      glMaterialfv(GL_FRONT,GL_DIFFUSE,front_diff_mat);
      glMaterialfv(GL_FRONT,GL_SPECULAR,front_spec_mat);
      glMaterialf(GL_FRONT,GL_SHININESS,front_mat_shin);
      glMaterialfv(GL_BACK,GL_AMBIENT,back_amb_mat);
      glMaterialfv(GL_BACK,GL_DIFFUSE,back_diff_mat);
      glMaterialfv(GL_BACK,GL_SPECULAR,back_spec_mat);
      glMaterialf(GL_BACK,GL_SHININESS,back_mat_shin);
    } else {
      glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT,front_amb_mat);
      glMaterialfv(GL_FRONT_AND_BACK,GL_DIFFUSE,front_diff_mat);
      glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,front_spec_mat);
      glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,front_mat_shin);
    }
    if (model->mesh->normals != NULL) {
      glBegin(GL_TRIANGLES);  
      for (i=0; i<model->mesh->num_faces; i++) {
	cur_face = &(model->mesh->faces[i]);
	
	
	glNormal3f(model->mesh->normals[cur_face->f0].x,
		   model->mesh->normals[cur_face->f0].y,
		   model->mesh->normals[cur_face->f0].z);
	glVertex3f(model->mesh->vertices[cur_face->f0].x,
		   model->mesh->vertices[cur_face->f0].y,
		   model->mesh->vertices[cur_face->f0].z); 
	
	glNormal3f(model->mesh->normals[cur_face->f1].x,
		   model->mesh->normals[cur_face->f1].y,
		   model->mesh->normals[cur_face->f1].z);  
	glVertex3f(model->mesh->vertices[cur_face->f1].x,
		   model->mesh->vertices[cur_face->f1].y,
		   model->mesh->vertices[cur_face->f1].z); 
	
	glNormal3f(model->mesh->normals[cur_face->f2].x,
		   model->mesh->normals[cur_face->f2].y,
		   model->mesh->normals[cur_face->f2].z); 
	glVertex3f(model->mesh->vertices[cur_face->f2].x,
		   model->mesh->vertices[cur_face->f2].y,
		   model->mesh->vertices[cur_face->f2].z);       
      }
      glEnd();
    } else {
      glBegin(GL_TRIANGLES);  
      for (i=0; i<model->mesh->num_faces; i++) {
	cur_face = &(model->mesh->faces[i]);
	
	glVertex3f(model->mesh->vertices[cur_face->f0].x,
		   model->mesh->vertices[cur_face->f0].y,
		   model->mesh->vertices[cur_face->f0].z); 
	
	glVertex3f(model->mesh->vertices[cur_face->f1].x,
		   model->mesh->vertices[cur_face->f1].y,
		   model->mesh->vertices[cur_face->f1].z); 
	
	glVertex3f(model->mesh->vertices[cur_face->f2].x,
		   model->mesh->vertices[cur_face->f2].y,
		   model->mesh->vertices[cur_face->f2].z);       
      }
      glEnd();
    }
    glEndList();
    break;
  case RW_ERROR_ONLY:
    drange = model->max_error-model->min_error;
    if (drange < FLT_MIN*100) drange = 1;
    if (error_mode == SAMPLE_ERROR && etex_id == NULL) {
      genErrorTextures();
    }
    glNewList(model_list, GL_COMPILE);
    glShadeModel((error_mode == MEAN_FACE_ERROR) ? GL_FLAT : GL_SMOOTH);
    if (error_mode != SAMPLE_ERROR) {
      glDisable(GL_TEXTURE_2D);
      glBegin(GL_TRIANGLES);
      for (i=0; i<model->mesh->num_faces; i++) {
        cur_face = &(model->mesh->faces[i]);
        if (model->verror[cur_face->f0] >= model->min_error) {
          cidx = (int) floor((CMAP_LENGTH-1)*(model->verror[cur_face->f0]-
                                              model->min_error)/drange+0.5);
          glColor3fv(colormap[cidx]);
        } else {
          glColor3f(no_err_value,no_err_value,no_err_value); /* gray */
        }
        glVertex3f(model->mesh->vertices[cur_face->f0].x,
                   model->mesh->vertices[cur_face->f0].y,
                   model->mesh->vertices[cur_face->f0].z);
        if (model->verror[cur_face->f1] >= model->min_error) {
          cidx = (int) floor((CMAP_LENGTH-1)*(model->verror[cur_face->f1]-
                                              model->min_error)/drange+0.5);
          glColor3fv(colormap[cidx]);
        } else {
          glColor3f(no_err_value,no_err_value,no_err_value); /* gray */
        }
        glVertex3f(model->mesh->vertices[cur_face->f1].x,
                   model->mesh->vertices[cur_face->f1].y,
                   model->mesh->vertices[cur_face->f1].z); 
        if (error_mode == VERTEX_ERROR) {
          if (model->verror[cur_face->f2] >= model->min_error) {
            cidx = (int) floor((CMAP_LENGTH-1)*(model->verror[cur_face->f2]-
                                                model->min_error)/drange+0.5);
            glColor3fv(colormap[cidx]);
          } else {
            glColor3f(no_err_value,no_err_value,no_err_value); /* gray */
          }
        } else {
          if (model->fe[i].sample_freq > 0) {
            cidx = (int) floor((CMAP_LENGTH-1)*(model->fe[i].mean_error-
                                                model->min_error)/drange+0.5);
            glColor3fv(colormap[cidx]);
          } else { /* no samples in this triangle => mean error meaningless */
            glColor3f(no_err_value,no_err_value,no_err_value); /* gray */
          }
        }
        glVertex3f(model->mesh->vertices[cur_face->f2].x,
                   model->mesh->vertices[cur_face->f2].y,
                   model->mesh->vertices[cur_face->f2].z);
      }
      glEnd();
    } else {
      glColor3f(1,1,1); /* white base */
      glEnable(GL_TEXTURE_2D);
      glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
      for (i=0; i<model->mesh->num_faces; i++) {
        glBindTexture(GL_TEXTURE_2D,etex_id[i]);
        cur_face = &(model->mesh->faces[i]);
        glBegin(GL_TRIANGLES);
        glTexCoord2f(0.5f/etex_sz[i],0.5f/etex_sz[i]);
        glVertex3f(model->mesh->vertices[cur_face->f0].x,
                   model->mesh->vertices[cur_face->f0].y,
                   model->mesh->vertices[cur_face->f0].z);
        glTexCoord2f(0.5f/etex_sz[i],
                     (model->fe[i].sample_freq-0.5f)/etex_sz[i]);
        glVertex3f(model->mesh->vertices[cur_face->f1].x,
                   model->mesh->vertices[cur_face->f1].y,
                   model->mesh->vertices[cur_face->f1].z); 
        glTexCoord2f((model->fe[i].sample_freq-0.5f)/etex_sz[i],
                     0.5f/etex_sz[i]);
        glVertex3f(model->mesh->vertices[cur_face->f2].x,
                   model->mesh->vertices[cur_face->f2].y,
                   model->mesh->vertices[cur_face->f2].z);
        glEnd();
      }      
    }
    glEndList();
    break;
  default:
      fprintf(stderr, "Invalid render flag found !!\n");
      return;
  }

  // Check for errors in display list generation
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

  makeCurrent();
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
  check_gl_errors("keyPressEvent(QMouseEvent)");

  if(move_state==1)
    emit(transfervalue(distance, mvmatrix));
  oldx = event->x();
  oldy = event->y();

}

void RawWidget::keyPressEvent(QKeyEvent *k) {
  GLboolean light_state;
  int i;
  
  switch(k->key()) {
  case Key_F1:
    emit toggleLine();
    break;
  case Key_F2:
    setLight();
    break;
  case Key_F3:
    // if we are going to sync make sure other widgets get out transformation
    // matrix first
    if (move_state==0) emit transfervalue(distance, mvmatrix);
    // now send the signal that we need to toggle synchronization
    emit toggleSync();
    break;
  case Key_F4:
    makeCurrent();
    if ((renderFlag & RW_CAPA_MASK) == RW_LIGHT_TOGGLE) {
      light_state = glIsEnabled(GL_LIGHTING);
      if (light_state == GL_TRUE) { // Invert normals
	for (i=0; i<model->mesh->num_vert; i++) 
	  neg_v(&(model->mesh->normals[i]), &(model->mesh->normals[i]));
	
	rebuild_list();
        updateGL();
      }
    }
    break;
  case Key_F5:
    if ((renderFlag & RW_CAPA_MASK) == RW_LIGHT_TOGGLE) {
      makeCurrent();
      two_sided_material = !two_sided_material;
      rebuild_list();
      updateGL();
    }
    break;
  default:
    break;
  }
}

// Check the OpenGl error state
void RawWidget::check_gl_errors(const char* where) {
  GLenum glerr;
  while ((glerr = glGetError()) != GL_NO_ERROR) {
    fprintf(stderr,"ERROR: at %s start: OpenGL: %s\n",where,
            gluErrorString(glerr));
  };
}
