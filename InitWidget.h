/* $Id: InitWidget.h,v 1.12 2002/01/15 17:32:15 aspert Exp $ */

#ifndef INITW_H
#define INITW_H

#include <qwidget.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <mesh_run.h>
#include <ScreenWidget.h>
#include <TextWidget.h>

class InitWidget : public QWidget {
  Q_OBJECT

public:
  InitWidget(struct args defArgs,
             struct model_error *m1, struct model_error *m2, 
	     QWidget *parent=0, const char *name=0 );
  ~InitWidget();

signals:
  void signalrunDone();

private slots:
  void loadMesh1();
  void loadMesh2();
  void getParameters();
  void incompleteFields();
  void meshRun();

private:
  void meshSetUp();
  QLineEdit *qledMesh1, *qledMesh2, *qledSplStep; 
  QCheckBox *chkSymDist, *chkLogWindow;
  struct args pargs;
  struct model_error *model1,*model2;
  ScreenWidget *c;
  TextWidget *textOut;
  struct outbuf *log;
};

extern "C" {
  void QT_prog(void *out, int p);
}

#endif

