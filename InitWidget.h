/* $Id: InitWidget.h,v 1.3 2001/09/10 15:07:42 dsanta Exp $ */

#ifndef INITW_H
#define INITW_H

#include <qlabel.h>
#include <qapplication.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qfiledialog.h>
#include <qhbox.h>
#include <qmessagebox.h>
#include <qcheckbox.h>

class InitWidget : public QWidget {
  Q_OBJECT

public:
  InitWidget(QWidget *parent=0, const char *name=0 );
  char *mesh1, *mesh2, *step;

public slots:
    void loadMesh1();
    void loadMesh2();
    void getParameters();
    void incompleteFields();
    
signals:
    void exit();

private:
    QLineEdit *qledMesh1, *qledMesh2, *qledSplStep; 

};

#endif

