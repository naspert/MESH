#ifndef SCREENWIDGET_H
#define SCREENWIDGET_H

#include <qmenubar.h>
#include <qpopupmenu.h>
#include <RawWidget.h>
#include <ColorMapWidget.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qfont.h>
#include <qlayout.h>
#include <qhbox.h>

class ScreenWidget : public QWidget
{
public:
  ScreenWidget( model *raw_model1,model *raw_model2,QWidget *parent=0, const char *name=0 );

 private:
  QPopupMenu* file;  
};

#endif
