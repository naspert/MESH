/* $Id: InitWidget.h,v 1.7 2001/09/28 11:48:14 aspert Exp $ */

#ifndef INITW_H
#define INITW_H

#include <qwidget.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <mesh_run.h>
#include <compute_error.h>
#include <ScreenWidget.h>

class InitWidget : public QWidget {
  Q_OBJECT

public:
  InitWidget(struct args defArgs, QWidget *parent=0, const char *name=0 );

private slots:
  void loadMesh1();
  void loadMesh2();
  void getParameters();
  void incompleteFields();
  void meshRun();

private:
    QLineEdit *qledMesh1, *qledMesh2, *qledSplStep; 
    QCheckBox *chkSymDist, *chkCurv;
    struct args pargs;
    struct model_error model1,model2;
    ScreenWidget *c;
};

#endif

