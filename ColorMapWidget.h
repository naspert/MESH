/* $Id: ColorMapWidget.h,v 1.8 2001/11/06 17:15:15 dsanta Exp $ */
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
 QSize sizeHint() const;
 QSize minimumSizeHint() const;

public slots:
  void rescale(double dmoymin, double dmoymax);

protected:
  void paintEvent(QPaintEvent *); 

 private:
  double **colormap;
  double dmax, dmin;
};

#endif
