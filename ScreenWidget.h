/* $Id: ScreenWidget.h,v 1.16 2002/02/20 18:28:37 dsanta Exp $ */
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

public slots:
  void aboutKeys();
  void aboutBugs();
  void quit();

};


#endif
