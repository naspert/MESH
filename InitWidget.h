/* $Id: InitWidget.h,v 1.10 2001/10/10 14:51:34 aspert Exp $ */

#ifndef INITW_H
#define INITW_H

// Ugly hack s.t. I can pass the QApplication to the constructor
#define QT_NO_TRANSLATION 

#include <qwidget.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <mesh_run.h>
#include <compute_error.h>
#include <ScreenWidget.h>
#include <TextWidget.h>

// Needed for 'pipe' function
#ifdef _WIN32
#  include <io.h>
#  include <fcntl.h>
#else
#  include <unistd.h>
#endif

// This needs to be here... don't ask me why
// Including stdio.h is not sufficient... sigh
#ifdef _WIN32
#  define fdopen _fdopen
#else
   extern "C" FILE* fdopen(int, const char*);
#endif
 


class InitWidget : public QWidget {
  Q_OBJECT

public:
  InitWidget(struct args defArgs,
             struct model_error *m1, struct model_error *m2, 
	     QApplication *app=0, QWidget *parent=0, const char *name=0 );
  ~InitWidget();

signals:
  void signalrunDone();

private slots:
  void loadMesh1();
  void loadMesh2();
  void getParameters();
  void incompleteFields();
  void meshRun();
  void runDone();

private:
    QLineEdit *qledMesh1, *qledMesh2, *qledSplStep; 
    QCheckBox *chkSymDist, *chkCurv, *chkLogWindow;
    struct args pargs;
    struct model_error *model1,*model2;
    QApplication *qApp;
    ScreenWidget *c;
    TextWidget *textOut;
    FILE *in_p;

};

#endif

