/* $Id: ColorMapWidget.h,v 1.4 2001/08/07 09:01:10 aspert Exp $ */
#ifndef COLORMAPWIDGET_H
#define COLORMAPWIDGET_H

#include <qpainter.h>
#include <ColorMap.h>
#include <qgl.h>
#include <qfont.h>

class ColorMapWidget : public QWidget
{
public:
  ColorMapWidget(double dmoymin, double dmoymax, 
		 QWidget *parent=0, const char *name=0 );
protected:
  void paintEvent(QPaintEvent *); 

 private:
  double **colormap;
  double dmax, dmin;
};

#endif
