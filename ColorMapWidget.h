/* $Id: ColorMapWidget.h,v 1.9 2002/02/20 18:24:11 dsanta Exp $ */
#ifndef COLORMAPWIDGET_H
#define COLORMAPWIDGET_H

#include <qwidget.h>
#include <compute_error.h>

class ColorMapWidget : public QWidget
{
Q_OBJECT
public:
  ColorMapWidget(const struct model_error *model1_error,
		 QWidget *parent=0, const char *name=0 );
 ~ColorMapWidget();
 QSize sizeHint() const;
 QSize minimumSizeHint() const;

protected:
  void paintEvent(QPaintEvent *); 

 private:
  const struct model_error *me;
  void doHistogram(int len);
  int *histogram;
  static const int CBAR_WIDTH = 25;
  static const int CBAR_STEP = 5;
  static const int N_LABELS = 9;
  float **colormap;
  int cmap_len;
  double dmax, dmin;
};

#endif
