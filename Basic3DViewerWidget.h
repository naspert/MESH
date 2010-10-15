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


#ifndef BASIC3DVIEWERWIDGET_H
#define BASIC3DVIEWERWIDGET_H

#include <qgl.h>
#include <qtimer.h>
//Added by qt3to4:
#include <QMouseEvent>
#include <QKeyEvent>

#define FOV 40.0 // Vertical field of view (in degrees) for the
                 // rendering


// This class extends QGLWidget and implements a viewer able to :
// - Handle mouse events (i.e. move the model) (Mouse*Event)
// - Rotate the model at each timer event (handleTimerEvent)
// - Switch between wireframe/filled redering modes (setLine)
// - Synchronize the viewpoint using external input (setViewParams)
//
// It is inherited by Lighted3DViewerWidget (which adds lighting
// capabilities to the surface) and Error3DViewerWidget (which is used
// to display error values directly on the surface)
class Basic3DViewerWidget : public QGLWidget
{

  Q_OBJECT

public:
  Basic3DViewerWidget(struct model *raw_model, QWidget *parent=0,
                      const char *name=0);
  virtual ~Basic3DViewerWidget() { };
  QSize sizeHint() const;
  QSize minimumSizeHint() const;
  

public slots:  
  void setLine(bool state);
  void switchSync(bool state);
  void setTimer(bool state);
  void changeSpeed(int value);

protected slots:
  void setViewParams(double, double, double, double*);
  void handleTimerEvent();

signals:
  // wired to setViewParams
  void transferViewParams(double, double, double, double*); 
  // wired to switchSync
  void toggleSync();
  // wired to setLine
  void toggleLine();
  // wired to setTimer
  void toggleTimer();

protected:
  // protected functions 
  virtual void initializeGL();
  void resizeGL( int, int );
  void paintGL();
  virtual void rebuildList();
  void mouseMoveEvent(QMouseEvent*);
  void mousePressEvent(QMouseEvent*);
  virtual void keyPressEvent(QKeyEvent*);
  void checkGLErrors(const char*) const;
  GLuint getModelList() const;
  void setModelList(GLuint);
  bool getGLInitialized() const;


private:
  // functions
  void display(double, double, double);

  // vars
  GLdouble dth, dph, dpsi; // Increments of rotation angles
  GLdouble distance, dstep; // Distance wrt. center of the bounding
                            // box and distance increment
  GLdouble tx, ty; // Translation values (for panning)
  GLint vp_w; // increment for panning (depends on the widget width)
  int oldx, oldy; // location of the last mouseEvent
  GLdouble mvmatrix[16]; // Buffer for GL_MODELVIEW_MATRIX 
  struct model *raw_model; // struct containing model info  
  QTimer *demo_mode_timer; // Timer used for "demo" mode

  // those vars are accessed in derived classes
  bool gl_initialized; // true once the GL state has been set up
  GLuint model_list; // display list index for the model 
 
  // State of various components
  int left_button_state;
  int middle_button_state;
  int right_button_state;
  int move_state;
  int timer_state;
  int timer_speed;
  

};
#endif
