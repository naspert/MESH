#ifndef COLORMAPWIDGET_H
#define COLORMAPWIDGET_H

#include <qpainter.h>
#include <ColorMap.h>
#include <qgl.h>

class ColorMapWidget : public QWidget
{
public:
  ColorMapWidget(QWidget *parent=0, const char *name=0 );
protected:
  void paintEvent(QPaintEvent *); 

 private:
  double **colormap;
};

#endif
