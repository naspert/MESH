/* $Id: InitWidget.h,v 1.6 2001/09/25 13:24:39 dsanta Exp $ */

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
    QCheckBox *chkSymDist;
    struct args pargs;
    struct model_error model1,model2;
    ScreenWidget *c;
};

#endif

