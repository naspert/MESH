/* $Id: ScreenWidget.h,v 1.13 2001/11/06 10:15:38 dsanta Exp $ */
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
  void updateColorBar(int id);

private:
  struct model_error *model_data;
  
};


#endif
