/* $Id: InitWidget.cpp,v 1.4 2001/09/20 16:15:17 dsanta Exp $ */
#include <InitWidget.h>

InitWidget::InitWidget(QWidget *parent, const char *name):
  QWidget( parent, name) {

  QLabel *qlabMesh1, *qlabMesh2, *qlabSplStep;
  QPushButton *B1, *B2, *OK;
  QListBox *qlboxSplStep;
  QGridLayout *bigGrid;
  QHBoxLayout *smallGrid1, *smallGrid2, *smallGrid3, *smallGrid4;

  /* Initialize */
  isValid = 0;

  /* First mesh */
  qledMesh1 = new QLineEdit("mesh1.raw", this);
  qlabMesh1 = new QLabel(qledMesh1, "First Mesh", this);
  B1 = new QPushButton("Browse...",this);
  connect(B1, SIGNAL(clicked()), this, SLOT(loadMesh1()));
  /* Second mesh */
  qledMesh2 = new QLineEdit("mesh2.raw", this);
  qlabMesh2 = new QLabel(qledMesh2, "Second Mesh", this);
  B2 = new QPushButton("Browse...", this);
  connect(B2, SIGNAL(clicked()), this, SLOT(loadMesh2()));
  
  /* Sampling step */
  qledSplStep = new QLineEdit("sampling step", this);
  qlboxSplStep = new QListBox(this);
  qlabSplStep = new QLabel(qlboxSplStep, "Sampling step (%)", this);
  qlboxSplStep->insertItem("2");
  qlboxSplStep->insertItem("1");
  qlboxSplStep->insertItem("0.5"); 
  qlboxSplStep->insertItem("0.2");
  qlboxSplStep->insertItem("0.1");
  qlboxSplStep->insertItem("0.05");
  qlboxSplStep->insertItem("0.02");
  connect(qlboxSplStep, SIGNAL(highlighted(const QString&)), 
	  qledSplStep, SLOT(setText(const QString&)));

  /* OK button */
  OK = new QPushButton("OK",this);
  connect(OK, SIGNAL(clicked()), this, SLOT(getParameters()));
  connect(this, SIGNAL(exit()), qApp, SLOT(closeAllWindows()));

  /* Build the topmost grid layout */
  bigGrid = new QGridLayout( this, 5, 1, 20 );

  /* Build the grid layout for 1st mesh */
  smallGrid1 = new QHBoxLayout();
  smallGrid1->addWidget(qlabMesh1, 0, 0);
  smallGrid1->addWidget(qledMesh1, 0, 1);
  smallGrid1->addWidget(B1, 0, 2);

  /* Build the grid layout for 2nd mesh */
  smallGrid2 = new QHBoxLayout();
  smallGrid2->addWidget(qlabMesh2, 0, 0);
  smallGrid2->addWidget(qledMesh2, 0, 1);
  smallGrid2->addWidget(B2, 0, 2);

  /* Build the grid layout for sampling freq */
  smallGrid3 = new QHBoxLayout();
  smallGrid3->addWidget(qlabSplStep, 0, 0);
  smallGrid3->addWidget(qledSplStep, 0, 1);
  smallGrid3->addWidget(qlboxSplStep, 0, 2);

  /* Build grid layout fir OK button */
  smallGrid4 = new QHBoxLayout();
  smallGrid4->addSpacing(100);
  smallGrid4->addWidget(OK, 0, 1);

  /* Put the sub-grid layouts into the topmost one */
  bigGrid->addLayout(smallGrid1, 0, 0);
  bigGrid->addLayout(smallGrid2, 1, 0);
  bigGrid->addLayout(smallGrid3, 2, 0);
  bigGrid->addLayout(smallGrid4, 3, 0);
 
}

void InitWidget::loadMesh1() {
  QString fn = QFileDialog::getOpenFileName(QString::null, "*.raw", this);
  if ( !fn.isEmpty() )
    qledMesh1->setText(fn);
}
  
void InitWidget::loadMesh2() {
  QString fn = QFileDialog::getOpenFileName(QString::null, "*.raw", this);
  if ( !fn.isEmpty() )
    qledMesh2->setText(fn);

}

void InitWidget::getParameters() {
  QString tmpMesh1, tmpMesh2, tmpSplStep;

  tmpMesh1 = qledMesh1->text();
  mesh1 = (char *)tmpMesh1.latin1();

  tmpMesh2 = qledMesh2->text();
  mesh2 = (char*)tmpMesh2.latin1();

  tmpSplStep = qledSplStep->text();
  step = (char*)tmpSplStep.latin1();

  if(tmpMesh1=="mesh1.raw" || tmpMesh2=="mesh2.raw" || 
     tmpSplStep=="sampling step")
    incompleteFields();
  else {
    isValid = 1;
    emit(exit());
  }
  
}
  
void InitWidget::incompleteFields() {
  QMessageBox::about(this,"ERROR",
		     "You must complete all the fields !");
}
