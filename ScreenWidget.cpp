#include <ScreenWidget.h>

/***************************************************************************/
/*        definition de la classe ScreenWidget                             */
/***************************************************************************/

ScreenWidget::ScreenWidget( model *raw_model1,model *raw_model2,QWidget *parent, const char *name ) : QWidget(parent,name)
{
  setMinimumSize( 1200, 600 );
//   setMaximumSize( 1000, 500 );
  
  QPushButton *h1 = new QPushButton( "line/fill",this);
  QPushButton *f1 = new QPushButton( "synchro/desynchro",this);  
  QPushButton *h2 = new QPushButton( "line/fill",this);
  QPushButton *f2 = new QPushButton( "synchro/desynchro",this);  
  RawWidget *w = new RawWidget(raw_model1,this,"w");
  RawWidget *y = new RawWidget(raw_model2,this,"y");

  connect(h1, SIGNAL(clicked()), w, SLOT(setLine()) );
  connect(f1, SIGNAL(clicked()), w, SLOT(aslot()) );
  connect(h2, SIGNAL(clicked()), y, SLOT(setLine()) );
  connect(f2, SIGNAL(clicked()), y, SLOT(aslot()) );

  connect(w, SIGNAL(transfervalue(double,double*)), y, SLOT(transfer(double,double*)) );
  connect(y, SIGNAL(transfervalue(double,double*)), w, SLOT(transfer(double,double*)) );


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



