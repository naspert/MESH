/* $Id: ScreenWidget.h,v 1.10 2001/09/25 13:19:34 dsanta Exp $ */
#ifndef SCREENWIDGET_H
#define SCREENWIDGET_H

#include <qwidget.h>
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
