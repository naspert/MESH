/* $Id: ColorMapWidget.h,v 1.10 2002/02/21 09:28:35 dsanta Exp $ */
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
  enum constIntHack1 { CBAR_WIDTH = 25 };
  enum constIntHack2 { CBAR_STEP = 5 };
  enum constIntHack3 { N_LABELS = 9 };
  float **colormap;
  int cmap_len;
  double dmax, dmin;
};

#endif
