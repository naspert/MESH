#ifndef INIT_H
#define INIT_H

#include <qlabel.h>
#include <qapplication.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qfiledialog.h>
#include <qhbox.h>

class InitWidget : public QWidget
{
  Q_OBJECT
public:
  InitWidget(QWidget *parent=0, const char *name=0 );
  char *mesh1,*mesh2,*thin;
  QString m,n,o;

public slots:
    void load1();
    void load2();
    void collect();
    
signals:

private:
QLineEdit *textEd1, *textEd2, *textEd3; 

};

#endif

