/* $Id: RawWidget.h,v 1.24 2002/01/30 15:41:15 aspert Exp $ */
#ifndef RAWWIDGET_H
#define RAWWIDGET_H

#include <compute_error.h>
#include <qgl.h>

#define FOV 40.0 // Vertical field of view for the rendering

// These two flags are exclusive. Either the widget can toggle from
// 'light' to 'wireframe' mode or it displays the error on the model
#define RW_LIGHT_TOGGLE 0x02
#define RW_ERROR_ONLY 0x01

// Bitmask to check the capacity of the widget
#define RW_CAPA_MASK 0x03

#define RW_DISPLAY_MASK 0x30
 
class RawWidget : public QGLWidget 
{ 

  Q_OBJECT 

public:  
  RawWidget(struct model_error *model, int renderType, QWidget *parent=0, 
	    const char *name=0); // Constructor
  ~RawWidget(); // Destructor
  QSize sizeHint() const;
  QSize minimumSizeHint() const;
  
public slots: 
  void setLine(bool state);
  void setLight();
  void switchSync(bool state);
  void transfer(double dist,double *mvmat);

  
signals:
  void transfervalue(double,double*);
  void toggleSync();
  void toggleLine();
   
protected:
  void initializeGL();
  void resizeGL( int, int );   
  void mouseMoveEvent(QMouseEvent*);
  void mousePressEvent(QMouseEvent*);
  void keyPressEvent(QKeyEvent*);
  void paintGL();

private:  
// functions 
  void display(double distance);
  void rebuild_list();
  void check_gl_errors(const char* where);

// vars
  int renderFlag; // flag to indicate whether the widget can be set in
  // the lighted mode or not
  GLdouble dth, dph, dpsi;
  double **colormap;
  struct model_error *model;
  GLdouble distance, dstep;
  int oldx,oldy;
  GLdouble mvmatrix[16]; // Buffer for GL_MODELVIEW_MATRIX 
  GLuint model_list; // display list index for the model 
// state vars
  int left_button_state;
  int middle_button_state;
  int right_button_state;
  int move_state;
  int computed_normals; // flag if normals have been computed or loaded
  int not_orientable_warned;
  int two_sided_material;
};

#endif
