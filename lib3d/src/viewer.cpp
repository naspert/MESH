/* $Id: viewer.cpp,v 1.2 2001/04/27 12:47:46 jacquet Exp $ */
#include <qpainter.h>
#include <qapplication.h>
#include <qevent.h>
#include <qgl.h>
#include <qhbox.h>
#include <qframe.h>
#include <qlayout.h>
#include <qslider.h>

#include <3dutils.h>
#include <compute_error.h>

/* ****************** */
/* Useful Global vars */
/* ****************** */
//GLfloat FOV = 40.0; /* vertical field of view */
/*GLdouble distance, dstep;  distance and incremental distance step */
//GLdouble mvmatrix[16]; /* Buffer for GL_MODELVIEW_MATRIX */
//GLuint model_list = 0; /* display lists idx storage */
double colormap[256][3];
GLfloat FOV = 40.0; /* vertical field of view */
GLdouble mvmatrix[16]; /* Buffer for GL_MODELVIEW_MATRIX */
GLuint model_list = 0; /* display lists idx storage */


/*****************************************************************************/
/*            creation d'une colormap                                        */
/*****************************************************************************/
void HSVtoRGB(double *r, double *g, double *b,double h)
{
  double f,p,q,t;
  int i;

  h/=60.0;
  i=floor(h);
  f=h-i;
  p=0.0;
  q=1-f;
  t=f;
  if(h==360)
    h=0.0;
  switch(i){
  case 0:*r=1; *g=t; *b=p;break;
  case 1:*r=q; *g=1; *b=p;break;
  case 2:*r=p; *g=1; *b=t;break;
  case 3:*r=p; *g=q; *b=1;break;
  case 4:*r=t; *g=p; *b=1;break;
  case 5:*r=1; *g=p; *b=q;break;
    
  }
  printf("%lf %lf %lf\n",*r,*g,*b);

}


/***************************************************************************/
/*        definition de la classe RawWidget                                */
/***************************************************************************/

class RawWidget : public QGLWidget
{

public:  
  RawWidget(model *raw_model,QWidget *parent=0, const char *name=0 );
  void display(double distance);
  void rebuild_list(model *raw_model);

protected:
  void initializeGL();
  void resizeGL( int, int );   
  void mouseMoveEvent(QMouseEvent*);
  void mousePressEvent(QMouseEvent*);
  void paintGL();
private:
  model *raw_model2;
  GLdouble distance,dstep;
  GLuint list;
  vertex center;
  int i;
  int oldx,oldy;

  //  int light_mode = 0;
  int left_button_state;
  int middle_button_state;
  int right_button_state;


};

RawWidget::RawWidget( model *raw_model, QWidget *parent, const char *name) 
  : QGLWidget( parent, name)
{ 
  setMinimumSize( 500, 500 );
  
  raw_model2=raw_model;
 
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
  glClearColor(0.0, 0.0, 0.0, 0.0);
  /*glColor3f(1.0, 1.0, 1.0); Settings for wireframe model */
  glFrontFace(GL_CCW);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  model_list=glGenLists(1);
  glNewList(model_list, GL_COMPILE);
  rebuild_list(raw_model2);
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
void RawWidget::rebuild_list(model *raw_model) {
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

  GLdouble dth, dph, dpsi;

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
  oldx = event->x();
  oldy = event->y();

}

/* creaion d'un Widget Colormap */
class ColorMapWidget : public QWidget
{
public:
  ColorMapWidget(QWidget *parent=0, const char *name=0 );
protected:
  void paintEvent(QPaintEvent *);
};

ColorMapWidget::ColorMapWidget(QWidget *parent, const char *name)
  :QWidget(parent,name)
{
  setMinimumSize( 20, 500 );
  setMaximumSize( 20, 500 );
  setBackgroundColor( black );
}

void ColorMapWidget::paintEvent(QPaintEvent *)
{
  QPainter p;
  p.begin(this);

  for(int i=0; i<256;i++){
    p.setBrush(QColor(255*colormap[i][0],255*colormap[i][1],255*colormap[i][2]));
    p.setPen(QColor(floor(255*colormap[i][0]),floor(255*colormap[i][1]),floor(255*colormap[i][2])));
    p.drawRect(0,i*2,20,2);
  }

//   for(int j=0; j<78;j++){
//     QColor c;
//     c.setHsv( j*5, 255, 255 );		// rainbow effect
//     p.setBrush( c );
//     p.setPen(c);
//     p.drawRect(25,j*2,20,2);
//   }
    p.end();
}


/* creation d'une box ou on met le glwidget */

class MyWidget : public QWidget
{
public:
  MyWidget( model *raw_model1,model *raw_model2,QWidget *parent=0, const char *name=0 );

};

MyWidget::MyWidget( model *raw_model1,model *raw_model2,QWidget *parent, const char *name ) : QWidget(parent,name)
{
  setMinimumSize( 1100, 500 );
//   setMaximumSize( 1000, 500 );
  
  RawWidget *w = new RawWidget(raw_model1,this,"w");  
  RawWidget *y = new RawWidget(raw_model2,this,"y");
  ColorMapWidget *z = new ColorMapWidget(this,"z");


  QHBoxLayout *hlayout = new QHBoxLayout( this, 20, 20,"hlayout");
  hlayout->addWidget(z,1);
  hlayout->addWidget( w,1);
  hlayout->addWidget(y,1);


}

/*****************************************************************************/
/***********   fonction principale                                      ******/
/*****************************************************************************/


int main( int argc, char **argv )
{
  char *in_filename1, *in_filename2;
  model *raw_model1, *raw_model2;
  int grid;
  cellules *cell;
  double samplethin,dcourant,dmax=0,superdmax=0,dmin=200,superdmin=200;;
  int **repface;
  int i,j,h=0;
  sample *sample2;
  double r,g,b,hue;

  for(hue=0.0;hue<240.0;hue=hue+240.0/256.0){
    HSVtoRGB(&r,&g,&b,hue);
    colormap[255-i][0]=r;
    colormap[255-i][1]=g;
    colormap[255-i][2]=b;
    i++;
  }


  if (argc!=4) {
    printf("nbre d'arg incorrect\n");
    printf("le 1er argument correspond a l'objet de plus basse resolution\n");
    printf("le 2nd argument correspond a l'objet de plus haute resolution\n");
    printf("le 3eme argument correspond au pas d'echantillonnage\n");
    exit(-1);
  }

  in_filename1 = argv[1];
  in_filename2 = argv[2];
  samplethin=atof(argv[3]);


  raw_model1=read_raw_model(in_filename1);
  raw_model2=read_raw_model(in_filename2);
  

  if(raw_model2->num_faces<1000)
    grid=10;
  else if(raw_model2->num_faces<10000)
    grid=20;
  else if(raw_model2->num_faces<100000)
    grid=30;

  cell=liste(grid,raw_model2);
  repface=cublist(cell,grid,raw_model2);
  
  for(i=0;i<raw_model1->num_faces;i++) {
    sample2=echantillon(raw_model1->vertices[raw_model1->faces[i].f0],
			raw_model1->vertices[raw_model1->faces[i].f1],
			raw_model1->vertices[raw_model1->faces[i].f2],
			samplethin); 
    for(j=0;j<sample2->nbsamples;j++){
      dcourant=pcd(sample2->sample[j],raw_model2,repface,grid);
     if(dcourant>dmax)
       dmax=dcourant;
     if(dcourant<dmin)
       dmin=dcourant;
     h++;
    }
    printf("face numero %d: dmax= %lf\n",i+1,dmax);
    if(dmax>superdmax)
      superdmax=dmax;
    dmax=0;
   if(dmin<superdmin)
     superdmin=dmin;
   dmin=200;
   
   free(sample2->sample);
   
   free(sample2);
   
  }
  printf("distance maximale: %lf\n",superdmax);
  printf("distance minimale: %lf\n",superdmin); 
  printf("nbsampleteste: %d\n",h);

  raw_model1->error=(int *)malloc(raw_model1->num_vert*sizeof(int));
  raw_model2->error=(int *)calloc(raw_model2->num_vert,sizeof(int));

  for(i=0;i<raw_model1->num_vert;i++) {
    dcourant=pcd(raw_model1->vertices[i],raw_model2,repface,grid);
    raw_model1->error[i]=(dcourant-superdmin)/(superdmax-superdmin)*255;
  }

  QApplication::setColorSpec( QApplication::CustomColor );
  QApplication a( argc, argv );  
  
  MyWidget w(raw_model1,raw_model2);
  a.setMainWidget( &w );
  w.show();
  return a.exec();
}
