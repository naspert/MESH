/* $Id: RawWidget.h,v 1.8 2001/08/07 12:32:22 aspert Exp $ */
#ifndef RAWWIDGET_H
#define RAWWIDGET_H

#include <3dutils.h>
#include <qgl.h>
#include <qevent.h>
#include <ColorMap.h>
#include <qkeycode.h>
#include <qnamespace.h>

#define FOV 40.0 // Vertical field of view for the rendering

class RawWidget : public QGLWidget 
{ 

  Q_OBJECT 

public:  
  RawWidget(model *raw_model,QWidget *parent=0, const char *name=0);

  
public slots: 
  void aslot();
  void setLine();
  void setLight();
  void transfer(double dist,double *mvmat);

  
signals:
  void transfervalue(double,double*);
    
   
protected:
  void initializeGL();
  void resizeGL( int, int );   
  void mouseMoveEvent(QMouseEvent*);
  void mousePressEvent(QMouseEvent*);
  void keyPressEvent(QKeyEvent*);
  void paintGL();

private:  
  void display(double distance);
  void rebuild_list();

  GLdouble dth, dph, dpsi;
  double **colormap;
  model *rawModelStruct;
  GLdouble distance, dstep;
  int oldx,oldy;
  GLdouble mvmatrix[16]; /* Buffer for GL_MODELVIEW_MATRIX */

  GLuint model_list; /* display list index for the model */


  int left_button_state;
  int middle_button_state;
  int right_button_state;

  int move_state;



};

#endif
