/* $Id: ColorMapWidget.h,v 1.6 2001/09/27 08:56:55 aspert Exp $ */
#ifndef COLORMAPWIDGET_H
#define COLORMAPWIDGET_H

#include <qwidget.h>

class ColorMapWidget : public QWidget
{
Q_OBJECT
public:
  ColorMapWidget(double dmoymin, double dmoymax, 
		 QWidget *parent=0, const char *name=0 );

public slots:
  void rescale(double dmoymin, double dmoymax);

protected:
  void paintEvent(QPaintEvent *); 

 private:
  double **colormap;
  double dmax, dmin;
};

#endif
