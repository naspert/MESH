/* $Id: ScreenWidget.h,v 1.9 2001/09/11 16:30:54 dsanta Exp $ */
#ifndef SCREENWIDGET_H
#define SCREENWIDGET_H

#include <qmessagebox.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <RawWidget.h>
#include <ColorMapWidget.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qfont.h>
#include <qlayout.h>
#include <qhbox.h>
#include <qapplication.h>
#include <qaction.h>
#include <qmenubar.h>
#include <qpixmap.h>
#include <qtoolbar.h>
#include <qfiledialog.h>
#include <qmime.h>



class ScreenWidget : public QWidget {
  Q_OBJECT
public:
  ScreenWidget(model_error *model1, model_error *model2,
               QWidget *parent=0, const char *name=0 );
  QPushButton *quitBut;// Needed to get the 'clicked' signal in the viewer

public slots:
  void aboutKeys();
  void aboutBugs();
 
    
};

#endif
