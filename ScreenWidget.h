/* $Id: ScreenWidget.h,v 1.5 2001/06/13 09:48:32 jacquet Exp $ */
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



class ScreenWidget : public QWidget
{
  Q_OBJECT
public:
  ScreenWidget( model *raw_model1,model *raw_model2,double dmoymin, double dmoymax, QWidget *parent=0, const char *name=0 );
/*   void keyPressEvent(QKeyEvent *k); */

  public slots:
    void about();

 private:
  QString filename;  
};

#endif
