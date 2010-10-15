/* $Id$ */

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

#include "Error3DViewerWidget.h"
#include <qmessagebox.h>
#include <qapplication.h>
#include "colormap.h"
#include "geomutils.h"
#include "xalloc.h"
#include <assert.h>

// Number of colors in the colormap
#define CMAP_LENGTH 256


Error3DViewerWidget::Error3DViewerWidget(struct model_error *model_err, 
					 bool tex_enabled, QWidget *parent,
					 const char *name)
  : Basic3DViewerWidget(model_err->mesh, parent, name), no_err_value(0.25)
{
  // Build the colormap used to display the mean error onto the surface of
  // the model
  csp = ColorMapWidget::HSV;
  colormap = colormap_hsv(CMAP_LENGTH);

  error_mode = VERTEX_ERROR;

  model = model_err;
  texture_enabled = tex_enabled;

  // Initialize the state
  etex_id = NULL;
  etex_sz = NULL;
  downsampling = 1;
}


Error3DViewerWidget::~Error3DViewerWidget()
{
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

// slot to toggle the error mode (per vertex, per face, per sample)
void Error3DViewerWidget::setErrorMode(int emode) 
{
  if (emode == VERTEX_ERROR || emode == MEAN_FACE_ERROR ||
      emode == SAMPLE_ERROR) {
    error_mode = emode;
    if (getGLInitialized()) { // only update GL if already initialized
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

// Sets the downsampling factor for the vertex error mode
void Error3DViewerWidget::setVEDownSampling(int n) 
{

  if (n < 1) {
    fprintf(stderr,"Invalid vertex error downsampling value %i\n",n);
    return;
  }
  downsampling = n;
  if (error_mode == VERTEX_ERROR && getGLInitialized()) {
    makeCurrent();
    // display wait cursor while rebuilding list (useful for n=1 only)
    QApplication::setOverrideCursor(Qt::waitCursor);
    rebuildList();
    QApplication::restoreOverrideCursor();
    updateGL();
  }
}

// slot to change the colormap
void Error3DViewerWidget::setColorMap(int newSpace) 
{
  if (newSpace != csp) {
    csp = (ColorMapWidget::colorSpace)newSpace;
    free(colormap);
    if (csp == ColorMapWidget::HSV)
      colormap = colormap_hsv(CMAP_LENGTH);
    else if (csp == ColorMapWidget::GRAYSCALE)
      colormap = colormap_gs(CMAP_LENGTH);
    else
      fprintf(stderr, "Invalid color space specified\n");
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

// Herited from Basic3DViewerWidget
void Error3DViewerWidget::initializeGL() 
{
  Basic3DViewerWidget::initializeGL();
   if (!model->info->closed) 
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
}

// Re-implemented from Basic3DViewerWidget, since we have to handle
// texture/downsampling stuff from the beginning.
void Error3DViewerWidget::rebuildList()
{
  GLenum glerr;
  GLuint list = getModelList();

  checkGLErrors("rebuildList() start");
 // Get a display list, if we don't have one yet.
  if (list == 0) {
    list=glGenLists(1);
    if (list == 0) {
      QMessageBox::critical(this,"GL error",
                            "No OpenGL display list available.\n"
                            "Cannot display model!");
      return;
    }
  }
  
  setModelList(list);

  switch (error_mode) {
  case VERTEX_ERROR:
    glDisable(GL_TEXTURE_2D);
    glShadeModel(GL_SMOOTH);
    glNewList(list, GL_COMPILE);
    drawVertexErrorT();
    glEndList();
    break;
  case MEAN_FACE_ERROR:
    glDisable(GL_TEXTURE_2D);
    glShadeModel(GL_FLAT);
    glNewList(list, GL_COMPILE);
    drawMeanFaceErrorT();
    glEndList();
    break;
  case SAMPLE_ERROR:
    if (etex_id == NULL) 
      genErrorTextures();
    glEnable(GL_TEXTURE_2D);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
    glNewList(list, GL_COMPILE);
    drawTexSampleErrorT();
    glEndList();
    break;
  default:
    fprintf(stderr,"Invalid error mode!\n");
  }
  // Check for errors in display list generation
  while ((glerr = glGetError()) != GL_NO_ERROR) {
    if (glerr == GL_OUT_OF_MEMORY) {
      QMessageBox::critical(this,"GL error",
                            "Out of memory generating display list.\n"
                            "Cannot display");
      glDeleteLists(list, 1);
      setModelList(0);
    } else {
      fprintf(stderr,"ERROR: OpenGL error while generating display list:\n%s",
              gluErrorString(glerr));
    }
  }
}

// Generate the texture for error display
void Error3DViewerWidget::genErrorTextures() 
{
  static const GLint internalformat = GL_R3_G3_B2;
  GLubyte *texture;
  GLint tw,max_n;
  QString tmps;
  int i;

  // Only in error mapping mode and if not disabled
  if (!texture_enabled) 
    return;
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
  checkGLErrors("error texture size check");
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
  checkGLErrors("error texture generation");
  free(texture);
  QApplication::restoreOverrideCursor();
}

// creates the error texture for fe and stores it in texture
int Error3DViewerWidget::fillTexture(const struct face_error *fe,
				     GLubyte *texture) const 
{
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

// Returns the ceil(log(v)/log(2)), if v is zero or less it returns zero
int Error3DViewerWidget::ceilLog2(int v)
{
  int i=0;

  v -= 1;
  while ((v >> i) > 0) 
    i++;
  
  return i;
}

// Sets the GL color corresponding to the error.
void Error3DViewerWidget::setGLColorForError(float error) const 
{
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
void Error3DViewerWidget::drawMeanFaceErrorT() const 
{
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
      setGLColorForError(model->fe[k].mean_error);
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
void Error3DViewerWidget::drawVertexErrorT() const 
{
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

      setGLColorForError(model->verror[cur_face->f0]);
      glEdgeFlag(GL_TRUE);
      glVertex3f(a.x,a.y,a.z);
      setGLColorForError(model->verror[cur_face->f1]);
      glEdgeFlag(GL_FALSE);
      glVertex3f(b.x,b.y,b.z);
      setGLColorForError(model->fe[k].serror[0]);
      glVertex3f(v3.x,v3.y,v3.z);

      setGLColorForError(model->verror[cur_face->f0]);
      glVertex3f(a.x,a.y,a.z);
      setGLColorForError(model->fe[k].serror[0]);
      glVertex3f(v3.x,v3.y,v3.z);
      setGLColorForError(model->verror[cur_face->f2]);
      glEdgeFlag(GL_TRUE);
      glVertex3f(c.x,c.y,c.z);
      
      setGLColorForError(model->verror[cur_face->f1]);
      glVertex3f(b.x,b.y,b.z);
      setGLColorForError(model->verror[cur_face->f2]);
      glEdgeFlag(GL_FALSE);
      glVertex3f(c.x,c.y,c.z);
      setGLColorForError(model->fe[k].serror[0]);
      glVertex3f(v3.x,v3.y,v3.z);

    } else if (downsampling >= n) {
      /* displaying only at triangle vertices */
      glEdgeFlag(GL_TRUE);
      setGLColorForError(model->verror[cur_face->f0]);
      glVertex3f(model->mesh->vertices[cur_face->f0].x,
                 model->mesh->vertices[cur_face->f0].y,
                 model->mesh->vertices[cur_face->f0].z);
      setGLColorForError(model->verror[cur_face->f1]);
      glVertex3f(model->mesh->vertices[cur_face->f1].x,
                 model->mesh->vertices[cur_face->f1].y,
                 model->mesh->vertices[cur_face->f1].z); 
      setGLColorForError(model->verror[cur_face->f2]);
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
            setGLColorForError(model->fe[k].serror[l0]);
            glEdgeFlag(j0 == 0 && j1 == 0);
            glVertex3f(v0.x,v0.y,v0.z);
            setGLColorForError(model->fe[k].serror[l1]);
            glEdgeFlag(i1+j1 == n-1 && i2+j2 == n-1);
            glVertex3f(v1.x,v1.y,v1.z);
            setGLColorForError(model->fe[k].serror[l2]);
            glEdgeFlag(i2 == 0 && i0 == 0);
            glVertex3f(v2.x,v2.y,v2.z);
          }
          if (i3+j3 < n) {
            l3 = j3+i3*(2*n-i3+1)/2;
            v3.x = a.x+i3*u.x+j3*v.x;
            v3.y = a.y+i3*u.y+j3*v.y;
            v3.z = a.z+i3*u.z+j3*v.z;
            setGLColorForError(model->fe[k].serror[l3]);
            glEdgeFlag(i3 == 0 && i2 == 0);
            glVertex3f(v3.x,v3.y,v3.z);
            setGLColorForError(model->fe[k].serror[l2]);
            glEdgeFlag(j2 == 0 && j1 == 0);
            glVertex3f(v2.x,v2.y,v2.z);
            setGLColorForError(model->fe[k].serror[l1]);
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
void Error3DViewerWidget::drawTexSampleErrorT() const 
{
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
