/* $Id: ColorMapWidget.h,v 1.7 2001/10/01 16:47:35 dsanta Exp $ */
#ifndef COLORMAPWIDGET_H
#define COLORMAPWIDGET_H

#include <qwidget.h>

class ColorMapWidget : public QWidget
{
Q_OBJECT
public:
  ColorMapWidget(double dmoymin, double dmoymax, 
		 QWidget *parent=0, const char *name=0 );
 ~ColorMapWidget();

public slots:
  void rescale(double dmoymin, double dmoymax);

protected:
  void paintEvent(QPaintEvent *); 

 private:
  double **colormap;
  double dmax, dmin;
};

#endif
