/* $Id: RawWidget.cpp,v 1.63 2003/01/13 12:36:32 aspert Exp $ */


/*
 *
 *  Copyright (C) 2001-2002 EPFL (Swiss Federal Institute of Technology,
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







#include <RawWidget.h>
#include <qmessagebox.h>
#include <qapplication.h>
#include <colormap.h>
#include <geomutils.h>
#include <xalloc.h>
#include <assert.h>

// 
// This is a derived class from QGLWidget used to render models
// 

RawWidget::RawWidget(struct model_error *model_err, int renderType, 
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
  csp = ColorMapWidget::HSV;
  colormap = colormap_hsv(CMAP_LENGTH);

  // Get the structure containing the model
  this->model = model_err;

  // Get the flags
  renderFlag = renderType;
  error_mode = VERTEX_ERROR;

  // Initialize the state
  move_state=0;
  not_orientable_warned = 0;
  two_sided_material = 1;
  etex_id = NULL;
  etex_sz = NULL;
  downsampling = 1;
  gl_initialized = FALSE;

  // Compute the center of the bounding box of the model
  add_v(&(model_err->mesh->bBox[0]), &(model_err->mesh->bBox[1]), &center);
  prod_v(0.5, &center, &center);

  // Center the model around (0, 0, 0)
  for (i=0; i<model_err->mesh->num_vert; i++) 
    substract_v(&(model_err->mesh->vertices[i]), &center, 
		&(model_err->mesh->vertices[i]));

  
  
  // This should be enough to see the whole model when starting
  distance = dist_v(&(model_err->mesh->bBox[0]), &(model_err->mesh->bBox[1]))/
    tan(FOV*M_PI_2/180.0);


  // This is the increment used when moving closer/farther from the object
  dstep = distance*0.01;

   // Connect the timer stuff
  demo_mode_timer = new QTimer(this);
  timer_state = 0;
  timer_speed = 1;
  connect(demo_mode_timer, SIGNAL(timeout()), this,
          SLOT(handleTimerEvent()));

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
    emit(transferValue(distance, mvmatrix));
  } else
    move_state = 0;
}


void RawWidget::setLine(bool state) {
  // state=TRUE -> switch to line
  // state=FALSE -> switch to fill
  GLint line_state[2]; // front and back values


  if (!gl_initialized) {
    fprintf(stderr,
            "RawWidget::setLine() called before GL context is initialized!\n");
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
  checkGlErrors("setLine(bool)");
  updateGL();
}

void RawWidget::setLight(bool state) {
  GLboolean light_state;

  if (!gl_initialized) {
    fprintf(stderr,
            "RawWidget::setLight() called before GL context is initialized!\n");
    return;
  }

  // Get state from renderer
  if ((renderFlag & RW_CAPA_MASK) ==  RW_LIGHT_TOGGLE) {
    makeCurrent();
    light_state = glIsEnabled(GL_LIGHTING);
    if (light_state != !state) // harmless
      printf("Mismatched state between qcbLight/light_state\n");
    if (light_state==GL_FALSE){ // We are now switching to lighted mode
      if (model->mesh->normals !=NULL){// Are these ones computed ?
	glEnable(GL_LIGHTING);
      } else {// Normals should have been computed
        fprintf(stderr,"ERROR: normals were not computed!\n");
      }
    }
    else if (light_state==GL_TRUE){// We are now switching to
                                   // non-lighted mode
      glDisable(GL_LIGHTING);
    }
    checkGlErrors("setLight()");
    updateGL();
  }
}

void RawWidget::setColorMap(int newSpace) {
  if (newSpace != csp) {
    csp = (ColorMapWidget::colorSpace)newSpace;
    free(colormap);
    if (csp == ColorMapWidget::HSV)
      colormap = colormap_hsv(CMAP_LENGTH);
    else if (csp == ColorMapWidget::GRAYSCALE)
      colormap = colormap_gs(CMAP_LENGTH);
    else
      fprintf(stderr, "Invalid color spce specified\n");
    if (error_mode != SAMPLE_ERROR) {
      makeCurrent();
      // display wait cursor while rebuilding list (useful for n=1 only)
      QApplication::setOverrideCursor(Qt::waitCursor);
      rebuildList();
      QApplication::restoreOverrideCursor();
    } else {
      genErrorTextures();
    }
    updateGL();
  }
}

void RawWidget::setTimer(bool state) {
  if (state) {
    timer_state = 1;
    if (move_state == 1)
      emit toggleSync(); // avoid to re-compute all parameters for
                         // all RawWidget's (the slot is connected
                         // to the same signal from ScreenWidget)
    demo_mode_timer->start(100); // Set the step top 100ms
  }
  else {
    timer_state = 0;
    demo_mode_timer->stop();
  }
    
}


// Returns the ceil(log(v)/log(2)), if v is zero or less it returns zero
int RawWidget::ceilLog2(int v) {
  int i=0;

  v -= 1;
  while ((v >> i) > 0) 
    i++;
  
  return i;
}

// creates the error texture for fe and stores it in texture
int RawWidget::fillTexture(const struct face_error *fe,
                           GLubyte *texture) const {
  int i,j,i2,j2,k,n,sz,cidx;
  GLubyte r=0, g=0, b=0; // to keep compiler happy
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
    sz = 1<<ceilLog2(n);
    drange = model->max_error - model->min_error;
    if (drange < FLT_MIN*100) drange = 1;
    for (i2=-1, k=0; i2<=sz; i2++) {
      i = (i2 >= 0) ? ((i2 < sz) ? i2 : sz-1) : 0;
      for (j2=-1; j2<=sz; j2++) {
        j = (j2 >= 0) ? ((j2 < sz) ? j2 : sz-1) : 0;
        if (i<n && j<(n-i)) { /* sample point */
          cidx = (int) (CMAP_LENGTH*(fe->serror[j+i*(2*n-i+1)/2]-
                                     model->min_error)/drange);
          if (cidx >= CMAP_LENGTH) cidx = CMAP_LENGTH-1;
          r = (GLubyte) (255*colormap[cidx][0]);
          g = (GLubyte) (255*colormap[cidx][1]);
          b = (GLubyte) (255*colormap[cidx][2]);
        } else if (j == n-i) {
          /* diagonal border texel, can be used in GL_LINEAR texture mode */
          e1 = (i>0 && j>0) ? fe->serror[(j-1)+(i-1)*(2*n-(i-1)+1)/2] : 0;
          e2 = (j>0) ? fe->serror[(j-1)+i*(2*n-i+1)/2] : 0;
          e3 = (i>0) ? fe->serror[j+(i-1)*(2*n-(i-1)+1)/2] : 0;
          cidx = (int) (CMAP_LENGTH*(e2+e3-e1-model->min_error)/drange);
          if (cidx < 0) {
            cidx = 0;
          } else if (cidx >= CMAP_LENGTH) {
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
  QString tmps;
  int i;

  // Only in error mapping mode and if not disabled
  if ((renderFlag & RW_CAPA_MASK) != RW_ERROR_ONLY) return;
  makeCurrent(); // make sure we use the correct GL context
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
  max_n = 1<<ceilLog2(max_n); // round (towards infinity) to power of two
  // Test if OpenGL implementation can deal with maximum texture size
  // Unfortunately GL_PROXY_TEXTURE_2D fails on IRIX 6.2 for some SGI
  // machines, so use older GL_MAX_TEXTURE_SIZE method.
  glGetIntegerv(GL_MAX_TEXTURE_SIZE,&tw);
  checkGlErrors("error texture size check");
  if (tw < max_n) {
    tmps.sprintf("The OpenGL implementation does not support\n"
                 "the required texture size (%ix%i).\n"
                 "Using plain white color",max_n,max_n);
    QMessageBox::critical(this,"OpenGL texture size exceeded",tmps);
    // Displaying another window can change the current GL context 
    makeCurrent();
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
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
  }
  checkGlErrors("error texture generation");
  free(texture);
  QApplication::restoreOverrideCursor();
}

// Sets the GL color corresponding to the error.
void RawWidget::setGlColorForError(float error) const {
  int cidx;
  float mine,drange;

  mine = model->min_error;
  if (error >= mine) {
    drange = model->max_error-model->min_error;
    cidx = (int) (CMAP_LENGTH*(error-mine)/drange);
    if (cidx >= CMAP_LENGTH) cidx = CMAP_LENGTH-1;
    glColor3fv(colormap[cidx]);
  } else {
    glColor3f(no_err_value,no_err_value,no_err_value); /* gray */
  }
}

// Draws triangles with the mean face error.
void RawWidget::drawMeanFaceErrorT() const {
  int k;
  face_t *cur_face;

  glBegin(GL_TRIANGLES);
  for (k=0; k<model->mesh->num_faces; k++) {
    cur_face = &(model->mesh->faces[k]);
    glVertex3f(model->mesh->vertices[cur_face->f0].x,
               model->mesh->vertices[cur_face->f0].y,
               model->mesh->vertices[cur_face->f0].z);
    glVertex3f(model->mesh->vertices[cur_face->f1].x,
               model->mesh->vertices[cur_face->f1].y,
               model->mesh->vertices[cur_face->f1].z); 
    if (model->fe[k].sample_freq > 0) {
      setGlColorForError(model->fe[k].mean_error);
    } else {
      glColor3f(no_err_value,no_err_value,no_err_value); /* gray */
    }
    glVertex3f(model->mesh->vertices[cur_face->f2].x,
               model->mesh->vertices[cur_face->f2].y,
               model->mesh->vertices[cur_face->f2].z);
  }
  glEnd();
}

// Draws triangles with vertex error, using downsampling to reduce the total
// number of triangles. If downsampling is one, each error sample will get
// drawn.
void RawWidget::drawVertexErrorT() const {
  int k,i,j,jmax,n;
  vertex_t u,v;
  vertex_t a,b,c;
  face_t *cur_face;
  int i0,i1,i2,i3;
  int j0,j1,j2,j3;
  int l0,l1,l2,l3;
  vertex_t v0,v1,v2,v3;

  glBegin(GL_TRIANGLES);
  for (k=0; k<model->mesh->num_faces; k++) {
    n = model->fe[k].sample_freq;
    cur_face = &(model->mesh->faces[k]);
    if (n == 1 && downsampling == 1) {
      /* displaying only at triangle vertices + center */
      a = model->mesh->vertices[cur_face->f0];
      b = model->mesh->vertices[cur_face->f1];
      c = model->mesh->vertices[cur_face->f2];
      v3.x = 1/3.0*(a.x+b.x+c.x);
      v3.y = 1/3.0*(a.y+b.y+c.y);
      v3.z = 1/3.0*(a.z+b.z+c.z);

      setGlColorForError(model->verror[cur_face->f0]);
      glEdgeFlag(GL_TRUE);
      glVertex3f(a.x,a.y,a.z);
      setGlColorForError(model->verror[cur_face->f1]);
      glEdgeFlag(GL_FALSE);
      glVertex3f(b.x,b.y,b.z);
      setGlColorForError(model->fe[k].serror[0]);
      glVertex3f(v3.x,v3.y,v3.z);

      setGlColorForError(model->verror[cur_face->f0]);
      glVertex3f(a.x,a.y,a.z);
      setGlColorForError(model->fe[k].serror[0]);
      glVertex3f(v3.x,v3.y,v3.z);
      setGlColorForError(model->verror[cur_face->f2]);
      glEdgeFlag(GL_TRUE);
      glVertex3f(c.x,c.y,c.z);
      
      setGlColorForError(model->verror[cur_face->f1]);
      glVertex3f(b.x,b.y,b.z);
      setGlColorForError(model->verror[cur_face->f2]);
      glEdgeFlag(GL_FALSE);
      glVertex3f(c.x,c.y,c.z);
      setGlColorForError(model->fe[k].serror[0]);
      glVertex3f(v3.x,v3.y,v3.z);

    } else if (downsampling >= n) {
      /* displaying only at triangle vertices */
      glEdgeFlag(GL_TRUE);
      setGlColorForError(model->verror[cur_face->f0]);
      glVertex3f(model->mesh->vertices[cur_face->f0].x,
                 model->mesh->vertices[cur_face->f0].y,
                 model->mesh->vertices[cur_face->f0].z);
      setGlColorForError(model->verror[cur_face->f1]);
      glVertex3f(model->mesh->vertices[cur_face->f1].x,
                 model->mesh->vertices[cur_face->f1].y,
                 model->mesh->vertices[cur_face->f1].z); 
      setGlColorForError(model->verror[cur_face->f2]);
      glVertex3f(model->mesh->vertices[cur_face->f2].x,
                 model->mesh->vertices[cur_face->f2].y,
                 model->mesh->vertices[cur_face->f2].z);
    } else { /* displaying at error samples and triangle vertices */
      assert(n > 1);
      a = model->mesh->vertices[cur_face->f0];
      b = model->mesh->vertices[cur_face->f1];
      c = model->mesh->vertices[cur_face->f2];
      substract_v(&b,&a,&u);
      substract_v(&c,&a,&v);
      prod_v(1/(float)(n-1),&u,&u);
      prod_v(1/(float)(n-1),&v,&v);
      for (i=0; i<n-1; i+=downsampling) {
        i2 = (i+downsampling < n) ? i+downsampling : n-1;
        for (j=0, jmax=n-i-1; j<jmax; j+=downsampling) {
          if (i+j+downsampling < n) {
            i0 = i;
            j0 = j;
            i1 = i+downsampling;
            j1 = j;
            i2 = i;
            j2 = j+downsampling;
            i3 = i1;
            j3 = j2;
          } else {
            i2 = i;
            j2 = j;
            i0 = (i+downsampling < n) ? i+downsampling : n-1;
            j0 = (j>0) ? j-downsampling : j;
            assert(j0 >= 0);
            i1 = i0;
            j1 = n-1-i1;
            i3 = i;
            j3 = n-1-i3;
            assert(j3 >= 0);
          }
          l0 = j0+i0*(2*n-i0+1)/2;
          l1 = j1+i1*(2*n-i1+1)/2;
          l2 = j2+i2*(2*n-i2+1)/2;
          v0.x = a.x+i0*u.x+j0*v.x;
          v0.y = a.y+i0*u.y+j0*v.y;
          v0.z = a.z+i0*u.z+j0*v.z;
          v1.x = a.x+i1*u.x+j1*v.x;
          v1.y = a.y+i1*u.y+j1*v.y;
          v1.z = a.z+i1*u.z+j1*v.z;
          v2.x = a.x+i2*u.x+j2*v.x;
          v2.y = a.y+i2*u.y+j2*v.y;
          v2.z = a.z+i2*u.z+j2*v.z;
          if (i0 != i1 || j0 != j1) { /* avoid possible degenerate */
            setGlColorForError(model->fe[k].serror[l0]);
            glEdgeFlag(j0 == 0 && j1 == 0);
            glVertex3f(v0.x,v0.y,v0.z);
            setGlColorForError(model->fe[k].serror[l1]);
            glEdgeFlag(i1+j1 == n-1 && i2+j2 == n-1);
            glVertex3f(v1.x,v1.y,v1.z);
            setGlColorForError(model->fe[k].serror[l2]);
            glEdgeFlag(i2 == 0 && i0 == 0);
            glVertex3f(v2.x,v2.y,v2.z);
          }
          if (i3+j3 < n) {
            l3 = j3+i3*(2*n-i3+1)/2;
            v3.x = a.x+i3*u.x+j3*v.x;
            v3.y = a.y+i3*u.y+j3*v.y;
            v3.z = a.z+i3*u.z+j3*v.z;
            setGlColorForError(model->fe[k].serror[l3]);
            glEdgeFlag(i3 == 0 && i2 == 0);
            glVertex3f(v3.x,v3.y,v3.z);
            setGlColorForError(model->fe[k].serror[l2]);
            glEdgeFlag(j2 == 0 && j1 == 0);
            glVertex3f(v2.x,v2.y,v2.z);
            setGlColorForError(model->fe[k].serror[l1]);
            glEdgeFlag(i1+j1 == n-1 && i3+j3 == n-1);
            glVertex3f(v1.x,v1.y,v1.z);
          }
        }
      }
    }
  }
  glEdgeFlag(GL_TRUE); /* restore default */
  glEnd();
}

// Draws textured error, with a texel at each error sample
void RawWidget::drawTexSampleErrorT() const {
  int k;
  face_t *cur_face;

  if (etex_id == NULL || etex_sz == NULL) {
    fprintf(stderr,"Attempted to draw textures before generating them!\n");
    return;
  }
  glColor3f(1,1,1); /* white base */
  for (k=0; k<model->mesh->num_faces; k++) {
    glBindTexture(GL_TEXTURE_2D,etex_id[k]);
    cur_face = &(model->mesh->faces[k]);
    glBegin(GL_TRIANGLES);
    glTexCoord2f(0.5f/etex_sz[k],0.5f/etex_sz[k]);
    glVertex3f(model->mesh->vertices[cur_face->f0].x,
               model->mesh->vertices[cur_face->f0].y,
               model->mesh->vertices[cur_face->f0].z);
    glTexCoord2f(0.5f/etex_sz[k],(model->fe[k].sample_freq-0.5f)/etex_sz[k]);
    glVertex3f(model->mesh->vertices[cur_face->f1].x,
               model->mesh->vertices[cur_face->f1].y,
               model->mesh->vertices[cur_face->f1].z); 
    glTexCoord2f((model->fe[k].sample_freq-0.5f)/etex_sz[k],0.5f/etex_sz[k]);
    glVertex3f(model->mesh->vertices[cur_face->f2].x,
               model->mesh->vertices[cur_face->f2].y,
               model->mesh->vertices[cur_face->f2].z);
    glEnd();
  }      
}

void RawWidget::setErrorMode(int emode) {
  if ((renderFlag & RW_CAPA_MASK) == RW_ERROR_ONLY) {
    if (emode == VERTEX_ERROR || emode == MEAN_FACE_ERROR ||
        emode == SAMPLE_ERROR) {
      error_mode = emode;
      if (gl_initialized) { // only update GL if already initialized
        makeCurrent();
        QApplication::setOverrideCursor(Qt::waitCursor);
        rebuildList();
        QApplication::restoreOverrideCursor();
        updateGL();
      }
    } else {
      fprintf(stderr,"invalid mode in setErrorMode()\n");
    }
  }
}

// Sets the downsampling factor for the vertex error mode
void RawWidget::setVEDownSampling(int n) {
  if ((renderFlag & RW_CAPA_MASK) == RW_ERROR_ONLY) {
    if (n < 1) {
      fprintf(stderr,"Invalid vertex error downsampling value %i\n",n);
      return;
    }
    downsampling = n;
    if (error_mode == VERTEX_ERROR && gl_initialized) {
      makeCurrent();
      // display wait cursor while rebuilding list (useful for n=1 only)
      QApplication::setOverrideCursor(Qt::waitCursor);
      rebuildList();
      QApplication::restoreOverrideCursor();
      updateGL();
    }
  }
}

// display callback
void RawWidget::paintGL() {
  display(distance);
  checkGlErrors("paintGL()");
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
  checkGlErrors("resizeGL()");
}

// Initializations for the renderer
void RawWidget::initializeGL() { 
  static const GLfloat amb[] = {0.1f, 0.1f, 0.1f, 1.0f};
  static const GLfloat dif[] = {1.0f, 1.0f, 1.0f, 1.0f};
  static const GLfloat spec[] = {1.0f, 1.0f, 1.0f, 1.0f};
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

  rebuildList();
  
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
      fprintf(stderr,"ERROR: normals were not computed!\n");
    }
  }
  checkGlErrors("initializeGL()");
  gl_initialized = TRUE;
}


// 'display' function called by the paintGL call back
// clears the buffers, computes correct transformation matrix
// and calls the model's display list
void RawWidget::display(double dist) {
  GLfloat lpos[] = {-1.0, 1.0, 1.0, 0.0} ;

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glLoadIdentity();
  /* Set the light position relative to eye point */
  glLightfv(GL_LIGHT0, GL_POSITION, lpos);  
  glTranslated(0.0, 0.0, -dist); /* Translate the object along z axis */
  glMultMatrixd(mvmatrix); /* Perform rotation */
  glCallList(model_list);
}


// This function generates the model's display list, depending on the
// viewing parameters (light...)
void RawWidget::rebuildList() {
  // Surface material characteristics for lighted mode
  static const float front_amb_mat[4] = {0.11f, 0.06f, 0.11f, 1.0f};
  static const float front_diff_mat[4] = {0.43f, 0.47f, 0.54f, 1.0f};
  static const float front_spec_mat[4] = {0.33f, 0.33f, 0.52f, 1.0f};
  static const float front_mat_shin = 10.0f;
  static const float back_amb_mat[4] = {0.3f, 0.3f, 0.3f, 1.0f};
  static const float back_diff_mat[4] = {0.5f, 0.5f, 0.5f, 1.0f};
  static const float back_spec_mat[4] = {0.33f, 0.33f, 0.52f, 1.0f};
  static const float back_mat_shin = 10.0f;
  // Color for non-lighted mode
  static const float lighted_color[3] = {1.0f, 1.0f, 1.0f};
  // Local vars
  int i;
  face_t *cur_face;
  GLenum glerr;

  checkGlErrors("rebuildList() start");

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
      glNewList(model_list, GL_COMPILE);
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
      glEndList();
    } else {
      glNewList(model_list, GL_COMPILE);
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
      glEndList();
    }
    break;
  case RW_ERROR_ONLY:
    if (error_mode == SAMPLE_ERROR && etex_id == NULL) {
      genErrorTextures();
    }

    switch (error_mode) {
    case VERTEX_ERROR:
      glDisable(GL_TEXTURE_2D);
      glShadeModel(GL_SMOOTH);
      glNewList(model_list, GL_COMPILE);
      drawVertexErrorT();
      glEndList();
      break;
    case MEAN_FACE_ERROR:
      glDisable(GL_TEXTURE_2D);
      glShadeModel(GL_FLAT);
      glNewList(model_list, GL_COMPILE);
      drawMeanFaceErrorT();
      glEndList();
      break;
    case SAMPLE_ERROR:
      glEnable(GL_TEXTURE_2D);
      glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
      glNewList(model_list, GL_COMPILE);
      drawTexSampleErrorT();
      glEndList();
      break;
    default:
      fprintf(stderr,"Invalid error mode!\n");
    }
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


void RawWidget::handleTimerEvent() {
  makeCurrent();
  glPushMatrix(); 
  glLoadIdentity();
  glRotated(timer_speed*0.5, 0.0, 1.0, 0.0);
  glMultMatrixd(mvmatrix); 
  glGetDoublev(GL_MODELVIEW_MATRIX, mvmatrix); 
  glPopMatrix(); 
  updateGL();
  if(move_state==1)
    emit(transferValue(distance, mvmatrix));
}

void RawWidget::changeSpeed(int value) {
  timer_speed = value;
}

/* ********************************************************* */
/* Callback function when the mouse is dragged in the window */
/* Only does sthg when a button is pressed                   */
/* ********************************************************* */
void RawWidget::mouseMoveEvent(QMouseEvent *event) {
  int dx,dy;

  dx= event->x() - oldx;
  dy= event->y() - oldy;

  if (!gl_initialized) {
    fprintf(stderr,"received RawWidget::mouseMoveEvent() before GL context "
            "is initialized!\n");
    return;
  }

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
  checkGlErrors("keyPressEvent(QMouseEvent)");

  if(move_state==1)
    emit(transferValue(distance, mvmatrix));
  oldx = event->x();
  oldy = event->y();

}


void RawWidget::invertNormals(bool state) {
  GLboolean lightState=state;
  int i;

  if (!gl_initialized) {
    fprintf(stderr,"RawWidget::invertNormals() called before GL context is "
            "initialized!\n");
    return;
  }

  if ((renderFlag & RW_CAPA_MASK) == RW_LIGHT_TOGGLE) {
    makeCurrent();
    QApplication::setOverrideCursor(Qt::waitCursor);
    lightState = glIsEnabled(GL_LIGHTING);
    if (lightState == GL_TRUE) {
      for (i=0; i<model->mesh->num_vert; i++) 
	neg_v(&(model->mesh->normals[i]), &(model->mesh->normals[i]));
      
	rebuildList();
        updateGL();
    }
    QApplication::restoreOverrideCursor();
  }
}

void RawWidget::setTwoSidedMaterial(bool state) {
  
  if ((renderFlag & RW_CAPA_MASK) == RW_LIGHT_TOGGLE) {
    if (state != (bool)two_sided_material) // harmless ...
      printf("Mismatched state qcbTwoSide/two_sided_material\n");
    two_sided_material = !two_sided_material;
    if (gl_initialized) { // only update GL if already initialized
      makeCurrent();
      QApplication::setOverrideCursor(Qt::waitCursor);
      rebuildList();
      updateGL();
      QApplication::restoreOverrideCursor();
    }
  }
}

void RawWidget::keyPressEvent(QKeyEvent *k) {
  
  switch(k->key()) {
  case Key_T:
    emit toggleTimer();
    break;
  case Key_F1:
    emit toggleLine();
    break;
  case Key_F2:
    emit toggleLight();
    break;
  case Key_F3:
    // if we are going to sync make sure other widgets get out transformation
    // matrix first
    if (move_state==0) emit transferValue(distance, mvmatrix);
    // now send the signal that we need to toggle synchronization
    emit toggleSync();
    break;
  case Key_F4:
    emit toggleNormals();
    break;
  case Key_F5:
    emit toggleTwoSidedMaterial();
    break;
  default:
    break;
  }
}

// Check the OpenGl error state
void RawWidget::checkGlErrors(const char* where) {
  GLenum glerr;
  while ((glerr = glGetError()) != GL_NO_ERROR) {
    fprintf(stderr,"ERROR: at %s start: OpenGL: %s\n",where,
            gluErrorString(glerr));
  }
}
