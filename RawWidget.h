/* $Id: RawWidget.h,v 1.37 2002/08/30 09:18:40 aspert Exp $ */


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







#ifndef RAWWIDGET_H
#define RAWWIDGET_H

#include <compute_error.h>
#include <qgl.h>
#include <ColorMapWidget.h>

#define FOV 40.0 // Vertical field of view for the rendering

// These two flags are exclusive. Either the widget can toggle from
// 'light' to 'wireframe' mode or it displays the error on the model
#define RW_LIGHT_TOGGLE 0x02
#define RW_ERROR_ONLY 0x01

// Bitmask to check the capacity of the widget
#define RW_CAPA_MASK 0x03

#define RW_DISPLAY_MASK 0x30

// Colormap length
#define CMAP_LENGTH 256

class RawWidget : public QGLWidget 
{ 

  Q_OBJECT 

public:  
  RawWidget(struct model_error *model_err, int renderType, QWidget *parent=0, 
	    const char *name=0); // Constructor
  ~RawWidget(); // Destructor
  QSize sizeHint() const;
  QSize minimumSizeHint() const;

  enum ErrorMode { VERTEX_ERROR = 0,
                   MEAN_FACE_ERROR = 1,
                   SAMPLE_ERROR = 2 };

public slots: 
  void setLine(bool state);
  void switchSync(bool state);
  void transfer(double dist,double *mvmat);
  void setErrorMode(int emode);
  void setVEDownSampling(int n);
  void invertNormals(bool state);
  void setTwoSidedMaterial(bool state);
  void setLight(bool state);
  void setColorMap(int newSpace);

signals:
  void transferValue(double,double*);
  void toggleSync();
  void toggleLight();
  void toggleLine();
  void toggleTwoSidedMaterial();
  void toggleNormals();
   
protected:
  void initializeGL();
  void resizeGL( int, int );   
  void mouseMoveEvent(QMouseEvent*);
  void mousePressEvent(QMouseEvent*);
  void keyPressEvent(QKeyEvent*);
  void paintGL();

private:  
// functions 
  void display(double dist);
  void rebuildList();
  static void checkGlErrors(const char* where);
  void genErrorTextures();
  int fillTexture(const struct face_error *fe, GLubyte *texture) const;
  static int ceilLog2(int v);
  void setGlColorForError(float error) const;
  void drawMeanFaceErrorT() const;
  void drawVertexErrorT() const;
  void drawTexSampleErrorT() const;

// vars
  int renderFlag; // flag to indicate whether the widget can be set in
  // the lighted mode or not
  GLdouble dth, dph, dpsi;
  float **colormap;
  ColorMapWidget::colorSpace csp;
  struct model_error *model;
  GLdouble distance, dstep;
  int oldx,oldy;
  GLdouble mvmatrix[16]; // Buffer for GL_MODELVIEW_MATRIX 
  GLuint model_list; // display list index for the model 
  GLuint *etex_id; // texture IDs for per triangle sample error
  int *etex_sz;    // texture size for each of etex_id textures
  const GLfloat no_err_value; // gray value for when there is no error for
                              // primitive
  int downsampling; // downsampling factor for vertex error
// state vars
  int left_button_state;
  int middle_button_state;
  int right_button_state;
  int move_state;
  int computed_normals; // flag if normals have been computed or loaded
  int not_orientable_warned;
  int two_sided_material;
  int error_mode;
  bool gl_initialized; // true once the GL state has been set up

};

#endif

