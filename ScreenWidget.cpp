#include <ScreenWidget.h>

/***************************************************************************/
/*        definition de la classe ScreenWidget                             */
/***************************************************************************/



ScreenWidget::ScreenWidget( model *raw_model1,model *raw_model2,QWidget *parent, const char *name ) : QWidget(parent,name)
{
  setMinimumSize( 1050, 620 );
//   setMaximumSize( 1000, 500 );
  
//   file = new QPopupMenu();
//   file->setCheckable( TRUE );
//   file->insertItem( "Exit",  this, SLOT(quit()), CTRL+Key_Q );

  // Create a menu bar
//   QMenuBar *m = new QMenuBar( this );
//   m->setSeparator( QMenuBar::InWindowsStyle );
//   m->insertItem("&File", file );

  // Create the GUI

  QPushButton *h1 = new QPushButton( "line/fill",this);
  h1->setToggleButton(TRUE);
  QPushButton *f1 = new QPushButton( "synchro/desynchro",this);  
  f1->setToggleButton(TRUE);
  QPushButton *h2 = new QPushButton( "line/fill",this);
  h2->setToggleButton(TRUE);
  QPushButton *f2 = new QPushButton( "synchro/desynchro",this);
  f2->setToggleButton(TRUE);
  RawWidget *w = new RawWidget(raw_model1,this,"w");
  RawWidget *y = new RawWidget(raw_model2,this,"y");
  ColorMapWidget *z = new ColorMapWidget(this,"z");

  connect(h1, SIGNAL(stateChanged(int)), w, SLOT(setLine(int)) );
  connect(f1, SIGNAL(clicked()), w, SLOT(aslot()) );
  connect(h2, SIGNAL(stateChanged(int)), y, SLOT(setLine(int)) );
  connect(f2, SIGNAL(clicked()), y, SLOT(aslot()) );

  connect(w, SIGNAL(transfervalue(double,double*)), y, SLOT(transfer(double,double*)) );
  connect(y, SIGNAL(transfervalue(double,double*)), w, SLOT(transfer(double,double*)) );

  QGridLayout *bigrid = new QGridLayout (this,2, 3, 5);
  bigrid->addWidget(z,1,0);
  bigrid->addWidget(w,1,1);
  bigrid->addWidget(y,1,2);

  QHBoxLayout *smallgrid1 = new QHBoxLayout();
  smallgrid1->addWidget(h1,0,0);
  smallgrid1->addWidget(f1,0,1);
  smallgrid1->addSpacing(250);
  bigrid->addLayout(smallgrid1,0,1);

  QHBoxLayout *smallgrid2 = new QHBoxLayout();
  smallgrid2->addWidget(h2,0,0);
  smallgrid2->addWidget(f2,0,1);
  smallgrid2->addSpacing(250);  
  bigrid->addLayout(smallgrid2,0,2);

}



