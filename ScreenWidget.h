/* $Id: ScreenWidget.h,v 1.17 2002/02/22 12:54:29 aspert Exp $ */
#ifndef SCREENWIDGET_H
#define SCREENWIDGET_H

/* QT includes */
#include <qwidget.h>
#include <qmessagebox.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qpushbutton.h>
#include <qfont.h>
#include <qlayout.h>
#include <qhbox.h>
#include <qapplication.h>
#include <qaction.h>
#include <qtoolbar.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <compute_error.h>


class ScreenWidget : public QWidget {
  Q_OBJECT
public:
  ScreenWidget(struct model_error *model1, struct model_error *model2,
               QWidget *parent=0, const char *name=0 );


protected slots:
  void quit();
  void infoLeftModel();
  void infoRightModel(); 
  void aboutKeys();
  void aboutBugs();

private:
// local copies of the parameters passed to the constructor
  struct model_error *locMod1, *locMod2; 
  enum whichModel {LEFT_MODEL=0, RIGHT_MODEL=1};
  void infoModel(struct model_error *model, int id);
};


#endif
