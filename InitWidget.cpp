/* $Id: InitWidget.cpp,v 1.14 2002/02/04 17:34:16 dsanta Exp $ */

#include <InitWidget.h>

#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qfiledialog.h>
#include <qhbox.h>
#include <qmessagebox.h>
#include <qvalidator.h>
#include <qcheckbox.h>
#include <qtimer.h>
#include <qapplication.h>
#include <qprogressdialog.h>

InitWidget::InitWidget(struct args defArgs,
                       struct model_error *m1, struct model_error *m2,
		       QWidget *parent, const char *name):
  QWidget( parent, name) {

  QLabel *qlabMesh1, *qlabMesh2, *qlabSplStep;
  QPushButton *B1, *B2, *OK;
  QListBox *qlboxSplStep;
  QGridLayout *bigGrid;
  QHBoxLayout *smallGrid1, *smallGrid2, *smallGrid3, *smallGrid4, *smallGrid5;
  QHBoxLayout *smallGrid6;

  /* Initialize */
  pargs = defArgs;
  model1 = m1;
  model2 = m2;
  c = NULL;
  log = NULL;

  /* First mesh */
  qledMesh1 = new QLineEdit("", this);
  qlabMesh1 = new QLabel(qledMesh1, "First Mesh", this);
  B1 = new QPushButton("Browse...",this);
  connect(B1, SIGNAL(clicked()), this, SLOT(loadMesh1()));
  /* Second mesh */
  qledMesh2 = new QLineEdit("", this);
  qlabMesh2 = new QLabel(qledMesh2, "Second Mesh", this);
  B2 = new QPushButton("Browse...", this);
  connect(B2, SIGNAL(clicked()), this, SLOT(loadMesh2()));
  
  /* Sampling step */
  qledSplStep = new QLineEdit(QString("%1").arg(pargs.sampling_step*100), 
			      this);
  qledSplStep->setValidator(new QDoubleValidator(1e-3,1e10,10,0));
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

  /* Symmetric distance checkbox */
  chkSymDist = new QCheckBox("Calculate the symmetric distance (double run)",
                             this);
  chkSymDist->setChecked(pargs.do_symmetric);

  /* Log window checkbox */
  chkLogWindow = new QCheckBox("Log output in external window", this);
  chkLogWindow->setChecked(pargs.do_wlog);

  /* OK button */
  OK = new QPushButton("OK",this);
  connect(OK, SIGNAL(clicked()), this, SLOT(getParameters()));

  /* Build the topmost grid layout */
  bigGrid = new QGridLayout( this, 6, 1, 20 );

  /* Build the grid layout for 1st mesh */
  smallGrid1 = new QHBoxLayout(bigGrid);
  smallGrid1->addWidget(qlabMesh1, 0, 0);
  smallGrid1->addWidget(qledMesh1, 0, 1);
  smallGrid1->addWidget(B1, 0, 2);

  /* Build the grid layout for 2nd mesh */
  smallGrid2 = new QHBoxLayout(bigGrid);
  smallGrid2->addWidget(qlabMesh2, 0, 0);
  smallGrid2->addWidget(qledMesh2, 0, 1);
  smallGrid2->addWidget(B2, 0, 2);

  /* Build the grid layout for sampling freq */
  smallGrid3 = new QHBoxLayout(bigGrid);
  smallGrid3->addWidget(qlabSplStep, 0, 0);
  smallGrid3->addWidget(qledSplStep, 0, 1);
  smallGrid3->addWidget(qlboxSplStep, 0, 2);

  /* Build grid layout for symmetric distance checkbox */
  smallGrid5 = new QHBoxLayout(bigGrid);
  smallGrid5->addWidget(chkSymDist);


  
  /* Build grid layout for external log window */
  smallGrid6 = new QHBoxLayout(bigGrid);
  smallGrid6->addWidget(chkLogWindow);

  /* Build grid layout for OK button */
  smallGrid4 = new QHBoxLayout(bigGrid);
  smallGrid4->addSpacing(100);
  smallGrid4->addWidget(OK, 0, 1);
}

InitWidget::~InitWidget() {
  delete c;
  delete textOut;
  outbuf_delete(log);
}

void InitWidget::loadMesh1() {
  QStringList mfilters = QStringList() <<
    "3D Models (*.raw; *.wrl)" <<
    "All files (*.*)";
  QFileDialog fd(QString::null,QString::null,this,"Model 1",TRUE);
  fd.setMode(QFileDialog::ExistingFile);
  fd.setFilters(mfilters);
  fd.show();
  QString fn = fd.selectedFile();
  if ( !fn.isEmpty() )
    qledMesh1->setText(fn);
}
  
void InitWidget::loadMesh2() {
  QStringList mfilters = QStringList() <<
    "3D Models (*.raw; *.wrl)" <<
    "All files (*.*)";
  QFileDialog fd(QString::null,QString::null,this,"Model 2",TRUE);
  fd.setMode(QFileDialog::ExistingFile);
  fd.setFilters(mfilters);
  fd.show();
  QString fn = fd.selectedFile();
  if ( !fn.isEmpty() )
    qledMesh2->setText(fn);

}

void InitWidget::getParameters() {
  QString str;
  int pos;

  str = qledSplStep->text();
  if (qledMesh1->text().isEmpty() || qledMesh2->text().isEmpty() ||
      qledSplStep->validator()->validate(str,pos) !=
      QValidator::Acceptable) {
    incompleteFields();
  } else {
    meshSetUp();
    hide();
    // Use a timer, so that the current events can be processed before
    // starting the compute intensive work.
    QTimer::singleShot(0,this,SLOT(meshRun()));
  }
}

void InitWidget::incompleteFields() {
  QMessageBox::about(this,"ERROR",
		     "Incomplete or invalid values in fields\n"
                     "Please correct");
}

void InitWidget::meshSetUp() {
  pargs.m1_fname = (char *) qledMesh1->text().latin1();
  pargs.m2_fname = (char *) qledMesh2->text().latin1();
  pargs.sampling_step = atof((char*)qledSplStep->text().latin1())/100;
  pargs.do_symmetric = chkSymDist->isChecked() == TRUE;
  pargs.do_wlog = chkLogWindow->isChecked() == TRUE;

  if (!pargs.do_wlog) {
    log = outbuf_new(stdio_puts,stdout);
  } else {
    textOut = new TextWidget();
    log = outbuf_new(TextWidget_puts,textOut);
    textOut->show();
  }
}

// Should only be called after meshSetUp
void InitWidget::meshRun() {
  QProgressDialog qProg("Calculating distance",0,100);
  struct prog_reporter pr;

  qProg.setMinimumDuration(1500);
  pr.prog = QT_prog;
  pr.cb_out = &qProg;
  mesh_run(&pargs,model1,model2, log, &pr);
  outbuf_flush(log);
  c = new ScreenWidget(model1, model2);
  if (qApp != NULL) {
    qApp->setMainWidget(c);
  }
  c->show();
}

void QT_prog(void *out, int p) {
  QProgressDialog *qpd;
  qpd = (QProgressDialog*)out;
  qpd->setProgress(p<0 ? qpd->totalSteps() : p );
  qApp->processEvents();
}

