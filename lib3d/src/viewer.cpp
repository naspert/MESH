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


/* computes the distance between a point and a plan defined by 3 points */
// double distance(vertex point,vertex A,vertex normal)
// {
// double k;
// double dist;

// k=-(normal.x*A.x+normal.y*A.y+normal.z*A.z);

// dist=fabs(-normal.x*point.x-normal.y*point.y-normal.z*point.z-k);

// return dist;

// }



/*****************************************************************************/
/*               fonction qui echantillonne un triangle                      */
/*****************************************************************************/

// sample* echantillon(vertex a, vertex b, vertex c,double k)
// {
//   int h=0;
//   double i,j;
//   vertex l1,l2;
//   sample *sample1;

//   if((sample1=(sample*)malloc(sizeof(sample)))==NULL){
//   printf("impossible d'allouer de la memoire");
//   exit(-1);
//   }

//   l1.x=b.x-a.x;
//   l1.y=b.y-a.y;
//   l1.z=b.z-a.z;

//   l2.x=c.x-a.x;
//   l2.y=c.y-a.y;
//   l2.z=c.z-a.z;


   
//   if((sample1->sample=(vertex*)malloc(sizeof(vertex)))==NULL){
//     printf("impossible d'allouer de la memoire");
//     exit(-1);
//   }
//   for (i=0;i<=1;i+=k) {
//     for (j=0;j<=1;j+=k) {
//       if (i+j<1.000001) {
// 	if(h>0)
// 	  sample1->sample=(vertex*)realloc(sample1->sample,(h+1)*sizeof(vertex));
//         sample1->sample[h].x=a.x+i*l1.x+j*l2.x;
//         sample1->sample[h].y=a.y+i*l1.y+j*l2.y;
//         sample1->sample[h].z=a.z+i*l1.z+j*l2.z;
//         h++;
// 	}
//     }
//   }
//   sample1->nbsamples=h;
  
//   return(sample1);

// }
/****************************************************************************/
/*     fonction qui calcule la distance d'un point a une surface            */
/****************************************************************************/
// double pcd(vertex point,model *raw_model,int **repface,int grid)
// {
// int m,n,o;
// int a,b,c;
// vertex bbox0,bbox1,A,normal;
// int cellule;
// int j=0,k=0;
// double dist,dmin=200;

// bbox0=raw_model->bBox[0];
// bbox1=raw_model->bBox[1];

//  m=(point.x-bbox0.x)*grid/(bbox1.x-bbox0.x);
//  n=(point.y-bbox0.y)*grid/(bbox1.y-bbox0.y);
//  o=(point.z-bbox0.z)*grid/(bbox1.z-bbox0.z);
 
//  if(m==grid)
//    m=grid-1;
//  if(n==grid)
//    n=grid-1;
//  if(o==grid)
//    o=grid-1;
 
//  cellule=m+n*grid+o*grid*grid;
 /*printf("cellule: %d\n",cellule);*/

//  while(dmin==200){
//    for(c=o-k;c<=o+k;c++){ 
//      for(b=n-k;b<=n+k;b++){
//        for(a=m-k;a<=m+k;a++){
// 	 cellule=a+b*grid+c*grid*grid;
// 	 j=0;
// 	 if(cellule>=0 && cellule<grid*grid*grid){ 
// 	   while(repface[cellule][j]!=-1){
// 	     A=raw_model->vertices[raw_model->faces[repface[cellule][j]].f0];
// 	     normal=raw_model->face_normals[repface[cellule][j]];
	     
	     
// 	     dist=distance(point,A,normal);
	     /*printf("cellule: %d\n",cellule);
	       printf("%lf %lf %lf\n",A.x,A.y,A.z);
	       printf("%lf %lf %lf\n",B.x,B.y,B.z);
	       printf("%lf %lf %lf\n",C.x,C.y,C.z);
	       
	       printf("face: %d dist: %lf\n",repface[cellule][j],dist);*/
// 	     if(dist<dmin)
// 	       dmin=dist;
	     
// 	     j++;
// 	   }
// 	 }
//        }
//      }
//    }
//    k++;
//  }
 /*printf("dmin: %lf\n",dmin);*/
// return dmin;

// }

/****************************************************************************/
/* fonction qui repertorie pour chaque face les cellules avec lesquelles    */
/*     elle a une intersection                                              */
/****************************************************************************/
// cellules* liste(int grid,model *raw_model)
// {
// cellules *cell;
// int h,i,j,k,m,n,o,cellule,state=0;
// sample *sample1;
// vertex A,B,C,bbox0,bbox1;

// bbox0=raw_model->bBox[0];
// bbox1=raw_model->bBox[1];


// raw_model->face_normals=(vertex*)malloc(raw_model->num_faces*sizeof(vertex));
// cell=(cellules *)malloc((raw_model->num_faces)*sizeof(cellules));

//  for(i=0;i<raw_model->num_faces;i++){
//    h=0;
//    cell[i].cube=(int *)malloc(sizeof(int));   

//    A=raw_model->vertices[raw_model->faces[i].f0];
//    B=raw_model->vertices[raw_model->faces[i].f1];
//    C=raw_model->vertices[raw_model->faces[i].f2];

//    raw_model->face_normals[i]=ncrossp(A,B,C);
//    sample1=echantillon(A,B,C,0.05);

//    for(j=0;j<sample1->nbsamples;j++){
//      state=0;
//      m=(sample1->sample[j].x-bbox0.x)*grid/(bbox1.x-bbox0.x);
//      n=(sample1->sample[j].y-bbox0.y)*grid/(bbox1.y-bbox0.y);
//      o=(sample1->sample[j].z-bbox0.z)*grid/(bbox1.z-bbox0.z);
     
//      if(m==grid)
//        m=grid-1;
//      if(n==grid)
//        n=grid-1;
//      if(o==grid)
//        o=grid-1;

//      cellule=m+n*grid+o*grid*grid;

//      for(k=0;k<=h;k++){
//        if(cellule==cell[i].cube[k]){
//          state=1;
//          break;
//        }
//      }
//      if(state==0){
//        if(h>0){
//          if((cell[i].cube=(int *)realloc(cell[i].cube,(h+1)*sizeof(int)))==NULL){
//            printf("erreur d'allocation memoire");
//            exit(-1);
//          }
//        }
//        cell[i].cube[h]=cellule;
//        h++;
//      } 
//    }
//    if(sample1->sample != NULL)
//      free(sample1->sample);
//    if(sample1 != NULL)
//      free(sample1);
//    cell[i].nbcube=h;

//  }
 /* for(i=0;i<raw_model->num_faces;i++){
   printf("face %d",i);
   for(j=0;j<cell[i].nbcube;j++){
     printf(" %d",cell[i].cube[j]);
   }
   printf("\n");
   }*/
 
// return cell;
// }


/*****************************************************************************/
/* fonction qui repertorie pour chaque cellule la liste des faces avec       */
/*      lesquelles elle a une intersection                                   */
/*****************************************************************************/

// int** cublist(cellules *cell,int grid,model *raw_model)
// {

// int **tab,i,j,k;
// int *mem;

// mem=(int*)calloc(grid*grid*grid,sizeof(int));
// tab=(int **)malloc(grid*grid*grid*sizeof(int*));

//  for(j=0;j<raw_model->num_faces;j++){
//    for(k=0;k<cell[j].nbcube;k++){
//      i=cell[j].cube[k];
//      if(mem[i]==0)
//        tab[i]=NULL;
//      tab[i]=(int *)realloc(tab[i],(mem[i]+1)*sizeof(int));
//      tab[i][mem[i]]=j;
//      mem[i]++;
//    }
//  }

//  for(i=0;i<grid*grid*grid;i++){
//    if(mem[i]==0)
//      tab[i]=NULL;
//    tab[i]=(int *)realloc(tab[i],(mem[i]+1)*sizeof(int));
//    tab[i][mem[i]]=-1;
//  }

 /* for(i=0;i<grid*grid*grid;i++){
   j=0;
   printf("cell %d ",i);
   while(tab[i][j]!=-1){
     printf("%d ",tab[i][j]);
     j++;
   }
   printf("\n");
   }*/

// return(tab);
// }



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

  switch(i){
  case 0:*r=1; *g=t; *b=p;break;
  case 1:*r=q; *g=1; *b=p;break;
  case 2:*r=p; *g=1; *b=t;break;
  case 3:*r=p; *g=q; *b=1;break;
  case 4:*r=t; *g=p; *b=1;break;
  case 5:*r=1; *g=p; *b=q;break;
    
  }

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

/* creation d'une box ou on met le glwidget */

class MyWidget : public QWidget
{
public:
  MyWidget( model *raw_model1,model *raw_model2,QWidget *parent=0, const char *name=0 );

};

MyWidget::MyWidget( model *raw_model1,model *raw_model2,QWidget *parent, const char *name ) : QWidget(parent,name)
{
  setMinimumSize( 1000, 500 );
  setMaximumSize( 1000, 500 );
  
  RawWidget *w = new RawWidget(raw_model1,this,"w");  
  RawWidget *y = new RawWidget(raw_model2,this,"y");

  QSlider* z = new QSlider ( 0, 360, 60, 0, QSlider::Vertical, this, "ysl" );
  z->setTickmarks( QSlider::Left );

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

  for(hue=240.0;hue>0.0;hue=hue-240.0/16.0){
    HSVtoRGB(&r,&g,&b,hue);
    colormap[i][0]=r;
    colormap[i][1]=g;
    colormap[i][2]=b;
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
    raw_model1->error[i]=(dcourant-superdmin)/(superdmax-superdmin)*15;
  }

  QApplication::setColorSpec( QApplication::CustomColor );
  QApplication a( argc, argv );  
  
  MyWidget w(raw_model1,raw_model2);
  a.setMainWidget( &w );
  w.show();
  return a.exec();
}
