/* $Id: InitWidget.cpp,v 1.17 2002/02/25 16:09:44 aspert Exp $ */

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

  QLabel *qlabMesh1, *qlabMesh2, *qlabSplStep, *qlabMinSplFreq;
  QPushButton *B1, *B2, *OK;
  QListBox *qlboxSplStep;
  QGridLayout *bigGrid;


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
  qledSplStep->setValidator(new QDoubleValidator(1e-3,1e10,10,qledSplStep));
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
  chkSymDist = new QCheckBox("Compute the symmetrical distance (double run)",
                             this);
  chkSymDist->setChecked(pargs.do_symmetric);

  /* Minimum sample frequency */
  qledMinSplFreq = new QLineEdit(QString("%1").arg(pargs.min_sample_freq), 
                                 this);
  qledMinSplFreq->setValidator(new QIntValidator(0,INT_MAX,qledMinSplFreq));
  qlabMinSplFreq = new QLabel(qledMinSplFreq,
                              "Minimum sampling frequency", this);

  /* Log window checkbox */
  chkLogWindow = new QCheckBox("Log output in external window", this);
  chkLogWindow->setChecked(pargs.do_wlog);

  /* Texture enable checkbox */
  chkTexture = new QCheckBox("Enable error display with texture (CAUTION)", 
                             this);
  chkTexture->setChecked(pargs.do_texture);
  
  /* OK button */
  OK = new QPushButton("OK",this);
  connect(OK, SIGNAL(clicked()), this, SLOT(getParameters()));

  /* Build the grid layout */
  bigGrid = new QGridLayout( this, 10, 3, 20 );

  /* 1st mesh input */
  bigGrid->addWidget(qlabMesh1, 0, 0, Qt::AlignRight);
  bigGrid->addWidget(qledMesh1, 0, 1);
  bigGrid->addWidget(B1, 0, 2);

  /* 2nd mesh input */
  bigGrid->addWidget(qlabMesh2, 1, 0, Qt::AlignRight);
  bigGrid->addWidget(qledMesh2, 1, 1);
  bigGrid->addWidget(B2, 1, 2);

  /* sampling freq input */
  bigGrid->addWidget(qlabSplStep, 3, 0, Qt::AlignRight);
  bigGrid->addWidget(qledSplStep, 3, 1);
  bigGrid->addMultiCellWidget(qlboxSplStep, 2, 4, 2, 2);

  /* Min. sampling frequency input*/
  bigGrid->addWidget(qlabMinSplFreq, 5, 0, Qt::AlignRight);
  bigGrid->addWidget(qledMinSplFreq, 5, 1);

  /* Build grid layout for symmetric distance checkbox */
  bigGrid->addMultiCellWidget(chkSymDist, 6, 6, 0, 2, Qt::AlignLeft);
  bigGrid->addMultiCellWidget(chkLogWindow, 7, 7, 0, 2, Qt::AlignLeft);
  bigGrid->addMultiCellWidget(chkTexture, 8, 8, 0, 2, Qt::AlignLeft);


  /* Build grid layout for OK button */
  bigGrid->addMultiCellWidget(OK, 9, 9, 0, 2, Qt::AlignCenter);
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
  QString str,str2;
  int pos;

  str = qledSplStep->text();
  str2 = qledMinSplFreq->text();
  if (qledMesh1->text().isEmpty() || qledMesh2->text().isEmpty() ||
      qledSplStep->validator()->validate(str,pos) != QValidator::Acceptable ||
      qledMinSplFreq->validator()->validate(str2,pos) !=
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
  QMessageBox::critical(this,"ERROR",
                        "Incomplete or invalid values in fields\n"
                        "Please correct");
}

void InitWidget::meshSetUp() {
  pargs.m1_fname = (char *) qledMesh1->text().latin1();
  pargs.m2_fname = (char *) qledMesh2->text().latin1();
  pargs.sampling_step = atof((char*)qledSplStep->text().latin1())/100;
  pargs.do_symmetric = chkSymDist->isChecked() == TRUE;
  pargs.min_sample_freq = atoi((char*)qledMinSplFreq->text().latin1());
  pargs.do_wlog = chkLogWindow->isChecked() == TRUE;
  pargs.do_texture = (chkTexture->isChecked() == TRUE);

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
  c = new ScreenWidget(model1, model2, pargs.do_texture);
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

