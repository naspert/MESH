/* $Id: ScreenWidget.h,v 1.22 2002/03/01 09:57:53 aspert Exp $ */
#ifndef SCREENWIDGET_H
#define SCREENWIDGET_H

/* QT includes */
#include <qwidget.h>
#include <qhgroupbox.h>
#include <qslider.h>
#include <qspinbox.h>
#include <qcheckbox.h>
#include <compute_error.h>


class ScreenWidget : public QWidget {
  Q_OBJECT
public:
  ScreenWidget(struct model_error *model1, struct model_error *model2,
               int do_texture=0, QWidget *parent=0, const char *name=0);

signals:
  void dsValChange(int n);

protected slots:
  void quit();
  void infoLeftModel();
  void infoRightModel();
  void disableSlider(int errMode);
  void trapChanges(int n);
  void aboutKeys();
  void aboutBugs();
  void aboutMesh();
  void updatecbStatus(bool state);

private:
  QHGroupBox *qgbSlider;
  QSlider *qslidDispSampDensity;
  QSpinBox *qspSampDensity;
  QCheckBox *qcbInvNorm, *qcbTwoSide;
// local copies of the parameters passed to the constructor
  struct model_error *locMod1, *locMod2; 
  enum whichModel {LEFT_MODEL=0, RIGHT_MODEL=1};
  void infoModel(struct model_error *model, int id);
  void changeGroupBoxTitle(int n);
};


#endif
