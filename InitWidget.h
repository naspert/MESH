/* $Id: InitWidget.h,v 1.8 2001/10/01 16:49:17 dsanta Exp $ */

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
  InitWidget(struct args defArgs,
             struct model_error *m1, struct model_error *m2,
             QWidget *parent=0, const char *name=0 );
  ~InitWidget();

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
    struct model_error *model1,*model2;
    ScreenWidget *c;
};

#endif

