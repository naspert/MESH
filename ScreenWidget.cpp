/* $Id: ScreenWidget.cpp,v 1.21 2001/10/10 12:57:55 aspert Exp $ */
#include <ScreenWidget.h>



ScreenWidget::ScreenWidget(struct model_error *model1,
                           struct model_error *model2,
			   QWidget *parent, 
			   const char *name ):QWidget(parent,name) {
  QAction *fileQuitAction;
  QPushButton *syncBut, *lineSwitch1, *lineSwitch2;
  QMenuBar *mainBar;
  QPopupMenu *fileMenu, *helpMenu;
  QFrame *frameModel1, *frameModel2;
  QHBoxLayout *hLay1, *hLay2;
  QGridLayout *bigGrid;
  RawWidget *glModel1, *glModel2;
  ColorMapWidget *errorColorBar;
  QPushButton *quitBut;
  QRadioButton *erBut, *k1But, *k2But, *kgBut;
  QButtonGroup *radGrp=NULL;
  bool drawCurv=FALSE;

  setMinimumSize( 1070, 540 );
  setMaximumSize( 1070, 540 );
  

  model_data = model1;

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
  if (model1->k1_error != NULL) 
    drawCurv = TRUE;

  if (drawCurv)
    glModel1 = new RawWidget(model1, RW_ERROR_AND_CURV | RW_COLOR_ERROR, 
			     frameModel1, "glModel1");
  else 
    glModel1 = new RawWidget(model1, RW_ERROR_ONLY | RW_COLOR_ERROR, 
			     frameModel1, "glModel1");

  glModel1->setFocusPolicy(StrongFocus);
  glModel2 = new RawWidget(model2, RW_LIGHT_TOGGLE, frameModel2, "glModel2");
  glModel2->setFocusPolicy(StrongFocus);
  errorColorBar = new ColorMapWidget(model1->min_verror,
				     model1->max_verror, this, 
				     "errorColorBar");




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
  connect(glModel2, SIGNAL(transfervalue(double,double*)), 
	  glModel1, SLOT(transfer(double,double*)));


  // Build synchro and quit buttons
  syncBut = new QPushButton("Synchronize viewpoints", this);
  syncBut->setMinimumSize(40, 30);
  syncBut->setToggleButton(TRUE);

  connect(syncBut, SIGNAL(toggled(bool)), 
	  glModel1, SLOT(switchSync(bool))); 
  connect(syncBut, SIGNAL(toggled(bool)), 
	  glModel2, SLOT(switchSync(bool)));
  connect(glModel1, SIGNAL(toggleSync()),syncBut, SLOT(toggle()));
  connect(glModel2, SIGNAL(toggleSync()),syncBut, SLOT(toggle()));

  quitBut = new QPushButton("Quit", this);
  quitBut->setMinimumSize(20, 30);
  connect(quitBut, SIGNAL(clicked()), this, SLOT(close()));


  // Build the two line/fill toggle buttons
  lineSwitch1 = new QPushButton("Line/Fill", this);
  lineSwitch1->setToggleButton(TRUE);

  connect(lineSwitch1, SIGNAL(toggled(bool)), 
	  glModel1, SLOT(setLine(bool)));
  connect(glModel1, SIGNAL(toggleLine()),lineSwitch1, SLOT(toggle()));

  lineSwitch2 = new QPushButton("Line/Fill", this);
  lineSwitch2->setToggleButton(TRUE);

  connect(lineSwitch2, SIGNAL(toggled(bool)), 
	  glModel2, SLOT(setLine(bool)));
  connect(glModel2, SIGNAL(toggleLine()),lineSwitch2, SLOT(toggle()));

  if (drawCurv) {
    radGrp = new QButtonGroup(1, Qt::Vertical, this);
    erBut = new QRadioButton("Haus. error", radGrp);
    erBut->setChecked(TRUE);
    k1But = new QRadioButton("k1 error", radGrp);
    k2But = new QRadioButton("k2 error", radGrp);
    kgBut = new QRadioButton("kg error", radGrp);
    
  // This is only needed to set the button ids to our RW_* values
    radGrp->insert(erBut, RW_COLOR_ERROR);
    radGrp->insert(k1But, RW_COLOR_K1);
    radGrp->insert(k2But, RW_COLOR_K2);
    radGrp->insert(kgBut, RW_COLOR_KG);

    connect(radGrp, SIGNAL(clicked(int)), 
	    glModel1, SLOT(switchDisplayedInfo(int)));
    connect(radGrp, SIGNAL(clicked(int)), 
	    this, SLOT(updateColorBar(int)));
    connect(this, SIGNAL(actualUpdate(double, double)),
	    errorColorBar, SLOT(rescale(double, double)));
	    
  }

  // Build the topmost grid layout
  if (drawCurv)
    bigGrid = new QGridLayout (this, 6, 3, 5);
  else
    bigGrid = new QGridLayout (this, 5, 3, 5);

  bigGrid->addWidget(errorColorBar, 1, 0);
  bigGrid->addWidget(frameModel1, 1, 1);
  bigGrid->addWidget(frameModel2, 1, 2);
  bigGrid->addWidget(lineSwitch1, 2, 1, Qt::AlignCenter);
  bigGrid->addWidget(lineSwitch2, 2, 2, Qt::AlignCenter);
  if (drawCurv) {
    bigGrid->addWidget(radGrp, 3, 1);
    bigGrid->addWidget(syncBut, 4, 1, Qt::AlignCenter);
    bigGrid->addWidget(quitBut, 5, 1, Qt::AlignCenter);
  } else {
    bigGrid->addWidget(syncBut, 3, 1, Qt::AlignCenter);
    bigGrid->addWidget(quitBut, 4, 1, Qt::AlignCenter);
  }
  

}

void ScreenWidget::aboutKeys()
{
    QMessageBox::about( this, "Key bindings",
			"F1: Toggle Wireframe/Fill\n"
			"F2: Toggle lighting (right model only)\n"
			"F3: Toggle viewpoint synchronization\n"
			"F4: Invert normals (right model only)\n"
                        "F5: Toggle two sided material (right model only)");
}

void ScreenWidget::aboutBugs()
{
    QMessageBox::about( this, "Bug",
			"If you found a bug, please send an e-mail to :\n"
			"Nicolas.Aspert@epfl.ch or\n"
			"Diego.SantaCruz@epfl.ch");
}

void ScreenWidget::quit()
{
  QApplication::exit(0);
}

void ScreenWidget::updateColorBar(int id) {
  switch(id) {
  case RW_COLOR_ERROR:
    emit actualUpdate(model_data->min_verror, model_data->max_verror);
    break;
  case RW_COLOR_K1:
    emit actualUpdate(model_data->min_k1_error, model_data->max_k1_error);
    break;
  case RW_COLOR_K2:
    emit actualUpdate(model_data->min_k2_error, model_data->max_k2_error);
    break;
  case RW_COLOR_KG:
    emit actualUpdate(model_data->min_kg_error, model_data->max_kg_error);
    break;
  default:
    fprintf(stderr, "Invalid id=0x%x in updateColorBar\n", id);
    break;
  }
}
