#include <ScreenWidget.h>

/***************************************************************************/
/*        definition de la classe ScreenWidget                             */
/***************************************************************************/



ScreenWidget::ScreenWidget( model *raw_model1,model *raw_model2,QWidget *parent, const char *name ) : QWidget(parent,name)
{
  setMinimumSize( 1050, 640 );

  // create a printer
    

  QAction *fileOpenAction = new QAction( "Open File", QPixmap( fileopen ), "&Open", CTRL+Key_O, this, "open" );
//   connect( fileOpenAction, SIGNAL( activated() ) , this, SLOT( load() ) );
//   QMimeSourceFactory::defaultFactory()->setPixmap( "fileopen", QPixmap( fileopen ) );
  QAction *fileQuitAction = new QAction( "Quit", "&Quit", CTRL+Key_Q, this, "quit" );
  connect( fileQuitAction, SIGNAL( activated() ) , qApp, SLOT( closeAllWindows() ) );
  QAction *filePrintAction = new QAction( "Print File", QPixmap( fileprint ), "&Print", CTRL+Key_P, this, "print" );
//   connect( filePrintAction, SIGNAL( activated() ) , this, SLOT( print() ) );


  // populate a menu with some actions
  QMenuBar *m = new QMenuBar( this );
  QPopupMenu *file = new QPopupMenu(this);
  m->insertItem("&File",file);
  fileOpenAction->addTo( file );
  filePrintAction->addTo( file );
  file->insertSeparator();
  fileQuitAction->addTo( file );

  // Create the GUI
  RawWidget *w = new RawWidget(raw_model1,this,"w");
  w->setFocusPolicy(StrongFocus);
  RawWidget *y = new RawWidget(raw_model2,this,"y");
  y->setFocusPolicy(StrongFocus);
  ColorMapWidget *z = new ColorMapWidget(this,"z");

  QPushButton *quit = new QPushButton ( "quit",this );
  connect(quit, SIGNAL(clicked()), qApp, SLOT(quit()) );

  QPushButton *h1 = new QPushButton( "line/fill",this);
  h1->setToggleButton(TRUE);
  h1->setAccel(CTRL+'L');
  connect(h1, SIGNAL(clicked()), w, SLOT(setLine()) );

//   QPushButton *f1 = new QPushButton( "synchro/desynchro",this);  
//   f1->setToggleButton(TRUE);
//   connect(f1, SIGNAL(clicked()), w, SLOT(aslot()) );
  connect(w, SIGNAL(transfervalue(double,double*)), y, SLOT(transfer(double,double*)) );

  QPushButton *h2 = new QPushButton( "line/fill",this);
  h2->setToggleButton(TRUE);
  connect(h2, SIGNAL(clicked()), y, SLOT(setLine()) );

  QPushButton *l2 = new QPushButton( "light/no light",this);
  l2->setToggleButton(TRUE);
  connect(l2, SIGNAL(clicked()), y, SLOT(setLight()) );

  QGridLayout *bigrid = new QGridLayout (this,2, 3, 5);
  bigrid->addWidget(z,1,0);
  bigrid->addWidget(w,1,1);
  bigrid->addWidget(y,1,2);

  QHBoxLayout *smallgrid1 = new QHBoxLayout();
  smallgrid1->addWidget(quit,0,0);
  smallgrid1->addWidget(h1,0,1);
//   smallgrid1->addWidget(f1,0,2);
//   smallgrid1->addWidget(l1,0,2);  
  smallgrid1->addSpacing(200);
  bigrid->addLayout(smallgrid1,0,1);

  QHBoxLayout *smallgrid2 = new QHBoxLayout();
  smallgrid2->addWidget(h2,0,0);
//   smallgrid2->addWidget(f2,0,1);
  smallgrid2->addWidget(l2,1,0);  
  smallgrid2->addSpacing(250);  
  bigrid->addLayout(smallgrid2,0,2);

}

// void ScreenWidget::keyPressEvent(QKeyEvent *k)
// {

//   if(k->key()==Key_F3){
//     if(w->move_state==0){
//       w->move_state=1;
//       y->move_state=1;
//       y->distance=w->distance;
//       memcpy(y->mvmatrix,w->mvmatrix, 16*sizeof(double));
//     }
//     else {
//       w->move_state=0;
//       y->move_state=0;
//     }
//   }
// }
   



