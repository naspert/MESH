/* $Id: viewer.cpp,v 1.3 2001/05/03 08:00:22 jacquet Exp $ */
#include <qpainter.h>
#include <qpushbutton.h>
#include <qfont.h>
#include <qapplication.h>

#include <qhbox.h>

#include <qlayout.h>


#include <RawWidget.h>

#include <compute_error.h>




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
}

/* creation d'un Widget Colormap */
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

  for(int i=0; i<16;i++){
    p.setBrush(QColor(255*colormap[15-i][0],255*colormap[15-i][1],255*colormap[15-i][2]));
    p.setPen(QColor(floor(255*colormap[15-i][0]),floor(255*colormap[15-i][1]),floor(255*colormap[15-i][2])));
    p.drawRect(0,i*32,20,32);
  }
  p.end();
}

/* creation d'un widget pour la fenetre graphique*/

class ScreenWidget : public QWidget
{
public:
  ScreenWidget( model *raw_model1,model *raw_model2,QWidget *parent=0, const char *name=0 );

};

ScreenWidget::ScreenWidget( model *raw_model1,model *raw_model2,QWidget *parent, const char *name ) : QWidget(parent,name)
{
  setMinimumSize( 1200, 600 );
//   setMaximumSize( 1000, 500 );
  

  
  QPushButton *h1 = new QPushButton( "line/fill",this);
  QPushButton *f1 = new QPushButton( "fill",this);  
  QPushButton *h2 = new QPushButton( "line/fill",this);
  QPushButton *f2 = new QPushButton( "Fill",this);  
  RawWidget *w = new RawWidget(raw_model1,this,"w");
  RawWidget *y = new RawWidget(raw_model2,this,"y");

  connect(h1, SIGNAL(clicked()), w, SLOT(setLine()) );
  connect(f1, SIGNAL(clicked()), w, SLOT(aslot()) );
  connect(h2, SIGNAL(clicked()), y, SLOT(setLine()) );
  connect(w, SIGNAL(transfervalue(double,double*)), y, SLOT(transfer(double,double*)) );


  QHBoxLayout *hlayout2 = new QHBoxLayout(20,"hlayout");
  hlayout2->addWidget(h1);
  hlayout2->addWidget(f1);

  QHBoxLayout *hlayout3 = new QHBoxLayout(20,"hlayout");
  hlayout3->addWidget(h2);
  hlayout3->addWidget(f2);
 
  
  QVBoxLayout *vlayout = new QVBoxLayout( 20, "vlayout");
  vlayout->addLayout(hlayout2);
  vlayout->addWidget(w,1);

  QVBoxLayout *vlayout2 = new QVBoxLayout( 20, "vlayout2");
  vlayout2->addLayout(hlayout3);
  vlayout2->addWidget(y,1);
  
  ColorMapWidget *z = new ColorMapWidget(this,"z");


  QHBoxLayout *hlayout = new QHBoxLayout( this, 20, 20,"hlayout");
  hlayout->addWidget(z,1);
  hlayout->addLayout(vlayout,2);
  hlayout->addLayout(vlayout2,3);


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
  double samplethin,dcourant,dmax=0,superdmax=0,dmin=200,superdmin=200;
  double dmoy,dmoymax=0,dmoymin=200;
  int **repface;
  int **list_face;
  int *nbfaces;
  double *moyfacerror;
  int i,j,h=0;
  sample *sample2;
  double r,g,b,hue;
  FILE *f;

f=fopen("dida","w");

  for(hue=0.0;hue<240.0;hue=hue+240.0/16.0){
    HSVtoRGB(&r,&g,&b,hue);
    colormap[15-i][0]=r;
    colormap[15-i][1]=g;
    colormap[15-i][2]=b;
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

  moyfacerror=(double *)malloc(raw_model1->num_faces*sizeof(double));
  list_face=(int **)malloc(raw_model1->num_vert*sizeof(int *));
  nbfaces=(int *)calloc(raw_model1->num_vert,sizeof(int));

  listoffaces(raw_model1,nbfaces,list_face);
  
  for(i=0;i<raw_model1->num_faces;i++) {
    sample2=echantillon(raw_model1->vertices[raw_model1->faces[i].f0],
			raw_model1->vertices[raw_model1->faces[i].f1],
			raw_model1->vertices[raw_model1->faces[i].f2],
			samplethin);
    dmoy=0;
    for(j=0;j<sample2->nbsamples;j++){
      dcourant=pcd(sample2->sample[j],raw_model2,repface,grid,f);
      dmoy+=dcourant;
      if(dcourant>dmax)
       dmax=dcourant;
     if(dcourant<dmin)
       dmin=dcourant;
     h++;
    }
    
    dmoy/=sample2->nbsamples;
    moyfacerror[i]=dmoy;
    if(dmoy>dmoymax)
      dmoymax=dmoy;
    if(dmoy<dmoymin)
      dmoymin=dmoy;

    printf("face numero %d: dmax= %lf dmoy= %lf\n",i+1,dmax,dmoy);
    if(dmax>superdmax)
      superdmax=dmax;
    dmax=0;
    if(dmin<superdmin)
      superdmin=dmin;
    dmin=200;
    
    free(sample2->sample);
    
    free(sample2);
    
  }
  printf("distance maximale: %lf dist moy max: %lf\n",superdmax,dmoymax);
  printf("distance minimale: %lf dist moy min: %lf\n",superdmin,dmoymin); 
  printf("nbsampleteste: %d\n",h);
//   fprintf(f,"\n\n\n");

  
  raw_model1->error=(int *)malloc(raw_model1->num_vert*sizeof(int));
  raw_model2->error=(int *)calloc(raw_model2->num_vert,sizeof(int));
 

  for(i=0;i<raw_model1->num_vert;i++) {
    dcourant=0;  
    for(j=0;j<nbfaces[i];j++){ 
      dcourant+=(moyfacerror[list_face[i][j]]-dmoymin)/(dmoymax-dmoymin);
    }
    dcourant/=nbfaces[i];
    raw_model1->error[i]=dcourant*15;
    if(raw_model1->error[i]>15)
      raw_model1->error[i]=15;
    //raw_model1->error[i]=(int)floor(dcourant); 
    fprintf(f,"%d ",raw_model1->error[i]); 
    //fprintf(f,"%lf ",dcourant);
  }

  QApplication::setColorSpec( QApplication::CustomColor );
  QApplication a( argc, argv );  
  
  ScreenWidget w(raw_model1,raw_model2);
  a.setMainWidget( &w );
  w.show();
  return a.exec();
}
