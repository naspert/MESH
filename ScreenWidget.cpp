/* $Id: ScreenWidget.cpp,v 1.31 2002/02/22 12:54:29 aspert Exp $ */
#include <ScreenWidget.h>

#include <qhbox.h>
#include <qhbuttongroup.h>
#include <qvbuttongroup.h>
#include <qlabel.h>
#include <RawWidget.h>
#include <ColorMapWidget.h>


ScreenWidget::ScreenWidget(struct model_error *model1,
                           struct model_error *model2,
                           QWidget *parent, 
                           const char *name ):QWidget(parent,name) {
  QAction *fileQuitAction;
  QPushButton *syncBut, *lineSwitch1, *lineSwitch2;
  QMenuBar *mainBar;
  QPopupMenu *fileMenu, *infoMenu, *helpMenu;
  QHBox *frameModel1, *frameModel2;
  QGridLayout *bigGrid;
  RawWidget *glModel1, *glModel2;
  ColorMapWidget *errorColorBar;
  QPushButton *quitBut;
  QRadioButton *verrBut, *fmerrBut, *serrBut;
  QRadioButton *linBut, *logBut;
  QButtonGroup *radGrp=NULL, *histoGrp=NULL;
  const float p = 0.95; // max proportion of screen to use

  setCaption("Mesh: visualization");

  fileQuitAction = new QAction( "Quit", "Quit", CTRL+Key_Q, this, "quit" );
  connect(fileQuitAction, SIGNAL(activated()) , 
	  qApp, SLOT(closeAllWindows()));


  // Create the 'File' menu
  mainBar = new QMenuBar(this);
  fileMenu = new QPopupMenu(this);
  mainBar->insertItem("&File",fileMenu);
  fileQuitAction->addTo(fileMenu);

  // Create the 'Info' menu
  infoMenu = new QPopupMenu(this);
  // save the adresses of these structures for later
  locMod1 = model1;
  locMod2 = model2;
  mainBar->insertItem("&Info", infoMenu);
  infoMenu->insertItem("Left model information", this, 
                       SLOT(infoLeftModel()));
  infoMenu->insertItem("Right model information", this, 
                       SLOT(infoRightModel()));

  //Create the 'Help' menu
  helpMenu = new QPopupMenu(this);
  mainBar->insertItem("&Help", helpMenu);
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

  glModel1 = new RawWidget(model1, RW_ERROR_ONLY, 
                           frameModel1, "glModel1");

  glModel1->setFocusPolicy(StrongFocus);
  glModel2 = new RawWidget(model2, RW_LIGHT_TOGGLE, frameModel2, "glModel2");
  glModel2->setFocusPolicy(StrongFocus);
  errorColorBar = new ColorMapWidget(model1, this, "errorColorBar");

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

  // Build error mode selection buttons
  radGrp = new QHButtonGroup("Displayed information (left model)",this);
  radGrp->layout()->setMargin(3);
  verrBut = new QRadioButton("Vertex error", radGrp);
  verrBut->setChecked(TRUE);
  fmerrBut = new QRadioButton("Face mean error", radGrp);
  serrBut = new QRadioButton("Sample error", radGrp);
  radGrp->insert(verrBut, RawWidget::VERTEX_ERROR);
  radGrp->insert(fmerrBut, RawWidget::MEAN_FACE_ERROR);
  radGrp->insert(serrBut, RawWidget::SAMPLE_ERROR);
  connect(radGrp, SIGNAL(clicked(int)), glModel1, SLOT(setErrorMode(int)));

  // Build scale selection buttons
  histoGrp = new QVButtonGroup("Histogram scale",this);
  linBut = new QRadioButton("Linear", histoGrp);
  linBut->setChecked(TRUE);
  logBut = new QRadioButton("Logarithmic", histoGrp);
  histoGrp->insert(linBut, ColorMapWidget::LIN_SCALE);
  histoGrp->insert(logBut, ColorMapWidget::LOG_SCALE);
  connect(histoGrp, SIGNAL(clicked(int)), 
          errorColorBar, SLOT(doHistogram(int)));

  // Build the topmost grid layout
  bigGrid = new QGridLayout (this, 3, 7, 5);
  bigGrid->setMenuBar(mainBar);
  bigGrid->addWidget(errorColorBar, 0, 0);
  bigGrid->addMultiCellWidget(frameModel1, 0, 0, 1, 3);
  bigGrid->addMultiCellWidget(frameModel2, 0, 0, 4, 6);
  bigGrid->addWidget(lineSwitch1, 1, 2, Qt::AlignCenter);
  bigGrid->addWidget(lineSwitch2, 1, 5, Qt::AlignCenter);
  bigGrid->addMultiCellWidget(syncBut, 1, 1, 3, 4, Qt::AlignCenter);
  bigGrid->addMultiCellWidget(radGrp, 2, 2, 1, 4, Qt::AlignLeft);
  bigGrid->addMultiCellWidget(histoGrp, 1, 2, 0, 0, Qt::AlignCenter);
  bigGrid->addWidget(quitBut, 2, 5, Qt::AlignCenter);

  // Now set a sensible default widget size
  QSize prefSize = layout()->sizeHint();
  QSize screenSize = QApplication::desktop()->size();


  if (prefSize.width() > p*screenSize.width()) {
    prefSize.setWidth((int)(p*screenSize.width()));
  }
  if (prefSize.height() > p*screenSize.height()) {
    prefSize.setHeight((int)(p*screenSize.height()));
  }
  resize(prefSize.width(),prefSize.height());
}

void ScreenWidget::infoLeftModel()
{
  infoModel(locMod1, LEFT_MODEL);
}

void ScreenWidget::infoRightModel()
{
  infoModel(locMod2, RIGHT_MODEL);
}

void ScreenWidget::infoModel(struct model_error *model, int id) 
{
  QString tmp, fullText;
  fullText.sprintf("%d vertices\n%d triangles\n", model->mesh->num_vert, 
                   model->mesh->num_faces);

  // Orientable model ?
  if (model->info->orientable)
    fullText += "Orientable model\n";
  else 
    fullText += "Non-orientable model\n";

  // Manifold model ?
  if (model->info->manifold)
    fullText += "Manifold model\n";
  else
    fullText += "Non-manifold model\n"; 

  // Closed model ?
  if (model->info->closed)
    fullText += "Closed model\n";
  else
    fullText += "Non-closed model\n"; 

  fullText += tmp.sprintf("%d connected component(s)\n", 
                          model->info->n_disjoint_parts);

  switch(id) {
  case LEFT_MODEL:
    QMessageBox::information(this, "Left Model Information", fullText);
    break;
  case RIGHT_MODEL:
    QMessageBox::information(this, "Right Model Information", fullText);
    break;
  default:
    QMessageBox::warning(this, "Error", "Invalid paremeter !!\n");
    break;
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


