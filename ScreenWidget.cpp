/* $Id: ScreenWidget.cpp,v 1.13 2001/08/10 10:04:57 dsanta Exp $ */
#include <ScreenWidget.h>

ScreenWidget::ScreenWidget(model *raw_model1, model *raw_model2, 
			   double dmoymin, double dmoymax, QWidget *parent, 
			   const char *name ):QWidget(parent,name) {
  QAction *fileQuitAction;
  QPushButton *syncBut;
  QMenuBar *mainBar;
  QPopupMenu *fileMenu, *helpMenu;
  QFrame *frameModel1, *frameModel2;
  QHBoxLayout *hLay1, *hLay2;
  QGridLayout *bigGrid;
  RawWidget *glModel1, *glModel2;
  ColorMapWidget *colorBar;

  setMinimumSize( 1070, 540 );
  setMaximumSize( 1070, 540 );
  

  fileQuitAction = new QAction( "Quit", "Quit", CTRL+Key_Q, this, "quit" );
  connect(fileQuitAction, SIGNAL(activated()) , 
	  qApp, SLOT(closeAllWindows()));


  // Create the 'File' menu
  mainBar = new QMenuBar(this);
  fileMenu = new QPopupMenu(this);
  mainBar->insertItem("&File",fileMenu);
  fileMenu->insertSeparator();
  fileQuitAction->addTo(fileMenu);

  //Create the 'Help' menu
  helpMenu = new QPopupMenu(this);
  mainBar->insertItem("&Help", helpMenu);
  helpMenu->insertSeparator();
  helpMenu->insertItem("&Key utilities", this, SLOT(aboutKeys()),CTRL+Key_H);
  helpMenu->insertItem("&Bug", this, SLOT(aboutBugs()));

  // --------------
  // Create the GUI
  // --------------

  // Create frames to put around the OpenGL widgets
  frameModel1 = new QFrame(this, "frameModel1");
  frameModel1->setFrameStyle(QFrame::Sunken | QFrame::Panel);
  frameModel1->setLineWidth(2);

  frameModel2 = new QFrame(this, "frameModel2");
  frameModel2->setFrameStyle(QFrame::Sunken | QFrame::Panel);
  frameModel2->setLineWidth(2);


  // Create the colorbar and the 2 GL windows.
  glModel1 = new RawWidget(raw_model1, RW_COLOR, frameModel1, "glModel1");
  glModel1->setFocusPolicy(StrongFocus);
  glModel2 = new RawWidget(raw_model2, RW_LIGHT_TOGGLE, 
			   frameModel2,"glModel2");
  glModel2->setFocusPolicy(StrongFocus);
  colorBar = new ColorMapWidget(dmoymin, dmoymax, this, "colorBar");

  // Put the 1st GL widget inside the frame
  hLay1 = new QHBoxLayout(frameModel1, 2, 2, "hLay1");
  hLay1->addWidget(glModel1, 1);
  // Put the 2nd GL widget inside the frame
  hLay2 = new QHBoxLayout(frameModel2, 2, 2, "hLay2");
  hLay2->addWidget(glModel2, 1);

  // This is to synchronize the viewpoints of the two models
  // We need to pass the viewing matrix from one RawWidget
  // to another
  connect(glModel1, SIGNAL(transfervalue(double,double*)), 
	  glModel2, SLOT(transfer(double,double*)));
  connect(glModel1, SIGNAL(transfervalue(double,double*)), 
	  glModel1, SLOT(transfer(double,double*)));


  // Build two buttons
  syncBut = new QPushButton("Synchronize viewpoints", this);
  syncBut->setMinimumSize(40, 30);
  syncBut->setToggleButton(TRUE);
  
  quitBut = new QPushButton("Quit", this);
  quitBut->setMinimumSize(20, 30);
  
  connect(syncBut, SIGNAL(toggled(bool)), 
	  glModel1, SLOT(switchSync(bool))); 
  
  

  // Build the topmost grid layout
  bigGrid = new QGridLayout (this, 4, 3, 5);
  bigGrid->addWidget(colorBar, 1, 0);
  bigGrid->addWidget(frameModel1, 1, 1);
  bigGrid->addWidget(frameModel2, 1, 2);
  bigGrid->addWidget(syncBut, 2, 1, Qt::AlignCenter);
  bigGrid->addWidget(quitBut, 3, 1, Qt::AlignCenter);
  

}

void ScreenWidget::aboutKeys()
{
    QMessageBox::about( this, "Key bindings",
			"F1: Toggle Wireframe/Fill\n"
			"F2: Toggle Light/No light (2nd model only)\n"
			"F3: Toggle viewpoint synchronization\n"
			"F4: Invert normals (if applicable)\n"
                        "F5: Force computed (vs. loaded) normals");
}

void ScreenWidget::aboutBugs()
{
    QMessageBox::about( this, "Bug",
			"If you found a bug, please send an e-mail to :\n"
			"Nicolas.Aspert@epfl.ch or\n"
			"Diego.SantaCruz@epfl.ch");
}
