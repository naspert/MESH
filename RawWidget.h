#ifndef RAWWIDGET_H
#define RAWWIDGET_H

#include <3dutils.h>
#include <qgl.h>
#include <qevent.h>
#include <ColorMap.h>
#include <qkeycode.h>
#include <qnamespace.h>


class RawWidget : public QGLWidget 
{ 

  Q_OBJECT // To please 'moc' but useless otherwise ...
public:  
  RawWidget(model *raw_model,QWidget *parent=0, const char *name=0);
  void display(double distance);
  void rebuild_list(double **colormap,model *raw_model);
  
public slots: 
  void aslot();
  void setLine();
  void setLight();
  void transfer(double dist,double *mvmat);
  /*     void setFill(); */
  
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
  GLdouble dth, dph, dpsi;
  double **colormap;
  model *raw_model2;
  GLdouble distance,dstep;
  GLuint list;
  vertex center;
  int i;
  int oldx,oldy;
  GLfloat FOV; /* vertical field of view */
  GLdouble mvmatrix[16]; /* Buffer for GL_MODELVIEW_MATRIX */
  GLuint model_list; /* display lists idx storage */

  int light_mode;
  int line_state; /* 0->FILL 1->LINE */
  int left_button_state;
  int middle_button_state;
  int right_button_state;
  int move_state;
  GLint state;
  GLboolean light_state;
  int j;

};

#endif
