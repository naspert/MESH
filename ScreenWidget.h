/* $Id: ScreenWidget.h,v 1.15 2002/01/15 17:02:05 aspert Exp $ */
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

signals:
  void actualUpdate(double dmoymin, double dmoymax);

public slots:
  void aboutKeys();
  void aboutBugs();
  void quit();


private:
  struct model_error *model_data;
  
};


#endif
