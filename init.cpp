/* $Id: init.cpp,v 1.6 2001/07/12 08:36:17 aspert Exp $ */

#include <init.h>

InitWidget::InitWidget(QWidget *parent, const char *name) 
  : QWidget( parent, name)
{

  textEd1 = new QLineEdit("first mesh",this);
  QLabel *textLabel1 = new QLabel(textEd1, "First Mesh", this);
  QPushButton *B1 = new QPushButton("parcourir",this);
  connect(B1, SIGNAL(clicked()), this, SLOT(load1()));
  textEd2 = new QLineEdit("second mesh",this);
  QLabel *textLabel2 = new QLabel(textEd2,"Second Mesh",this);
  QPushButton *B2 = new QPushButton("parcourir",this);
  connect(B2, SIGNAL(clicked()), this, SLOT(load2()));
  textEd3 = new QLineEdit("samplethin",this);
  QListBox *samplethin = new QListBox(this);
  QLabel *textLabel3 = new QLabel(samplethin,"Sampling step",this);
  samplethin->insertItem("0.5");
  samplethin->insertItem("0.2");
  samplethin->insertItem("0.1"); 
  samplethin->insertItem("0.05");
  samplethin->insertItem("0.02");
  samplethin->insertItem("0.01");
  connect(samplethin, SIGNAL(highlighted(const QString&)), textEd3, SLOT(setText(const QString&)));
  QPushButton *OK = new QPushButton("OK",this);
  connect(OK, SIGNAL(clicked()), this, SLOT(collect()));
  connect(this, SIGNAL(exit()), qApp, SLOT(closeAllWindows()));


  QGridLayout *bigGrid = new QGridLayout( this, 5, 1, 20 );

  QHBoxLayout *smallGrid1 = new QHBoxLayout();
  smallGrid1->addWidget(textLabel1,0,0);
  smallGrid1->addWidget(textEd1,0,1);
  smallGrid1->addWidget(B1,0,2);

  QHBoxLayout *smallGrid2 = new QHBoxLayout();
  smallGrid2->addWidget(textLabel2,0,0);
  smallGrid2->addWidget(textEd2,0,1);
  smallGrid2->addWidget(B2,0,2);

  QHBoxLayout *smallGrid3 = new QHBoxLayout();
  smallGrid3->addWidget(textLabel3,0,0);
  smallGrid3->addWidget(textEd3,0,1);
  smallGrid3->addWidget(samplethin,0,2);


  QHBoxLayout *smallGrid4 = new QHBoxLayout();
  smallGrid4->addSpacing(100);
  smallGrid4->addWidget(OK,0,1);

  bigGrid->addLayout(smallGrid1,0,0);
  bigGrid->addLayout(smallGrid2,1,0);
  bigGrid->addLayout(smallGrid3,2,0);
  bigGrid->addLayout(smallGrid4,3,0);
 

}

void InitWidget::load1()
{
  QString fn = QFileDialog::getOpenFileName( QString::null, "*.raw",
				     this);
  if ( !fn.isEmpty() )
    textEd1->setText(fn);
}
  
void InitWidget::load2()
{
  QString fn = QFileDialog::getOpenFileName( QString::null, "*.raw",
					     this);
  if ( !fn.isEmpty() )
    textEd2->setText(fn);

}

void InitWidget::collect()
{
//   QString n;

  m=textEd1->text();
  mesh1=(char *)n.latin1();
  n=textEd2->text();
  mesh2=(char*)n.latin1();
  o=textEd3->text();
  thin=(char*)n.latin1();
  if(m=="first mesh" || n=="second mesh" || o=="samplethin"){
    about();
  }
  else
    emit(exit());
    
  
}
  
void InitWidget::about()
{
  QMessageBox::about(this,"ERROR",
		     "vous devez remplir tous les champs\n"
		     "mais c'est pas vrai");
}
