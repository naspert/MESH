#include <ScreenWidget.h>

/***************************************************************************/
/*        definition de la classe ScreenWidget                             */
/***************************************************************************/


ScreenWidget::ScreenWidget( model *raw_model1,model *raw_model2,double dmoymin, double dmoymax,QWidget *parent, const char *name ) : QWidget(parent,name)
{
  setMinimumSize( 1070, 540 );
  setMaximumSize( 1070, 540 );


  QAction *fileQuitAction = new QAction( "Quit", "Quit", CTRL+Key_Q, this, "quit" );
  connect( fileQuitAction, SIGNAL( activated() ) , qApp, SLOT( closeAllWindows() ) );


  // populate a menu with some actions  
  QMenuBar *m = new QMenuBar( this );
  QPopupMenu *file = new QPopupMenu(this);
  m->insertItem("&File",file);
  file->insertSeparator();
  fileQuitAction->addTo( file );

  //add a help menu
  QPopupMenu *help = new QPopupMenu( this );
  m->insertItem( "&Help", help );
  help->insertSeparator();
  help->insertItem( "&Key utilities", this, SLOT(about()),CTRL+Key_H);
  help->insertItem( "&Bug", this, SLOT(about2()));


  // Create the GUI
  // Create nice frames to put around the OpenGL widgets
  QFrame* f1 = new QFrame( this, "frame1" );
  f1->setFrameStyle( QFrame::Sunken | QFrame::Panel );
  f1->setLineWidth( 2 );
  QFrame* f2 = new QFrame( this, "frame1" );
  f2->setFrameStyle( QFrame::Sunken | QFrame::Panel );
  f2->setLineWidth( 2 );



  RawWidget *w = new RawWidget(raw_model1,f1,"w");
  w->setFocusPolicy(StrongFocus);
  RawWidget *y = new RawWidget(raw_model2,f2,"y");
  y->setFocusPolicy(StrongFocus);
  ColorMapWidget *z = new ColorMapWidget(dmoymin,dmoymax,this,"z");

    // Put the GL widget inside the frame
    QHBoxLayout* flayout1 = new QHBoxLayout( f1, 2, 2, "flayout1");
    flayout1->addWidget( w, 1 );
    // Put the GL widget inside the frame
    QHBoxLayout* flayout2 = new QHBoxLayout( f2, 2, 2, "flayout1");
    flayout2->addWidget( y, 1 );


//   QPushButton *quit = new QPushButton ( "quit",this );
//   connect(quit, SIGNAL(clicked()), qApp, SLOT(quit()) );

//   QPushButton *h1 = new QPushButton( "line/fill",this);
//   h1->setToggleButton(TRUE);
//   h1->setAccel(CTRL+'L');
//   connect(h1, SIGNAL(clicked()), w, SLOT(setLine()) );

//   QPushButton *f1 = new QPushButton( "synchro/desynchro",this);  
//   f1->setToggleButton(TRUE);
//   connect(f1, SIGNAL(clicked()), w, SLOT(aslot()) );
  connect(w, SIGNAL(transfervalue(double,double*)), y, SLOT(transfer(double,double*)) );
  connect(w, SIGNAL(transfervalue(double,double*)), w, SLOT(transfer(double,double*)) );

//   QPushButton *h2 = new QPushButton( "line/fill",this);
//   h2->setToggleButton(TRUE);
//   connect(h2, SIGNAL(clicked()), y, SLOT(setLine()) );

//   QPushButton *l2 = new QPushButton( "light/no light",this);
//   l2->setToggleButton(TRUE);
//   connect(l2, SIGNAL(clicked()), y, SLOT(setLight()) );

  QGridLayout *bigrid = new QGridLayout (this,2, 3, 5);
  bigrid->addWidget(z,1,0);
  bigrid->addWidget(f1,1,1);
  bigrid->addWidget(f2,1,2);

//   QHBoxLayout *smallgrid1 = new QHBoxLayout();
//   smallgrid1->addWidget(quit,0,0);
//   smallgrid1->addWidget(h1,0,1);
//   smallgrid1->addWidget(f1,0,2);
//   smallgrid1->addWidget(l1,0,2);  
//   smallgrid1->addSpacing(300);
//   bigrid->addLayout(smallgrid1,0,1);

//   QHBoxLayout *smallgrid2 = new QHBoxLayout();
//   smallgrid2->addWidget(h2,0,0);
// //   smallgrid2->addWidget(f2,0,1);
//   smallgrid2->addWidget(l2,1,0);  
//   smallgrid2->addSpacing(250);  
//   bigrid->addLayout(smallgrid2,0,2);

}

void ScreenWidget::about()
{
    QMessageBox::about( this, "Key utilities",
			"Key F1: passage en mode LINE/FILL\n"
			"Key F2: passage en mode LIGHT/NO LIGHT\n"
			"Key F3: passage en mode SYNCHRO/DESYNCHRO\n");
}

void ScreenWidget::about2()
{
    QMessageBox::about( this, "Bug",
			"if you note a bug, please join N.ASPERT\n"
			"Nicolas.Aspert@epfl.ch\n"
			"or use Metro");
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
