/* $Id: ScreenWidget.cpp,v 1.38 2002/02/27 12:09:30 aspert Exp $ */
#include <ScreenWidget.h>

#include <qhbox.h>
#include <qlayout.h>
#include <qaction.h>
#include <qapplication.h>
#include <qmessagebox.h>
#include <qmenubar.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qhbuttongroup.h>
#include <qvbuttongroup.h>
#include <qstring.h>
#include <RawWidget.h>
#include <ColorMapWidget.h>
#include <mesh.h>

ScreenWidget::ScreenWidget(struct model_error *model1,
                           struct model_error *model2,
                           int do_texture,
                           QWidget *parent, 
                           const char *name ):QWidget(parent,name) {
  QAction *fileQuitAction;
  QPushButton *syncBut, *lineSwitch1, *lineSwitch2;
  QMenuBar *mainBar;
  QPopupMenu *fileMenu, *infoMenu, *helpMenu;
  QHBox *frameModel1, *frameModel2;
  QGridLayout *bigGrid, *smallGrid;
  RawWidget *glModel1, *glModel2;
  ColorMapWidget *errorColorBar;
  QPushButton *quitBut;
  QRadioButton *verrBut, *fmerrBut, *serrBut;
  QRadioButton *linBut, *logBut;
  QButtonGroup *dispInfoGrp=NULL, *histoGrp=NULL;
  QString tmp;
  const float p = 0.95f; // max proportion of screen to use
  int max_ds; // maximum downsampling value
  int i;

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
  helpMenu->insertItem("&About", this, SLOT(aboutMesh()));

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
  connect(glModel1, SIGNAL(transferValue(double,double*)), 
	  glModel2, SLOT(transfer(double,double*)));
  connect(glModel2, SIGNAL(transferValue(double,double*)), 
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
  dispInfoGrp = new QHButtonGroup("Displayed information (left model)",this);
  dispInfoGrp->layout()->setMargin(3);
  verrBut = new QRadioButton("Vertex error", dispInfoGrp);
  verrBut->setChecked(TRUE);
  fmerrBut = new QRadioButton("Face mean error", dispInfoGrp);
  serrBut = new QRadioButton("Sample error", dispInfoGrp);
  dispInfoGrp->insert(verrBut, RawWidget::VERTEX_ERROR);
  dispInfoGrp->insert(fmerrBut, RawWidget::MEAN_FACE_ERROR);
  dispInfoGrp->insert(serrBut, RawWidget::SAMPLE_ERROR);
  if (!do_texture) 
    serrBut->setDisabled(TRUE);
  connect(dispInfoGrp, SIGNAL(clicked(int)), 
          glModel1, SLOT(setErrorMode(int)));
  connect(dispInfoGrp, SIGNAL(clicked(int)), this, SLOT(disableSlider(int)));

  // Build downsampling control
  for (i=0, max_ds=1; i<model1->mesh->num_faces; i++) {
    if (model1->fe[i].sample_freq > max_ds) max_ds = model1->fe[i].sample_freq;
  }
  // This is needed s.t. we can add children widget to the GroupBox
  qgbSlider = new 
    QHGroupBox(tmp.sprintf("Subsampling factor of the error = %d", max_ds), 
               this);

  qslidDispSampDensity = new QSlider(1, max_ds, 1, max_ds, 
                                     QSlider::Horizontal, qgbSlider);
  qslidDispSampDensity->setTickInterval((max_ds-1)/5);
  qslidDispSampDensity->setTickmarks(QSlider::Both);
  qslidDispSampDensity->setTracking(FALSE);
  glModel1->setVEDownSampling(max_ds); // Initialization

  qspSampDensity = new QSpinBox(1, max_ds, 1, qgbSlider);
  qspSampDensity->setValue(max_ds);


  // Connect the slider and spinbox to a 'phony' slot s.t. we avoid
  // loops between valueChanged signals and setValue slots
  connect(qspSampDensity, SIGNAL(valueChanged(int)), this, 
          SLOT(trapChanges(int)));
  connect(qslidDispSampDensity, SIGNAL(valueChanged(int)), this, 
          SLOT(trapChanges(int)));
  // The dsValChange signal is emitted once when there has been a real
  // change 
  connect(this, SIGNAL(dsValChange(int)), 
          glModel1, SLOT(setVEDownSampling(int)));

  // Build scale selection buttons
  histoGrp = new QVButtonGroup("X scale",this);
  linBut = new QRadioButton("Linear", histoGrp);
  linBut->setChecked(TRUE);
  logBut = new QRadioButton("Log", histoGrp);
  histoGrp->insert(linBut, ColorMapWidget::LIN_SCALE);
  histoGrp->insert(logBut, ColorMapWidget::LOG_SCALE);
  connect(histoGrp, SIGNAL(clicked(int)), 
          errorColorBar, SLOT(doHistogram(int)));

  // Build the topmost grid layout
  bigGrid = new QGridLayout (this, 3, 7, 5, -1);
  bigGrid->setMenuBar(mainBar);
  bigGrid->addWidget(errorColorBar, 0, 0);
  bigGrid->addMultiCellWidget(frameModel1, 0, 0, 1, 3);
  bigGrid->addMultiCellWidget(frameModel2, 0, 0, 4, 6);
  bigGrid->addWidget(lineSwitch1, 1, 2, Qt::AlignCenter);
  bigGrid->addWidget(lineSwitch2, 1, 5, Qt::AlignCenter);
  bigGrid->addMultiCellWidget(syncBut, 1, 1, 3, 4, Qt::AlignCenter);
  bigGrid->addMultiCellWidget(histoGrp, 1, 2, 0, 0, Qt::AlignCenter);

  // sub layout for dispInfoGrp and Quit button -> avoid resize problems
  smallGrid = new QGridLayout(1, 4, 3);
  smallGrid->addWidget(dispInfoGrp, 0, 0, Qt::AlignLeft);
  smallGrid->addMultiCellWidget(qgbSlider, 0, 0, 1, 2);
  smallGrid->addWidget(quitBut, 0, 3, Qt::AlignCenter);
  bigGrid->addMultiCellLayout(smallGrid, 2, 2, 1, 6);

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

void ScreenWidget::changeGroupBoxTitle(int n) 
{
  QString tmp;

  qgbSlider->setTitle(tmp.sprintf("Subsampling factor of the error = %d", n));
}

void ScreenWidget::disableSlider(int errMode) 
{
  switch (errMode) {
  case (RawWidget::VERTEX_ERROR):
    qgbSlider->setDisabled(FALSE);
    break;
  case (RawWidget::MEAN_FACE_ERROR): 
  case (RawWidget::SAMPLE_ERROR):
    qgbSlider->setDisabled(TRUE);
    break;
  default: /* should never get here */
    return;
  }
}

void ScreenWidget::trapChanges(int n) 
{
  int slv = qslidDispSampDensity->value(); // value of the slider
  int spv = qspSampDensity->value(); // value of the spinbox
  bool hasChanged = FALSE;

  if (slv == n && spv == n)
    return;

  if (slv != n) {
    qslidDispSampDensity->setValue(n);
    hasChanged = TRUE;
  }

  if (spv != n) {
    qspSampDensity->setValue(n);
    hasChanged = TRUE;
  }
  
  if (hasChanged) {
    changeGroupBoxTitle(n);
    emit dsValChange(n);  
  }
}

void ScreenWidget::aboutMesh()
{
  QString msg;

  msg.sprintf("Mesh v %s\n"
              "Copyright (C) %s\n"
              "Authors: Nicolas Aspert, Diego Santa Cruz, Davy Jacquet\n",
              version, copyright);
  QMessageBox::about(this, "Mesh", msg);
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


