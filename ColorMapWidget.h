/* $Id: ColorMapWidget.h,v 1.5 2001/09/25 13:20:06 dsanta Exp $ */
#ifndef COLORMAPWIDGET_H
#define COLORMAPWIDGET_H

#include <qwidget.h>

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
