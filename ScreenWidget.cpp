/* $Id: ScreenWidget.cpp,v 1.25 2001/11/09 10:59:12 dsanta Exp $ */
#include <ScreenWidget.h>

#include <qhbox.h>
#include <qhbuttongroup.h>
#include <RawWidget.h>
#include <ColorMapWidget.h>


ScreenWidget::ScreenWidget(struct model_error *model1,
                           struct model_error *model2,
                           QWidget *parent, 
                           const char *name ):QWidget(parent,name) {
  QAction *fileQuitAction;
  QPushButton *syncBut, *lineSwitch1, *lineSwitch2;
  QMenuBar *mainBar;
  QPopupMenu *fileMenu, *helpMenu;
  QHBox *frameModel1, *frameModel2;
  QGridLayout *bigGrid;
  RawWidget *glModel1, *glModel2;
  ColorMapWidget *errorColorBar;
  QPushButton *quitBut;
  QRadioButton *erBut, *kmBut, *kgBut;
  QButtonGroup *radGrp=NULL;
  bool drawCurv=FALSE;

  setCaption("Mesh: visualization");
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
  frameModel1 = new QHBox(this, "frameModel1");
  frameModel1->setFrameStyle(QFrame::Sunken | QFrame::Panel);
  frameModel1->setLineWidth(2);

  frameModel2 = new QHBox(this, "frameModel2");
  frameModel2->setFrameStyle(QFrame::Sunken | QFrame::Panel);
  frameModel2->setLineWidth(2);


  // Create the colorbar and the 2 GL windows.
  if (model1->kg_error != NULL) 
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

  // This is to synchronize the viewpoints of the two models
  // We need to pass the viewing matrix from one RawWidget
  // to another
  connect(glModel1, SIGNAL(transfervalue(double,double*)), 
	  glModel2, SLOT(transfer(double,double*)));
  connect(glModel2, SIGNAL(transfervalue(double,double*)), 
	  glModel1, SLOT(transfer(double,double*)));


  // Build synchro and quit buttons
  syncBut = new QPushButton("Synchronize viewpoints", this);
  syncBut->setToggleButton(TRUE);

  connect(syncBut, SIGNAL(toggled(bool)), 
	  glModel1, SLOT(switchSync(bool))); 
  connect(syncBut, SIGNAL(toggled(bool)), 
	  glModel2, SLOT(switchSync(bool)));
  connect(glModel1, SIGNAL(toggleSync()),syncBut, SLOT(toggle()));
  connect(glModel2, SIGNAL(toggleSync()),syncBut, SLOT(toggle()));

  quitBut = new QPushButton("Quit", this);
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
    radGrp = new QHButtonGroup(this);
    radGrp->layout()->setMargin(3);
    erBut = new QRadioButton("Haus. error", radGrp);
    erBut->setChecked(TRUE);
    kmBut = new QRadioButton("km error", radGrp);
    kgBut = new QRadioButton("kg error", radGrp);
    
  // This is only needed to set the button ids to our RW_* values
    radGrp->insert(erBut, RW_COLOR_ERROR);
    radGrp->insert(kmBut, RW_COLOR_KM);
    radGrp->insert(kgBut, RW_COLOR_KG);

    connect(radGrp, SIGNAL(clicked(int)), 
	    glModel1, SLOT(switchDisplayedInfo(int)));
    connect(radGrp, SIGNAL(clicked(int)), 
	    this, SLOT(updateColorBar(int)));
    connect(this, SIGNAL(actualUpdate(double, double)),
	    errorColorBar, SLOT(rescale(double, double)));
	    
  }

  // Build the topmost grid layout
  bigGrid = new QGridLayout (this, 3, 7, 5);
  bigGrid->setMenuBar(mainBar);
  bigGrid->addWidget(errorColorBar, 0, 0);
  bigGrid->addMultiCellWidget(frameModel1, 0, 0, 1, 3);
  bigGrid->addMultiCellWidget(frameModel2, 0, 0, 4, 6);
  bigGrid->addWidget(lineSwitch1, 1, 2, Qt::AlignCenter);
  bigGrid->addWidget(lineSwitch2, 1, 5, Qt::AlignCenter);
  bigGrid->addMultiCellWidget(syncBut, 1, 1, 3, 4, Qt::AlignCenter);
  if (drawCurv) {
    bigGrid->addWidget(radGrp, 2, 2, Qt::AlignCenter);
  }
  bigGrid->addMultiCellWidget(quitBut, 2, 2, 3, 4, Qt::AlignCenter);

  // Now set a sensible default widget size
  QSize prefSize = layout()->sizeHint();
  QSize screenSize = QApplication::desktop()->size();
  double p = 0.95; // max proportion of screen to use

  if (prefSize.width() > p*screenSize.width()) {
    prefSize.setWidth((int)(p*screenSize.width()));
  }
  if (prefSize.height() > p*screenSize.height()) {
    prefSize.setHeight((int)(p*screenSize.height()));
  }
  resize(prefSize.width(),prefSize.height());
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
  case RW_COLOR_KM:
    emit actualUpdate(model_data->min_km_error, model_data->max_km_error);
    break;
  case RW_COLOR_KG:
    emit actualUpdate(model_data->min_kg_error, model_data->max_kg_error);
    break;
  default:
    fprintf(stderr, "Invalid id=0x%x in updateColorBar\n", id);
    break;
  }
}
