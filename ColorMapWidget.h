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
